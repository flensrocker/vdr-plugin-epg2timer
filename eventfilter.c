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


cTimer *epg2timer::cEventFilter::CreateTimer(const cEvent *event) const
{
  cTimer *timer = new cTimer(event);
  if (_action == faInactive)
     timer->ClrFlags(tfActive);
  return timer;
}


bool epg2timer::cEventFilter::UpdateTimer(cTimer *timer, const cEvent *event) const
{
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
