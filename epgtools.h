#ifndef epg2timer_epgtools_h
#define epg2timer_epgtools_h

#include <vdr/epg.h>


namespace epg2timer
{
  class cEpgTools
  {
  public:
    static cStringList *ExtractTagValues(const cStringList &Tags, const cEvent *Event);
  };
}

#endif
