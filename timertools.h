#ifndef epg2timer_timertools_h
#define epg2timer_timertools_h

#include <vdr/timers.h>

namespace epg2timer
{
  class cTimerTools
  {
  public:
    static const cTimer *FindTimer(cTimers *timers, const cSchedules *schedules, const cEvent *event);
  };
}

#endif
