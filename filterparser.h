#ifndef epg2timer_filterparser_h
#define epg2timer_filterparser_h

#include <vdr/config.h>

namespace epg2timer
{
  class cEventFilterBase;
  class cEventFilter;
  class cFilterContext;

  class cFilterParser
  {
  public:
    // Channel filter-line:
    // type=channel,from=channel-id,to=channel-id
    // if "from" or "to" are omitted, they default to the invalid channel-id

    // Contains filter-line:
    // type=contains,search=needle,field=all,field=title,field=shorttext,field=description
    // "field" is optional, default is "all"

    // Tag filter-line:
    // type=tag,<tagname>=[int|str]<op><value>,<tagname>=[int|str]<op><value>,...
    //   int-ops: ==, !=, <, <=, >, >=
    //   str-ops: ==, !=, <, <=, >, >=, empty, notempty, contains, notcontains, startswith, endswith
    //   If multiple tags are given, all must match.
    // examples:
    //   find all events with a season number greater or equal to 4 and lesser then 7:
    //   type=tag,Staffel=int>=4,Staffel=int<7
    //   find all news events:
    //   type=tag,Genre=str==Nachrichten
    //   find the episode with "Stein, Schere, Spock" in it:
    //   type=tag,Episode=strcontainsStein\, Schere\, Spock

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
    // # A filename pattern can be set with a "filename=" line.
    // # Beside %title% and %shorttext% you can use any tag-value from the description.
    // # Integer values can be formatted with leading zeros like %Staffel:2%.
    //
    // # Create inactive timers for all "Star Trek" events on all channels
    // Star Trek {
    //   type=contains,search=star trek,field=title
    //   action=inactive
    //   filename=%title%~Staffel_%Staffel:2%~%Staffelfolge:2%_-_%shorttext%
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
    static bool LoadFilterFile(const cFilterContext& Context, const char *Filename, cList<cEventFilter> &Filters);
  };
}

#endif