#include "filtercontext.h"

#include "stringconverter.h"
#include "tagsynonym.h"


epg2timer::cFilterContext::cFilterContext(void)
{
  _converter = new cStringConverter();
  _channels = NULL;
}


epg2timer::cFilterContext::~cFilterContext(void)
{
  delete _converter;
}


void epg2timer::cFilterContext::AddTagSynonyms(cTagSynonym *Synonyms)
{
  if (Synonyms != NULL)
     _tagSynonyms.Add(Synonyms);
}


const cStringList *epg2timer::cFilterContext::TagSynonyms(const char *Tag) const
{
  for (const cTagSynonym *ts = _tagSynonyms.First(); ts; ts = _tagSynonyms.Next(ts)) {
      if (ts->Contains(Tag))
         return ts->Synonyms();
      }
  return NULL;
}
