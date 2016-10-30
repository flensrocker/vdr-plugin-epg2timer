#include "epgtools.h"


epg2timer::cEpgTestHandler::cEpgTestHandler()
{
  _startOffset = 0;
}

epg2timer::cEpgTestHandler::~cEpgTestHandler()
{
}

bool epg2timer::cEpgTestHandler::SetDescription(cEvent *Event, const char *Description)
{
  if (_startOffset != 0) {
     cString desc = cString::sprintf("%s\nStart-Offset: %d", Description, _startOffset);
     Event->SetDescription(*desc);
     }
  else
     Event->SetDescription(Description);
  return true;
}

bool epg2timer::cEpgTestHandler::SetStartTime(cEvent *Event, time_t StartTime)
{
  Event->SetStartTime(StartTime + 60 * _startOffset);
  return true;
}

bool epg2timer::cEpgTestHandler::SetVps(cEvent *Event, time_t Vps)
{
  Event->SetVps(0);
  return true;
}

void epg2timer::cEpgTestHandler::SetStartOffset(int Offset)
{
  _startOffset = Offset;
}
