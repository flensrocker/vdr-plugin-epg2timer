#include "eventfilter.h"

#include "tools/filename.h"


epg2timer::cEventFilter::cEventFilter(const cFilterContext& Context, const char *Name, eFilterActions Action, const char *Filename, const cEventFilterBase *Filter, int MarginStart, int MarginStop, int Priority, int Lifetime)
 :_context(Context)
{
  _name = Name;
  _action = Action;
  _filename = Filename;
  _marginStart = MarginStart;
  _marginStop = MarginStop;
  _priority = Priority;
  _lifetime = Lifetime;
  _filter = Filter;

  if (_marginStart < 0)
     _marginStart = Setup.MarginStart;
  if (_marginStop < 0)
     _marginStop = Setup.MarginStop;
}


bool epg2timer::cEventFilter::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if (_filter == NULL)
     return false;

  return _filter->Matches(Context, Event);
}


const char *epg2timer::cEventFilter::Name() const
{
  return *_name;
}

#define AUX_STARTTAG "<epg2timer>"
#define AUX_ENDTAG "</epg2timer>"

bool epg2timer::cEventFilter::AuxMatches(const cTimer *Timer) const
{
  bool matches = false;
  const char *aux = Timer->Aux();
  if ((aux == NULL) || ((aux = strstr(aux, AUX_STARTTAG)) == NULL))
     return matches;

  char *name = NULL;
  if (sscanf(aux, AUX_STARTTAG "%m[^<]" AUX_ENDTAG, &name) == 1)
     matches = (strcmp(name, *_name) == 0);
  free(name);
  return matches;
}


void epg2timer::cEventFilter::CalculateStartStopDay(cTimer *Timer, const cEvent *Event, int *Start, int *Stop, time_t *Day) const
{
  // copied from cTimer's constructor
  time_t tstart = Timer->HasFlags(tfVps) ? Event->Vps() : Event->StartTime();
  time_t tstop = tstart + Event->Duration();
  if (!(Timer->HasFlags(tfVps))) {
     tstart -= _marginStart * 60;
     tstop  += _marginStop * 60;
     }
  struct tm tm_r;
  struct tm *time = localtime_r(&tstart, &tm_r);
  *Day = cTimer::SetTime(tstart, 0);
  *Start = time->tm_hour * 100 + time->tm_min;
  time = localtime_r(&tstop, &tm_r);
  *Stop = time->tm_hour * 100 + time->tm_min;
  if (*Stop >= 2400)
     *Stop -= 2400;
}


cTimer *epg2timer::cEventFilter::CreateTimer(const cEvent *Event) const
{
  cTimer *timer = new cTimer(Event);
  // it's only pseudo xml, so ignore special characters
  // if someone names his filter with "</epgtimer>" in it, it's his problem...
  timer->SetAux(*cString::sprintf(AUX_STARTTAG "%s" AUX_ENDTAG, Name()));

  time_t day;
  int start;
  int stop;
  CalculateStartStopDay(timer, Event, &start, &stop, &day);
  timer->SetStart(start);
  timer->SetStop(stop);
  timer->SetDay(day);

  timer->SetPriority(_priority);
  timer->SetLifetime(_lifetime);

  if ((*_filename != NULL) && (**_filename != 0))
     timer->SetFile(*cFilenameTools::ReplaceTags(_context, *_filename, Event));

  if (_action == faInactive)
     timer->ClrFlags(tfActive);

  dsyslog("epg2timer: new timer for event %d, %s", Event->EventID(), Event->Title());
  return timer;
}


bool epg2timer::cEventFilter::UpdateTimer(cTimer *Timer, const cEvent *Event) const
{
  bool updated = false;
  if (!AuxMatches(Timer)) {
     dsyslog("epg2timer: aux mismatch: %s", Timer->Aux());
     return updated;
     }

  // active flag must not be updated, it's under the user's control

  // update filename if tags in description are updated
  if ((*_filename != NULL) && (**_filename != 0)) {
     cString filename = cFilenameTools::ReplaceTags(_context, *_filename, Event);
     if (strcmp(*filename, Timer->File()) != 0) {
        dsyslog("epg2timer: adjust filename from %s to %s on event (%d) %s", Timer->File(), *filename, Event->EventID(), Event->Title());
        Timer->SetFile(*filename);
        updated = true;
        }
     }

  time_t day;
  int start;
  int stop;
  CalculateStartStopDay(Timer, Event, &start, &stop, &day);

  if (Timer->Start() != start) {
     dsyslog("epg2timer: adjust start from %d to %d on event (%d) %s", Timer->Start(), start, Event->EventID(), Event->Title());
     Timer->SetStart(start);
     updated = true;
     }
  if (Timer->Stop() != stop) {
     dsyslog("epg2timer: adjust stop from %d to %d on event (%d) %s", Timer->Stop(), stop, Event->EventID(), Event->Title());
     Timer->SetStop(stop);
     updated = true;
     }
  if (Timer->Day() != day) {
     dsyslog("epg2timer: adjust day from %ld to %ld on event (%d) %s", Timer->Day(), day, Event->EventID(), Event->Title());
     Timer->SetDay(day);
     updated = true;
     }

  return updated;
}


epg2timer::cEventFilter::eFilterActions epg2timer::cEventFilter::Action() const
{
  return _action;
}
