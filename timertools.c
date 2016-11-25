#include "timertools.h"


cTimer *epg2timer::cTimerTools::FindTimer(cTimers *Timers, const cSchedules *Schedules, const cEvent *Event)
{
  if ((Timers == NULL) || (Schedules == NULL) || (Event == NULL))
     return NULL;

  for (cTimer *t = Timers->First(); t; t = Timers->Next(t)) {
      bool clearActive = false;
      if (!t->HasFlags(tfActive)) {
         // otherwise cTimer::Matches won't find this timer
         t->SetFlags(tfActive);
         clearActive = true;
         }
      t->SetEventFromSchedule(Schedules);
      int overlap = 0;
      eTimerMatch match = t->Matches(Event, &overlap);
      if (match == tmFull) {
         if (clearActive)
            t->ClrFlags(tfActive);
         return t;
         }
      if (clearActive)
         t->ClrFlags(tfActive);
      }
  return NULL;
}
