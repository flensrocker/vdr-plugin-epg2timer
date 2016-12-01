#ifndef epg2timer_filtercontext_h
#define epg2timer_filtercontext_h

#include "stringconverter.h"

class cChannels;


namespace epg2timer
{
  class cFilterContext
  {
  public:
    cFilterContext(void);
    ~cFilterContext(void);

    cStringConverter *Converter(void) const { return _converter; };
    const cChannels *Channels(void) const { return _channels; }
    void SetChannels(const cChannels *Channels) { _channels = Channels; }

  private:
    cStringConverter *_converter;
    const cChannels *_channels;
  };
}

#endif
