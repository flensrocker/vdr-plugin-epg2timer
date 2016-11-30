#ifndef epg2timer_filterfile_h
#define epg2timer_filterfile_h

#include <vdr/thread.h>
#include <vdr/tools.h>

namespace epg2timer
{
  class cEventFilter;
  class cFilterContext;

  class cFilterFile : public cThread
  {
  public:
    // Channel filter-line:
    // type=channel,from=channel-id,to=channel-id
    // if "from" or "to" are omitted, they default to the invalid channel-id

    // Contains filter-line:
    // type=contains,search=needle,field=all,field=title,field=shorttext,field=description
    // "field" is optional, default is "all"

    // Tag filter-line:
    // type=tag[,missing=true|false],<tagname>=[int|str]<op><value>,<tagname>=[int|str]<op><value>,...
    //   missing: default is "false". With "true" a tag matches if it's not present in the description.
    //            Usefull, if there's a problem with the EPG provider and not all values are given.
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
    // # Values can be formatted with leading zeros like %Staffel:2%.
    // # Custom leading characters can be provided like %Staffel:2,x%.
    // # TODO custom margin start/stop, priority, lifetime
    //
    // # global parameter
    // updateIntervalMin=10
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
    static cFilterFile *Load(const char *Filename);

    ~cFilterFile(void);

    int FilterCount(void) const { return _filters->Count(); };
    void UpdateTimers(bool Force);

  protected:
    virtual void Action(void);

  private:
    cFilterFile(const char *Filename);

    bool Load(void);

    cString _filename;
    time_t _lastModTime;
    time_t _lastUpdateAt;
    int _updateIntervalMin;
    cFilterContext *_context;
    cList<cEventFilter> *_filters;
  };
}

#endif
