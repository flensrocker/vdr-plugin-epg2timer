#ifndef epg2timer_filters_contains_h
#define epg2timer_filters_contains_h

#include "../filterbase.h"

namespace epg2timer
{
  // Matches, if the title, shorttext or description contains the given text.
  class cEventFilterContains : public cEventFilterBase
  {
  public:
    cEventFilterContains(const cFilterContext& Context, const char *Needle, int Fields);
    virtual ~cEventFilterContains(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;

  private:
    cString _needle;
    int _fields;

    bool Matches(const cFilterContext& Context, const char *Text) const;
  };
}

#endif
