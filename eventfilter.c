#include "eventfilter.h"


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
