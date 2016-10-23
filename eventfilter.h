#ifndef epg2timer_eventfilter_h
#define epg2timer_eventfilter_h

#include <vdr/epg.h>

namespace epg2timer
{
  class cEventFilter
  {
  public:
    enum eEventFields { efTitle       = 0x0001,
                        efShortText   = 0x0002,
                        efDescription = 0x0004,
                        efAll         = 0xFFFF
                      };

    virtual ~cEventFilter(void) {};
    virtual bool Matches(const cEvent *event) const = 0;
  };
  
  class cEventFilterContains : public cEventFilter
  {
  public:
    cEventFilterContains(const char *needle, int fields);
    virtual ~cEventFilterContains(void) {};
    virtual bool Matches(const cEvent *event) const;

  private:
    cString _needle;
    int _fields;

    bool Matches(const char *text) const;
  };

  // TODO cEventFilterAnd (list of filters, all must match)
  // TODO cEventFilterOr (list of filters, at least one must match)
  // TODO cEventFilterDescTag (find tag in description and match contents, e.g. season, year, actor etc.)
}

#endif
