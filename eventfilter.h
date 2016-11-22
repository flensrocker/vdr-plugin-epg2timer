#ifndef epg2timer_eventfilter_h
#define epg2timer_eventfilter_h

#include <vdr/epg.h>
#include <vdr/timers.h>

namespace epg2timer
{
  class cEventFilterBase : public cListObject
  {
  public:
    enum eEventFields { efTitle       = 0x0001,
                        efShortText   = 0x0002,
                        efDescription = 0x0004,
                        efAll         = 0xFFFF
                      };

    cEventFilterBase(void) {};
    virtual ~cEventFilterBase(void) {};
    virtual bool Matches(const cEvent *event) const = 0;
  };

  // Type returned by the filter file parser
  class cEventFilter : public cEventFilterBase
  {
  public:
    enum eFilterActions { faRecord   = 1,
                          faInactive = 2
                        };

    cEventFilter(const char *Name, eFilterActions Action, const cEventFilterBase *Filter);
    virtual ~cEventFilter(void) {};
    virtual bool Matches(const cEvent *event) const;

    const char *Name() const;
    eFilterActions Action() const;

    bool AuxMatches(const cTimer *timer) const;
    cTimer *CreateTimer(const cEvent *event) const;
    bool UpdateTimer(cTimer *timer, const cEvent *event) const;

  private:
    cString _name;
    eFilterActions _action;
    const cEventFilterBase *_filter;
  };

  // base class for "and" and "or" filter
  // cleans up the filter list
  class cEventFilterList : public cEventFilterBase
  {
  public:
    cEventFilterList(cList<cEventFilterBase> *filters);
    virtual ~cEventFilterList(void);
    virtual bool Matches(const cEvent *event) const;

  protected:
    cList<cEventFilterBase> *_filters;
  };

  // all filters in the list must match
  class cEventFilterAnd : public cEventFilterList
  {
  public:
    // cEventFilterAnd will take control over "filters"
    cEventFilterAnd(cList<cEventFilterBase> *filters);
    virtual ~cEventFilterAnd(void) {};
    virtual bool Matches(const cEvent *event) const;
  };

  // at least one filter in the list must match
  class cEventFilterOr : public cEventFilterList
  {
  public:
    // cEventFilterOr will take control over "filters"
    cEventFilterOr(cList<cEventFilterBase> *filters);
    virtual ~cEventFilterOr(void) {};
    virtual bool Matches(const cEvent *event) const;
  };

  // The event must belong to a channel between fromChannel and toChannel.
  // The channel numbers are used for comparison.
  // If fromChannel is the invalid channel-id, all channels with numbers up
  // to toChannel's number match.
  // If toChannel is the invalid channel-id, all channels with numbers
  // from fromChannel's number match.
  class cEventFilterChannel : public cEventFilterBase
  {
  public:
    cEventFilterChannel(tChannelID fromChannel, tChannelID toChannel);
    virtual ~cEventFilterChannel(void) {};
    virtual bool Matches(const cEvent *event) const;

  private:
    tChannelID _fromChannel;
    tChannelID _toChannel;
  };

  // Matches, if the title, shorttext or description contains
  // the given text.
  // TODO
  // - remove special characters from needle and text like '" etc.
  //   to stabilize matching
  class cEventFilterContains : public cEventFilterBase
  {
  public:
    cEventFilterContains(const char *needle, int fields);
    virtual ~cEventFilterContains(void) {};
    virtual bool Matches(const cEvent *event) const;

  private:
    cString _needle;
    int _fields;

    bool Matches(const char *text) const;
  };

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

      cTagFilter(const char *tag, eTagFilterOperator op, const char *comp);
      virtual ~cTagFilter(void) {};

      bool Matches(const char *tag, const char *value) const;
      const char *Tag(void) const { return *_tag; };

    private:
      cString _tag;
      eTagFilterOperator _op;
      int _intComp;
      cString _strComp;
      uint _strCompLen;
    };

    cEventFilterTag(cList<cTagFilter> *tagFilters);
    virtual ~cEventFilterTag(void);
    virtual bool Matches(const cEvent *event) const;

  private:
    cList<cTagFilter> *_tagFilters;
  };
}

#endif
