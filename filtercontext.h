#ifndef epg2timer_filtercontext_h
#define epg2timer_filtercontext_h

class cChannels;


namespace epg2timer
{
  class cStringConverter;
  
  class cFilterContext
  {
  public:
    cFilterContext(void);
    ~cFilterContext(void);

    cStringConverter *Converter(void) const { return _converter; };
#if APIVERSNUM < 20301
    cChannels *Channels(void) const { return const_cast<cChannels*>(_channels); }
#else
    const cChannels *Channels(void) const { return _channels; }
#endif
    void SetChannels(const cChannels *Channels) { _channels = Channels; }

  private:
    cStringConverter *_converter;
    const cChannels *_channels;
  };
}

#endif
