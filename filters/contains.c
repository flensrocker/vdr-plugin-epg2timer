#include "contains.h"

#include "../tools/stringconverter.h"
#include "../tools/filtercontext.h"


epg2timer::cEventFilterContains::cEventFilterContains(const cFilterContext& Context, const char *Needle, int Fields, bool Not)
{
  _needle = Context.Converter()->Convert(Needle);
  _fields = Fields;
  _not = Not;
}

bool epg2timer::cEventFilterContains::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if ((Event == NULL) || (_fields == 0) || (*_needle == NULL) || (**_needle == 0))
     return false;

  if ((_fields & efTitle) && Matches(Context, Event->Title()))
     return true;
  if ((_fields & efShortText) && Matches(Context, Event->ShortText()))
     return true;
  if ((_fields & efDescription) && Matches(Context, Event->Description()))
     return true;
  return false;
}

bool epg2timer::cEventFilterContains::Matches(const cFilterContext& Context, const char *Text) const
{
  if ((Text == NULL) || (*Text == 0))
     return false;
  cString t = Context.Converter()->Convert(Text);
  bool matches = (strcasestr(t, *_needle) != NULL);
  if (_not)
     return !matches;
  return matches;
}
