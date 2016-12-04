#ifndef epg2timer_filters_starttime_h
#define epg2timer_filters_starttime_h

#include "../filterbase.h"

namespace epg2timer
{
  // Matches, if the starttime is in the given range.
  class cEventFilterStartTime : public cEventFilterBase
  {
  public:
    cEventFilterStartTime(const char *After, const char *Before);
    virtual ~cEventFilterStartTime(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;

  private:
    cString _after;
    cString _before;
    int _mode;
  };
}

#endif
