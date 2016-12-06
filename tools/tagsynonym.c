#include "tagsynonym.h"


epg2timer::cTagSynonym::cTagSynonym(cStringList *Synonyms)
{
  _synonyms = Synonyms;
}


epg2timer::cTagSynonym::~cTagSynonym(void)
{
  delete _synonyms;
}


bool epg2timer::cTagSynonym::Contains(const char *Name) const
{
  if (_synonyms == NULL)
     return false;

  return (_synonyms->Find(Name) >= 0);
}
