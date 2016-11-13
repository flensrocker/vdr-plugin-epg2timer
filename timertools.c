#include "timertools.h"


const cTimer *epg2timer::cTimerTools::FindTimer(cTimers *timers, const cSchedules *schedules, const cEvent *event)
{
  if ((timers == NULL) || (schedules == NULL) || (event == NULL))
     return NULL;

  for (cTimer *t = timers->First(); t; t = timers->Next(t)) {
      bool clearActive = false;
      if (!t->HasFlags(tfActive)) {
         // otherwise cTimer::Matches won't find this timer
         t->SetFlags(tfActive);
         clearActive = true;
         }
      t->SetEventFromSchedule(schedules);
      int overlap = 0;
      eTimerMatch match = t->Matches(event, &overlap);
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
