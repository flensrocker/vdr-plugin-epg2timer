#include "eventfilter.h"


epg2timer::cEventFilter::cEventFilter(const char *Name, eFilterActions Action, const cEventFilterBase *Filter)
{
  _name = Name;
  _action = Action;
  _filter = Filter;
}


bool epg2timer::cEventFilter::Matches(const cEvent *event) const
{
  if (_filter == NULL)
     return false;

  return _filter->Matches(event);
}


const char *epg2timer::cEventFilter::Name() const
{
  return *_name;
}


bool epg2timer::cEventFilter::AuxMatches(const cTimer *timer) const
{
  const char *aux = timer->Aux();
  if ((aux == NULL) || (strlen(aux) < 10) || !startswith(aux, "epg2timer="))
     return false;

  return (strcasecmp(*_name, aux + 10) == 0);
}


cTimer *epg2timer::cEventFilter::CreateTimer(const cEvent *event) const
{
  cTimer *timer = new cTimer(event);

  // mark timer
  timer->SetAux(*cString::sprintf("epg2timer=%s", Name()));

  if (_action == faInactive)
     timer->ClrFlags(tfActive);
  return timer;
}


bool epg2timer::cEventFilter::UpdateTimer(cTimer *timer, const cEvent *event) const
{
  if (!AuxMatches(timer))
     return false;

  // active flag should not be updated, it's under user's control
  //if (_action == faInactive) {
  //   if (timer->HasFlags(tfActive)) {
  //      timer->ClrFlags(tfActive);
  //      return true;
  //      }
  //   }
  //else if (_action == faRecord) {
  //   if (!timer->HasFlags(tfActive)) {
  //      timer->SetFlags(tfActive);
  //      return true;
  //      }
  //   }
  return false;
}


epg2timer::cEventFilter::eFilterActions epg2timer::cEventFilter::Action() const
{
  return _action;
}


epg2timer::cEventFilterList::cEventFilterList(cList<cEventFilterBase> *filters)
{
  _filters = filters;
}


epg2timer::cEventFilterList::~cEventFilterList(void)
{
  delete _filters;
  _filters = NULL;
}


bool epg2timer::cEventFilterList::Matches(const cEvent *event) const
{
  return false;
}


epg2timer::cEventFilterAnd::cEventFilterAnd(cList<cEventFilterBase> *filters)
 :cEventFilterList(filters)
{
}

bool epg2timer::cEventFilterAnd::Matches(const cEvent *event) const
{
  for (cEventFilterBase *f = _filters->First(); f; f = _filters->Next(f)) {
      if (!f->Matches(event))
         return false;
      }
  return true;
}


epg2timer::cEventFilterOr::cEventFilterOr(cList<cEventFilterBase> *filters)
 :cEventFilterList(filters)
{
}

bool epg2timer::cEventFilterOr::Matches(const cEvent *event) const
{
  for (cEventFilterBase *f = _filters->First(); f; f = _filters->Next(f)) {
      if (f->Matches(event))
         return true;
      }
  return false;
}


epg2timer::cEventFilterChannel::cEventFilterChannel(tChannelID fromChannel, tChannelID toChannel)
{
  _fromChannel = fromChannel;
  _toChannel = toChannel;
}

bool epg2timer::cEventFilterChannel::Matches(const cEvent *event) const
{
  if (event == NULL)
     return false;
  if ((_fromChannel == tChannelID::InvalidID) && (_toChannel == tChannelID::InvalidID))
     return true;

  tChannelID eventChannelID = event->ChannelID();
  if (eventChannelID == tChannelID::InvalidID)
     return false;

#if APIVERSNUM < 20301
  cChannels *channels = &Channels;
#else
  // TODO vdr 2.3: get read-only channels
#endif
  if (channels == NULL)
     return false;

  const cChannel *eventChannel = channels->GetByChannelID(eventChannelID);
  if (eventChannel == NULL)
     return false;

  int eventChannelNumber = eventChannel->Number();

  if (_fromChannel == tChannelID::InvalidID) {
     const cChannel *to = channels->GetByChannelID(_toChannel);
     if (to == NULL)
        return false;

     return (eventChannelNumber <= to->Number());
     }
  else if (_toChannel == tChannelID::InvalidID) {
     const cChannel *from = channels->GetByChannelID(_fromChannel);
     if (from == NULL)
        return false;

     return (from->Number() <= eventChannelNumber);
     }
  else {
     const cChannel *from = channels->GetByChannelID(_fromChannel);
     const cChannel *to = channels->GetByChannelID(_toChannel);
     if ((from == NULL) || (to == NULL))
        return false;

     int fromNumber = from->Number();
     int toNumber = to->Number();

     // swap channels, fromNumber must be lesser than toNumber
     if (fromNumber > toNumber) {
        int swap = fromNumber;
        fromNumber = toNumber;
        toNumber = swap;
        }

     return (fromNumber <= eventChannelNumber) && (eventChannelNumber <= toNumber);
     }

  return true;
}


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


epg2timer::cEventFilterTag::cTagFilter::cTagFilter(const char *tag, eTagFilterOperator op, const char *comp)
{
  _tag = tag;
  _op = op;
  _intComp = 0;
  _strComp = "";
  _strCompLen = 0;

  if ((*_tag == NULL) || (**_tag == 0))
     _op = tfoInvalid;
  else if ((_op > tfoInvalid) && (_op < tfoIsInt)) {
     if (!isnumber(comp))
        _op = tfoInvalid;
     else
        _intComp = (int)StrToNum(comp);
     }
  else {
     _strComp = comp;
     _strCompLen = strlen(comp);
     }
}


bool epg2timer::cEventFilterTag::cTagFilter::Matches(const char *tag, const char *value) const
{
  if ((_op == tfoInvalid) || (value == NULL))
     return false;

  if (_op < tfoIsInt) {
     if (!isnumber(value))
        return false;

     int intval = (int)StrToNum(value);
     switch (_op) {
       case tfoIntEqual:
         return (intval == _intComp);
       case tfoIntNotEqual:
         return (intval != _intComp);
       case tfoIntLesser:
         return (intval < _intComp);
       case tfoIntLesserOrEqual:
         return (intval <= _intComp);
       case tfoIntGreater:
         return (intval > _intComp);
       case tfoIntGreaterOrEqual:
         return (intval >= _intComp);
       default:
         break;
       }
     }
  else {
     switch (_op) {
       case tfoStrEqual:
         return (strcasecmp(value, _strComp) == 0);
       case tfoStrNotEqual:
         return (strcasecmp(value, _strComp) != 0);
       case tfoStrLesser:
         return (strcasecmp(value, _strComp) < 0);
       case tfoStrLesserOrEqual:
         return (strcasecmp(value, _strComp) <= 0);
       case tfoStrGreater:
         return (strcasecmp(value, _strComp) > 0);
       case tfoStrGreaterOrEqual:
         return (strcasecmp(value, _strComp) >= 0);
       case tfoStrIsEmpty:
         return (*value == 0);
       case tfoStrIsNotEmpty:
         return (*value != 0);
       case tfoStrContains:
         return (strcasestr(value, *_strComp) != NULL);
       case tfoStrNotContains:
         return (strcasestr(value, *_strComp) == NULL);
       case tfoStrStartswith:
         return (strlen(value) <= _strCompLen) && startswith(*_strComp, value);
       case tfoStrEndswith:
         return (strlen(value) <= _strCompLen) && endswith(*_strComp, value);
       default:
         break;
       }
     }

  return false;
}


epg2timer::cEventFilterTag::cEventFilterTag(cList<cTagFilter> *tagFilters)
{
  _tagFilters = tagFilters;
}


epg2timer::cEventFilterTag::~cEventFilterTag(void)
{
  delete _tagFilters;
}


bool epg2timer::cEventFilterTag::Matches(const cEvent *event) const
{
  
  for (const cTagFilter *tf = _tagFilters->First(); tf; tf = _tagFilters->Next(tf)) {
      }
  return true;
}
