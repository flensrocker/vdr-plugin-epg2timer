#ifndef epg2timer_timertools_h
#define epg2timer_timertools_h

#include <vdr/timers.h>

namespace epg2timer
{
  class cTimerTools
  {
  public:
    static cTimer *FindTimer(cTimers *Timers, const cSchedules *Schedules, const cEvent *Event);
  };
}

#endif
