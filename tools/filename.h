#ifndef epg2timer_tools_filename_h
#define epg2timer_tools_filename_h

#include <vdr/epg.h>


namespace epg2timer
{
  class cFilterContext;

  class cFilenameTools
  {
  public:
    static cString ReplaceTags(const cFilterContext& Context, const char *Filename, const cEvent *Event);
  };
}

#endif
