#ifndef epg2timer_filtertools_h
#define epg2timer_filtertools_h

#include "eventfilter.h"

namespace epg2timer
{
  // Instantiate with an string like "key1=value1,key2=value2,key2=value3,key3=value4,...".
  // Comma and backslashes in values must be escaped with a backslash.
  // Keys must not contain the equal sign.
  class cParameterParser
  {
  public:
    cParameterParser(const char *Parameters);

    // If Number is greater than zero, returns the "Number + 1"'th appearance of the parameter (zero based index).
    const char *Get(const char *Name, int Number = 0) const;
    // Get the number of appearances of parameter "Name".
    int Count(const char *Name) const;

  private:
    cStringList  _list;
  };

  class cFilterTools
  {
  public:
    // Channel filter-line:
    // type=channel,from=channel-id,to=channel-id
    // if "from" or "to" are omitted, they default to the invalid channel-id

    // Contains filter-line:
    // type=contains,search=needle,field=all,field=title,field=shorttext,field=description
    // "field" is optional, default is "all"

    // And filter:
    // type=and {
    //   filter-line
    //   filter-line
    //   ...
    // }

    // Or filter:
    // type=or {
    //   filter-line
    //   filter-line
    //   ...
    // }

    // file format:
    // # The file must follow the rules for nested vdr config files.
    // # The text on the first level is the name of the filter.
    // # A filter-line starts with "type=".
    // # The default action of a filter is "record".
    // # Possible actions are: record, inactive
    //
    // # Create inactive timers for all "Star Trek" events on all channels
    // Star Trek {
    //   type=contains,search=star trek,field=title
    //   action=inactive
    // }
    // # Record all "Journal" and "Reportage" events on arte.tv
    // ARTE Journal/Reportage {
    //   type=and {
    //     type=channel,from=C-1-1051-28724,to=C-1-1051-28724
    //     type=or {
    //       type=contains,search=arte journal,field=title
    //       type=contains,search=arte reportage,field=title
    //     }
    //   }
    //   action=record
    // }
    static cEventFilterBase *ParseFilterLine(const char *FilterLine, cList<cNestedItem> *SubItems);
    static bool LoadFilterFile(const char *FileName, cList<cEventFilter> &Filters);
  };
}

#endif
