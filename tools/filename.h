#ifndef epg2timer_tools_filename_h
#define epg2timer_tools_filename_h

#include <vdr/epg.h>


namespace epg2timer
{
  class cFilenameTools
  {
  public:
    static cString ReplaceTags(const char *Filename, const cEvent *Event);
  };
}

#endif
