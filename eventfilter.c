#include "eventfilter.h"

#include "epgtools.h"
#include "filenametools.h"
#include "filtercontext.h"


epg2timer::cEventFilter::cEventFilter(const char *Name, eFilterActions Action, const char *Filename, const cEventFilterBase *Filter, int MarginStart, int MarginStop, int Priority, int Lifetime)
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
     timer->SetFile(*cFilenameTools::ReplaceTags(*_filename, Event));

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
     cString filename = cFilenameTools::ReplaceTags(*_filename, Event);
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


epg2timer::cEventFilterList::cEventFilterList(cList<cEventFilterBase> *Filters)
{
  _filters = Filters;
}


epg2timer::cEventFilterList::~cEventFilterList(void)
{
  delete _filters;
  _filters = NULL;
}


bool epg2timer::cEventFilterList::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  return false;
}


epg2timer::cEventFilterAnd::cEventFilterAnd(cList<cEventFilterBase> *Filters)
 :cEventFilterList(Filters)
{
}

bool epg2timer::cEventFilterAnd::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  for (cEventFilterBase *f = _filters->First(); f; f = _filters->Next(f)) {
      if (!f->Matches(Context, Event))
         return false;
      }
  return true;
}


epg2timer::cEventFilterOr::cEventFilterOr(cList<cEventFilterBase> *Filters)
 :cEventFilterList(Filters)
{
}

bool epg2timer::cEventFilterOr::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  for (cEventFilterBase *f = _filters->First(); f; f = _filters->Next(f)) {
      if (f->Matches(Context, Event))
         return true;
      }
  return false;
}


epg2timer::cEventFilterChannel::cEventFilterChannel(tChannelID FromChannel, tChannelID ToChannel)
{
  _fromChannel = FromChannel;
  _toChannel = ToChannel;
}

bool epg2timer::cEventFilterChannel::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if (Event == NULL)
     return false;
  if ((_fromChannel == tChannelID::InvalidID) && (_toChannel == tChannelID::InvalidID))
     return true;

  tChannelID eventChannelID = Event->ChannelID();
  if (eventChannelID == tChannelID::InvalidID)
     return false;

#if APIVERSNUM < 20301
  cChannels *channels = Context.Channels();
#else
  const cChannels *channels = Context.Channels();
#endif
  if (channels == NULL)
     return false;

  const cChannel *eventChannel = channels->GetByChannelID(eventChannelID);
  if (eventChannel == NULL)
     return false;

  int eventChannelNumber = eventChannel->Number();

  if (_fromChannel == tChannelID::InvalidID) {
     const cChannel *to = channels->GetByChannelID(_toChannel);
     if (to == NULL)
        return false;

     return (eventChannelNumber <= to->Number());
     }
  else if (_toChannel == tChannelID::InvalidID) {
     const cChannel *from = channels->GetByChannelID(_fromChannel);
     if (from == NULL)
        return false;

     return (from->Number() <= eventChannelNumber);
     }
  else {
     const cChannel *from = channels->GetByChannelID(_fromChannel);
     const cChannel *to = channels->GetByChannelID(_toChannel);
     if ((from == NULL) || (to == NULL))
        return false;

     int fromNumber = from->Number();
     int toNumber = to->Number();

     // swap channels, fromNumber must be lesser than toNumber
     if (fromNumber > toNumber) {
        int swap = fromNumber;
        fromNumber = toNumber;
        toNumber = swap;
        }

     return (fromNumber <= eventChannelNumber) && (eventChannelNumber <= toNumber);
     }

  return true;
}


epg2timer::cEventFilterContains::cEventFilterContains(const cFilterContext& Context, const char *Needle, int Fields)
{
  _needle = Context.Converter()->Convert(Needle);
  _fields = Fields;
}

bool epg2timer::cEventFilterContains::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if ((Event == NULL) || (_fields == 0) || (*_needle == NULL) || (**_needle == 0))
     return false;

  if ((_fields & efTitle) && Matches(Context, Event->Title()))
     return true;
  if ((_fields & efShortText) && Matches(Context, Event->ShortText()))
     return true;
  if ((_fields & efDescription) && Matches(Context, Event->Description()))
     return true;
  return false;
}

bool epg2timer::cEventFilterContains::Matches(const cFilterContext& Context, const char *Text) const
{
  if ((Text == NULL) || (*Text == 0))
     return false;
  cString t = Context.Converter()->Convert(Text);
  return (strcasestr(t, *_needle) != NULL);
}


epg2timer::cEventFilterTag::cTagFilter::cTagFilter(const cFilterContext& Context, const char *Tag, eTagFilterOperator Op, const char *Comp, bool Missing)
{
  _tag = Tag;
  _op = Op;
  _intComp = 0;
  _strComp = "";
  _strCompLen = 0;
  _missing = Missing;

  if ((*_tag == NULL) || (**_tag == 0))
     _op = tfoInvalid;
  else if ((_op > tfoInvalid) && (_op < tfoIsInt)) {
     if (!isnumber(Comp))
        _op = tfoInvalid;
     else
        _intComp = (int)StrToNum(Comp);
     }
  else {
     _strComp = Context.Converter()->Convert(Comp);
     _strCompLen = strlen(*_strComp);
     }
}


bool epg2timer::cEventFilterTag::cTagFilter::Matches(const char *Value) const
{
  if (_op == tfoInvalid)
     return false;
  if (Value == NULL)
     return _missing;

  if (_op < tfoIsInt) {
     if (!isnumber(Value))
        return false;

     int intval = (int)StrToNum(Value);
     switch (_op) {
       case tfoIntEqual:
         return (intval == _intComp);
       case tfoIntNotEqual:
         return (intval != _intComp);
       case tfoIntLesser:
         return (intval < _intComp);
       case tfoIntLesserOrEqual:
         return (intval <= _intComp);
       case tfoIntGreater:
         return (intval > _intComp);
       case tfoIntGreaterOrEqual:
         return (intval >= _intComp);
       default:
         break;
       }
     }
  else {
     switch (_op) {
       case tfoStrEqual:
         return (strcmp(Value, _strComp) == 0);
       case tfoStrNotEqual:
         return (strcmp(Value, _strComp) != 0);
       case tfoStrLesser:
         return (strcmp(Value, _strComp) < 0);
       case tfoStrLesserOrEqual:
         return (strcmp(Value, _strComp) <= 0);
       case tfoStrGreater:
         return (strcmp(Value, _strComp) > 0);
       case tfoStrGreaterOrEqual:
         return (strcmp(Value, _strComp) >= 0);
       case tfoStrIsEmpty:
         return (*Value == 0);
       case tfoStrIsNotEmpty:
         return (*Value != 0);
       case tfoStrContains:
         return (strstr(Value, *_strComp) != NULL);
       case tfoStrNotContains:
         return (strstr(Value, *_strComp) == NULL);
       case tfoStrStartswith:
         return (strlen(Value) <= _strCompLen) && startswith(*_strComp, Value);
       case tfoStrEndswith:
         return (strlen(Value) <= _strCompLen) && endswith(*_strComp, Value);
       default:
         break;
       }
     }

  return false;
}


epg2timer::cEventFilterTag::cEventFilterTag(cList<cTagFilter> *TagFilters, bool Missing)
{
  _tagFilters = TagFilters;
  _missing = Missing;
  for (const cTagFilter *tf = _tagFilters->First(); tf; tf = _tagFilters->Next(tf))
      _tagNames.Append(strdup(tf->Tag()));
}


epg2timer::cEventFilterTag::~cEventFilterTag(void)
{
  delete _tagFilters;
}


bool epg2timer::cEventFilterTag::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if ((Event == NULL) || (_tagFilters->Count() == 0))
     return false;

  cStringList *values = cEpgTools::ExtractTagValues(_tagNames, Event);
  if (values == NULL)
     return _missing;

  int i = 0;
  for (const cTagFilter *tf = _tagFilters->First(); tf; tf = _tagFilters->Next(tf), i++) {
      cString value = Context.Converter()->Convert(values->At(i));
      if (!tf->Matches(*value)) {
         delete values;
         return false;
         }
      }
  delete values;
  return true;
}
