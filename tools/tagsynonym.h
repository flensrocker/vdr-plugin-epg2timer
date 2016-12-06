#ifndef epg2timer_tools_tagsynonym_h
#define epg2timer_tools_tagsynonym_h

#include <vdr/tools.h>


namespace epg2timer
{
  class cTagSynonym : public cListObject
  {
  public:
    cTagSynonym(cStringList *Synonyms);
    virtual ~cTagSynonym(void);

    bool Contains(const char *Name) const;
    const cStringList *Synonyms(void) const { return _synonyms; };

  private:
    cStringList *_synonyms;
  };
}

#endif
