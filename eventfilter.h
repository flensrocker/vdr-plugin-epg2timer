#ifndef epg2timer_eventfilter_h
#define epg2timer_eventfilter_h

#include <vdr/epg.h>

namespace epg2timer
{
  class cEventFilter : public cListObject
  {
  public:
    enum eEventFields { efTitle       = 0x0001,
                        efShortText   = 0x0002,
                        efDescription = 0x0004,
                        efAll         = 0xFFFF
                      };

    static void MoveFilters(cList<cEventFilter> &from, cList<cEventFilter> &to);

    virtual ~cEventFilter(void) {};
    virtual bool Matches(const cEvent *event) const = 0;
  };

  // all filters in the list must match
  class cEventFilterAnd : public cEventFilter
  {
  public:
    // filters will be empty afterwards,
    // cEventFilterAnd will take control over cEventFilter objects
    cEventFilterAnd(cList<cEventFilter> &filters);
    virtual ~cEventFilterAnd(void) {};
    virtual bool Matches(const cEvent *event) const;

  private:
    cList<cEventFilter> _filters;
  };

  // at least one filter in the list must match
  class cEventFilterOr : public cEventFilter
  {
  public:
    // filters will be empty afterwards,
    // cEventFilterOr will take control over cEventFilter objects
    cEventFilterOr(cList<cEventFilter> &filters);
    virtual ~cEventFilterOr(void) {};
    virtual bool Matches(const cEvent *event) const;

  private:
    cList<cEventFilter> _filters;
  };

  // The event must belong to a channel between fromChannel and toChannel.
  // The channel numbers are used for comparison.
  // If fromChannel is the invalid channel-id, all channels with numbers up
  // to toChannel's number match.
  // If toChannel is the invalid channel-id, all channels with numbers
  // from fromChannel's number match.
  class cEventFilterChannel : public cEventFilter
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
  class cEventFilterContains : public cEventFilter
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
