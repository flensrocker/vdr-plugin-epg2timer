#include "filtercontext.h"

epg2timer::cFilterContext::cFilterContext(void)
{
  _converter = new cStringConverter();
}


epg2timer::cFilterContext::~cFilterContext(void)
{
  delete _converter;
}
