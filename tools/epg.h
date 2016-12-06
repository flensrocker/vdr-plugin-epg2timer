#ifndef epg2timer_tools_epg_h
#define epg2timer_tools_epg_h

#include <vdr/epg.h>


namespace epg2timer
{
  class cFilterContext;

  class cEpgTools
  {
  public:
    static cStringList *ExtractTagValues(const cFilterContext& Context, const cStringList &Tags, const cEvent *Event);
  };
}

#endif
