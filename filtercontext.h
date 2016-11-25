#ifndef epg2timer_filtercontext_h
#define epg2timer_filtercontext_h

#include "stringconverter.h"


namespace epg2timer
{
  class cFilterContext
  {
  public:
    cFilterContext(void);
    ~cFilterContext(void);

    cStringConverter *Converter(void) { return _converter; };

  private:
    cStringConverter *_converter;
  };
}

#endif
