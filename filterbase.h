#ifndef epg2timer_filterbase_h
#define epg2timer_filterbase_h

#include <vdr/epg.h>
#include <vdr/timers.h>

namespace epg2timer
{
  class cFilterContext;

  class cEventFilterBase : public cListObject
  {
  public:
    enum eEventFields { efTitle       = 0x0001,
                        efShortText   = 0x0002,
                        efDescription = 0x0004,
                        efAll         = 0xFFFF
                      };

    cEventFilterBase(void) {};
    virtual ~cEventFilterBase(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const = 0;
  };
}

#endif
