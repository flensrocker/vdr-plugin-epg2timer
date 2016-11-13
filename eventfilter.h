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

  // TODO cEventFilterDescTag (find tag in description and match contents, e.g. season, year, actor etc.)
}

#endif
