#ifndef epg2timer_tools_filtercontext_h
#define epg2timer_tools_filtercontext_h

#include <vdr/tools.h>

class cChannels;


namespace epg2timer
{
  class cStringConverter;
  class cTagSynonym;
  
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

    void AddTagSynonyms(cTagSynonym *Synonyms);
    const cStringList *TagSynonyms(const char *Tag) const;

  private:
    cStringConverter *_converter;
    const cChannels *_channels;
    cList<cTagSynonym> _tagSynonyms;
  };
}

#endif
