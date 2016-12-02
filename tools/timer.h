#ifndef epg2timer_tools_timer_h
#define epg2timer_tools_timer_h

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
