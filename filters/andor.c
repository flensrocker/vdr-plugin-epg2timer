#include "andor.h"

#include "../filtercontext.h"


epg2timer::cEventFilterList::cEventFilterList(cList<cEventFilterBase> *Filters)
{
  _filters = Filters;
}


epg2timer::cEventFilterList::~cEventFilterList(void)
{
  delete _filters;
  _filters = NULL;
}


bool epg2timer::cEventFilterList::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  return false;
}


epg2timer::cEventFilterAnd::cEventFilterAnd(cList<cEventFilterBase> *Filters)
 :cEventFilterList(Filters)
{
}

bool epg2timer::cEventFilterAnd::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  for (cEventFilterBase *f = _filters->First(); f; f = _filters->Next(f)) {
      if (!f->Matches(Context, Event))
         return false;
      }
  return true;
}


epg2timer::cEventFilterOr::cEventFilterOr(cList<cEventFilterBase> *Filters)
 :cEventFilterList(Filters)
{
}

bool epg2timer::cEventFilterOr::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  for (cEventFilterBase *f = _filters->First(); f; f = _filters->Next(f)) {
      if (f->Matches(Context, Event))
         return true;
      }
  return false;
}
