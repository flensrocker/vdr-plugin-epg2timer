#ifndef epg2timer_epgtools_h
#define epg2timer_epgtools_h

#include <vdr/tools.h>
#include <unicode/translit.h>


namespace epg2timer
{
  class cEpgTools
  {
  public:
    static cStringList *ExtractTagValues(const cStringList &tags, const char *description);

    class cStringConverter
    {
    public:
      cStringConverter(void);
      ~cStringConverter(void);

      cString Convert(const char *text) const;

    private:
      UErrorCode _status;
      Transliterator *_converter;
    };
  };
}

#endif
