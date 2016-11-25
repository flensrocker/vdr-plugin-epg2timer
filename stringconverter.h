#ifndef epg2timer_stringconverter_h
#define epg2timer_stringconverter_h

#include <vdr/tools.h>
#include <unicode/translit.h>


namespace epg2timer
{
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
}

#endif
