#include "eventfilter.h"


epg2timer::cEventFilterContains::cEventFilterContains(const char *needle, int fields)
{
  _needle = needle;
  _fields = fields;
}

bool epg2timer::cEventFilterContains::Matches(const cEvent *event) const
{
  if ((event == NULL) || (_fields == 0) || (*_needle == NULL) || (**_needle == 0))
     return false;

  if ((_fields & efTitle) && Matches(event->Title()))
     return true;
  if ((_fields & efShortText) && Matches(event->ShortText()))
     return true;
  if ((_fields & efDescription) && Matches(event->Description()))
     return true;
  return false;
}

bool epg2timer::cEventFilterContains::Matches(const char *text) const
{
  if ((text == NULL) || (*text == 0))
     return false;
  // TODO remove special characters like '" etc. to get better match
  return (strcasestr(text, *_needle) != NULL);
}
