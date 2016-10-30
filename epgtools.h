#ifndef epg2timer_epgtools_h
#define epg2timer_epgtools_h

#include <vdr/epg.h>

namespace epg2timer
{
  class cEpgTestHandler : public cEpgHandler
  {
  private:
    int _startOffset; // minutes

  public:
    cEpgTestHandler(void);
    virtual ~cEpgTestHandler(void);
    virtual bool SetDescription(cEvent *Event, const char *Description);
    virtual bool SetStartTime(cEvent *Event, time_t StartTime);
    virtual bool SetVps(cEvent *Event, time_t Vps);

    void SetStartOffset(int Offset);
  };
}

#endif
