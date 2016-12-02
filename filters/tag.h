#ifndef epg2timer_filters_tag_h
#define epg2timer_filters_tag_h

#include "../filterbase.h"


namespace epg2timer
{
  class cEventFilterTag : public cEventFilterBase
  {
  public:
    class cTagFilter : public cListObject
    {
    public:
      enum eTagFilterOperator { tfoInvalid           = 0,
                                tfoIntEqual          = 1,
                                tfoIntNotEqual       = 2,
                                tfoIntLesser         = 3,
                                tfoIntLesserOrEqual  = 4,
                                tfoIntGreater        = 5,
                                tfoIntGreaterOrEqual = 6,
                                tfoIsInt             = 10,
                                tfoStrEqual          = 11,
                                tfoStrNotEqual       = 12,
                                tfoStrLesser         = 13,
                                tfoStrLesserOrEqual  = 14,
                                tfoStrGreater        = 15,
                                tfoStrGreaterOrEqual = 16,
                                tfoStrIsEmpty        = 17,
                                tfoStrIsNotEmpty     = 18,
                                tfoStrContains       = 19,
                                tfoStrNotContains    = 20,
                                tfoStrStartswith     = 21,
                                tfoStrEndswith       = 22
                              };

      cTagFilter(const cFilterContext& Context, const char *Tag, eTagFilterOperator Op, const char *Comp, bool Missing);
      virtual ~cTagFilter(void) {};

      bool Matches(const char *Value) const;
      const char *Tag(void) const { return *_tag; };

    private:
      cString _tag;
      eTagFilterOperator _op;
      int _intComp;
      cString _strComp;
      uint _strCompLen;
      bool _missing;
    };

    cEventFilterTag(cList<cTagFilter> *TagFilters, bool Missing);
    virtual ~cEventFilterTag(void);
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;

  private:
    cList<cTagFilter> *_tagFilters;
    bool _missing;
    cStringList _tagNames;
  };
}

#endif
