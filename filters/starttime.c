#include "starttime.h"


static cString PadTimeString(const char *Time, const char *Default)
{
  cString result;
  if ((Time == NULL) || (*Time == 0))
     result = Default;
  else {
     result = Time;
     int len = strlen(Time);
     if (len > 4)
        result.Truncate(4);
     else {
       while (len < 4) {
             result = cString::sprintf("0%s", *result);
             len++;
             }
        }
     }
  return result;
}


epg2timer::cEventFilterStartTime::cEventFilterStartTime(const char *After, const char *Before)
{
  _after = PadTimeString(After, "0000");
  _before = PadTimeString(Before, "2359");
  _mode = strcmp(*_after, *_before);
}

bool epg2timer::cEventFilterStartTime::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if (Event == NULL)
     return false;

  bool matches = true;

  char startTime[5];
  struct tm tm_st;
  time_t st = Event->StartTime();
  strftime(startTime, sizeof(startTime), "%H%M", localtime_r(&st, &tm_st));
  startTime[4] = 0;

  if (_mode < 0) {
     // after (== "0100") and before (== "0300") on same day (after < before):
     // after <= startTime && startTime <= before
     matches = (strcmp(*_after, startTime) <= 0) && (strcmp(startTime, *_before) <= 0);
     }
  else if (_mode > 0) {
     // after (== "2300") and before (== "0100") on different days (after > before):
     // no match with "2200" or "0200":
     // match with "2315", "0000", "0050":
     //    after <= startTime && startTime <= "2359"
     // || "0000" <= startTime && startTime <= before
     matches = ((strcmp(*_after, startTime) <= 0) && (strcmp(startTime, "2359") <= 0))
            || ((strcmp("0000", startTime) <= 0) && (strcmp(startTime, *_before) <= 0));
     }
  else {
     // after == before
     matches = (strcmp(*_after, startTime) == 0);
     }

  return matches;
}
