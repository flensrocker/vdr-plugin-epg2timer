#ifndef epg2timer_filters_channel_h
#define epg2timer_filters_channel_h

#include "../filterbase.h"


namespace epg2timer
{
  // The event must belong to a channel between FromChannel and ToChannel.
  // The channel numbers are used for comparison.
  // If FromChannel is the invalid channel-id, all channels with numbers up
  // to ToChannel's number match.
  // If ToChannel is the invalid channel-id, all channels with numbers
  // from FromChannel's number match.
  class cEventFilterChannel : public cEventFilterBase
  {
  public:
    cEventFilterChannel(tChannelID FromChannel, tChannelID ToChannel);
    virtual ~cEventFilterChannel(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;

  private:
    tChannelID _fromChannel;
    tChannelID _toChannel;
  };
}

#endif
