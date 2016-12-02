#ifndef epg2timer_eventfilter_h
#define epg2timer_eventfilter_h

#include "filterbase.h"


namespace epg2timer
{
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
}

#endif
