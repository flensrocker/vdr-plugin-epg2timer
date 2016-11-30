#ifndef epg2timer_eventfilter_h
#define epg2timer_eventfilter_h

#include <vdr/epg.h>
#include <vdr/timers.h>

namespace epg2timer
{
  class cFilterContext;

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
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const = 0;
  };

  // Type returned by the file parser.
  class cEventFilter : public cEventFilterBase
  {
  public:
    enum eFilterActions { faRecord   = 1,
                          faInactive = 2
                        };

    cEventFilter(const char *Name, eFilterActions Action, const char *Filename, const cEventFilterBase *Filter, int MarginStart, int MarginStop, int Priority, int Lifetime);
    virtual ~cEventFilter(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;

    const char *Name() const;
    eFilterActions Action() const;

    bool AuxMatches(const cTimer *Timer) const;
    void CalculateStartStopDay(cTimer *Timer, const cEvent *Event, int *Start, int *Stop, time_t *Day) const;
    cTimer *CreateTimer(const cEvent *Event) const;
    bool UpdateTimer(cTimer *Timer, const cEvent *Event) const;

  private:
    cString _name;
    eFilterActions _action;
    cString _filename;
    int _marginStart;
    int _marginStop;
    int _priority;
    int _lifetime;
    const cEventFilterBase *_filter;
  };

  // Base class for "And" and "Or" filter.
  // Deletes the provided "Filters" list.
  class cEventFilterList : public cEventFilterBase
  {
  public:
    cEventFilterList(cList<cEventFilterBase> *Filters);
    virtual ~cEventFilterList(void);
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;

  protected:
    cList<cEventFilterBase> *_filters;
  };

  // All filters in the list must match.
  class cEventFilterAnd : public cEventFilterList
  {
  public:
    cEventFilterAnd(cList<cEventFilterBase> *Filters);
    virtual ~cEventFilterAnd(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;
  };

  // At least one filter in the list must match.
  class cEventFilterOr : public cEventFilterList
  {
  public:
    cEventFilterOr(cList<cEventFilterBase> *Filters);
    virtual ~cEventFilterOr(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;
  };

  // The event must belong to a channel between FromChannel and ToChannel.
  // The channel numbers are used for comparison.
  // If FromChannel is the invalid channel-id, all channels with numbers up
  // to ToChannel's number match.
  // If ToChannel is the invalid channel-id, all channels with numbers
  // from FromChannel's number match.
  class cEventFilterChannel : public cEventFilterBase
  {
  public:
    cEventFilterChannel(tChannelID FromChannel, tChannelID ToChannel);
    virtual ~cEventFilterChannel(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;

  private:
    tChannelID _fromChannel;
    tChannelID _toChannel;
  };

  // Matches, if the title, shorttext or description contains the given text.
  class cEventFilterContains : public cEventFilterBase
  {
  public:
    cEventFilterContains(const cFilterContext& Context, const char *Needle, int Fields);
    virtual ~cEventFilterContains(void) {};
    virtual bool Matches(const cFilterContext& Context, const cEvent *Event) const;

  private:
    cString _needle;
    int _fields;

    bool Matches(const cFilterContext& Context, const char *Text) const;
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
