#ifndef epg2timer_filters_andor_h
#define epg2timer_filters_andor_h

#include "../filterbase.h"


namespace epg2timer
{
  // Base class for "And" and "Or" filter.
  // Deletes the provided "Filters" list.
  class cEventFilterList : public cEventFilterBase
  {
  public:
    cEventFilterList(cList<cEventFilterBase> *Filters);
    virtual ~cEventFilterList(void);
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;

  protected:
    cList<cEventFilterBase> *_filters;
  };

  // All filters in the list must match.
  class cEventFilterAnd : public cEventFilterList
  {
  public:
    cEventFilterAnd(cList<cEventFilterBase> *Filters);
    virtual ~cEventFilterAnd(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;
  };

  // At least one filter in the list must match.
  class cEventFilterOr : public cEventFilterList
  {
  public:
    cEventFilterOr(cList<cEventFilterBase> *Filters);
    virtual ~cEventFilterOr(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;
  };
}

#endif
