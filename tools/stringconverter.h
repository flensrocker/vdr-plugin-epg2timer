#ifndef epg2timer_tools_stringconverter_h
#define epg2timer_tools_stringconverter_h

#ifndef EPG2TIMER_DISABLE_LIBICU

#include <unicode/uconfig.h>
#include <unicode/platform.h>
#include <unicode/utypes.h>
#include <unicode/unistr.h>
#include <unicode/translit.h>

#endif

#include <vdr/tools.h>


namespace epg2timer
{
  class cStringConverter
  {
  public:
    cStringConverter(void);
    ~cStringConverter(void);

    cString Convert(const char *Text) const;

  private:
    bool _systemIsUtf8;
#ifndef EPG2TIMER_DISABLE_LIBICU
    UErrorCode _status;
    icu::Transliterator *_converter;
#endif
  };
}

#endif
