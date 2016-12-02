#include "filtercontext.h"

#include "tools/stringconverter.h"


epg2timer::cFilterContext::cFilterContext(void)
{
  _converter = new cStringConverter();
  _channels = NULL;
}


epg2timer::cFilterContext::~cFilterContext(void)
{
  delete _converter;
}
