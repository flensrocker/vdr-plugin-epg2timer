#ifndef epg2timer_filenametools_h
#define epg2timer_filenametools_h

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
