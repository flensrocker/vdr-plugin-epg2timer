#include "filtercontext.h"

epg2timer::cFilterContext::cFilterContext(void)
{
  _converter = new cStringConverter();
  _channels = NULL;
}


epg2timer::cFilterContext::~cFilterContext(void)
{
  delete _converter;
}
