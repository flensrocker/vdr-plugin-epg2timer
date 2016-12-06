#include "filterfile.h"

#include <vdr/config.h>

#include "filters/andor.h"
#include "filters/channel.h"
#include "filters/contains.h"
#include "filters/starttime.h"
#include "filters/tag.h"

#include "eventfilter.h"
#include "tools/filtercontext.h"
#include "tools/parameterparser.h"
#include "tools/stringconverter.h"
#include "tools/tagsynonym.h"
#include "tools/timer.h"


namespace epg2timer
{
  static cEventFilterTag::cTagFilter *ParseTagFilter(const cFilterContext& Context, const char *Tag, const char *Value, bool Missing)
  {
    if ((Tag == NULL) || (*Tag == 0) || (Value == NULL) || (*Value == 0))
       return NULL;

    int valueLen = strlen(Value);
    if (valueLen < 5)
       return NULL;

    cEventFilterTag::cTagFilter::eTagFilterOperator op = cEventFilterTag::cTagFilter::tfoInvalid;
    cString comp;

    // parse longest operators first!
    if (startswith(Value, "int")) {
       int compOffset = 5;
       if (startswith(Value + 3, "==")) {
          op = cEventFilterTag::cTagFilter::tfoIntEqual;
          }
       else if (startswith(Value + 3, "!=")) {
          op = cEventFilterTag::cTagFilter::tfoIntNotEqual;
          }
       else if (startswith(Value + 3, "<=")) {
          op = cEventFilterTag::cTagFilter::tfoIntLesserOrEqual;
          }
       else if (startswith(Value + 3, ">=")) {
          op = cEventFilterTag::cTagFilter::tfoIntGreaterOrEqual;
          }
       else if (startswith(Value + 3, "<")) {
          op = cEventFilterTag::cTagFilter::tfoIntLesser;
          compOffset = 4;
          }
       else if (startswith(Value + 3, ">")) {
          op = cEventFilterTag::cTagFilter::tfoIntGreater;
          compOffset = 4;
          }
       else {
          esyslog("epg2timer: invalid tag operator in %s", Value);
          return NULL;
          }
       comp = cString(skipspace(Value + compOffset));
       }
    else if (startswith(Value, "str")) {
       int compOffset = 5;
       if (startswith(Value + 3, "notcontains")) {
          op = cEventFilterTag::cTagFilter::tfoStrNotContains;
          compOffset = 14;
          }
       else if (startswith(Value + 3, "startswith")) {
          op = cEventFilterTag::cTagFilter::tfoStrStartswith;
          compOffset = 13;
          }
       else if (startswith(Value + 3, "notempty")) {
          op = cEventFilterTag::cTagFilter::tfoStrIsNotEmpty;
          compOffset = 11;
          }
       else if (startswith(Value + 3, "contains")) {
          op = cEventFilterTag::cTagFilter::tfoStrContains;
          compOffset = 11;
          }
       else if (startswith(Value + 3, "endswith")) {
          op = cEventFilterTag::cTagFilter::tfoStrEndswith;
          compOffset = 11;
          }
       else if (startswith(Value + 3, "empty")) {
          op = cEventFilterTag::cTagFilter::tfoStrIsEmpty;
          compOffset = 8;
          }
       else if (startswith(Value + 3, "==")) {
          op = cEventFilterTag::cTagFilter::tfoStrEqual;
          }
       else if (startswith(Value + 3, "!=")) {
          op = cEventFilterTag::cTagFilter::tfoStrNotEqual;
          }
       else if (startswith(Value + 3, "<=")) {
          op = cEventFilterTag::cTagFilter::tfoStrLesserOrEqual;
          }
       else if (startswith(Value + 3, ">=")) {
          op = cEventFilterTag::cTagFilter::tfoStrGreaterOrEqual;
          }
       else if (startswith(Value + 3, "<")) {
          op = cEventFilterTag::cTagFilter::tfoStrLesser;
          compOffset = 4;
          }
       else if (startswith(Value + 3, ">")) {
          op = cEventFilterTag::cTagFilter::tfoStrGreater;
          compOffset = 4;
          }
       else {
          esyslog("epg2timer: invalid tag operator in %s", Value);
          return NULL;
          }
       comp = Context.Converter()->Convert(skipspace(Value + compOffset));
       }
    else {
       esyslog("epg2timer: invalid tag type in %s", Value);
       return NULL;
       }

    return new cEventFilterTag::cTagFilter(Context, Tag, op, *comp, Missing);
  }

  static cEventFilterBase *ParseFilterLine(const cFilterContext& Context, const char *FilterLine, cList<cNestedItem> *SubItems)
  {
    if ((FilterLine == NULL) || (*FilterLine == 0))
       return NULL;

    cParameterParser parser(FilterLine);
    const char *type = parser.Get("type");

    if ((strcmp(type, "and") == 0) || (strcmp(type, "or") == 0)) {
       if ((SubItems == NULL) || (SubItems->Count() == 0))
          esyslog("epg2timer: error while parsing filter \"%s\": missing subitems", type);
       else {
          cList<cEventFilterBase> *subFilters = NULL;
          for (cNestedItem *si = SubItems->First(); si; si = SubItems->Next(si)) {
              cEventFilterBase *subFilter = ParseFilterLine(Context, si->Text(), si->SubItems());
              if (subFilter != NULL) {
                 if (subFilters == NULL)
                    subFilters = new cList<cEventFilterBase>();
                 subFilters->Add(subFilter);
                 }
              }
          if (subFilters != NULL) {
             if (strcmp(type, "and") == 0)
                return new cEventFilterAnd(subFilters);
             else
                return new cEventFilterOr(subFilters);
             }
          }
       }
    else if (strcmp(type, "channel") == 0) {
       tChannelID from = tChannelID::InvalidID;
       tChannelID to = tChannelID::InvalidID;

       const char *fromText = parser.Get("from");
       if (fromText != NULL)
          from = tChannelID::FromString(fromText);
       const char *toText = parser.Get("to");
       if (toText != NULL)
          to = tChannelID::FromString(toText);

       return new cEventFilterChannel(from, to);
       }
    else if (startswith(type, "contains")) {
       bool containsnot = startswith(type + 8, "not");
       const char *needle = parser.Get("search");
       if ((needle == NULL) || (*needle == 0))
          return NULL;

       int fields = cEventFilter::efAll;
       int fieldCount = parser.Count("field");
       if (fieldCount > 0) {
          fields = 0;
          for (int i = 0; i < fieldCount; i++) {
              const char *field = parser.Get("field", i);
              if (field != NULL) {
                 if (strcmp(field, "title") == 0)
                    fields |= cEventFilter::efTitle;
                 else if (strcmp(field, "shorttext") == 0)
                    fields |= cEventFilter::efShortText;
                 else if (strcmp(field, "description") == 0)
                    fields |= cEventFilter::efDescription;
                 else if (strcmp(field, "all") == 0)
                    fields |= cEventFilter::efAll;
                 }
              }
          }
       return new cEventFilterContains(Context, needle, fields, containsnot);
       }
    else if (strcmp(type, "starttime") == 0) {
       const char *after = parser.Get("after");
       const char *before = parser.Get("before");
       return new cEventFilterStartTime(after, before);
       }
    else if (strcmp(type, "tag") == 0) {
       const char *m = parser.Get("missing");
       bool missing = (m != NULL) && (strcasecmp(m, "true") == 0);

       cList<cEventFilterTag::cTagFilter> *tagFilters = NULL;

       cString tag;
       for (int i = 0; i < parser.Size(); i++) {
           const char *value = parser.At(i, tag);
           if ((strcmp(*tag, "type") == 0) || (strcmp(*tag, "missing") == 0))
              continue;

           cEventFilterTag::cTagFilter *tagFilter = ParseTagFilter(Context, *tag, value, missing);
           if (tagFilter != NULL) {
              if (tagFilters == NULL)
                 tagFilters = new cList<cEventFilterTag::cTagFilter>();
              tagFilters->Add(tagFilter);
              }
           }

       if (tagFilters == NULL)
          return NULL;
       return new cEventFilterTag(tagFilters, missing);
       }

    return NULL;
  }
}


epg2timer::cFilterFile *epg2timer::cFilterFile::Load(const char *Filename)
{
  cFilterFile *filterFile = new cFilterFile(Filename);

  if (!filterFile->Load()) {
     delete filterFile;
     return NULL;
     }

  return filterFile;
}


epg2timer::cFilterFile::cFilterFile(const char *Filename)
{
  _filename = Filename;
  _lastModTime = LastModifiedTime(Filename);
  _lastUpdateAt = 0;
  _updateIntervalMin = 10;
  _context = new cFilterContext();
  _filters = new cList<cEventFilter>();

  SetDescription("epg2timer-update");
}


epg2timer::cFilterFile::~cFilterFile(void)
{
  Cancel(5);
  delete _filters;
  delete _context;
}


void epg2timer::cFilterFile::UpdateTimers(bool Force)
{
  bool needsUpdate = Force;

  time_t modTime = LastModifiedTime(*_filename);
  if (modTime == 0)
     esyslog("epg2timer: can't get last modified time of %s", *_filename);
  else if (_lastModTime != modTime) {
     isyslog("epg2timer: reloading file %s", *_filename);
     _lastModTime = modTime;
     if (!Load())
        esyslog("epg2timer: can't reload file %s, keeping current filters", *_filename);
     needsUpdate = true;
     }

  if (!needsUpdate) {
     time_t now = time(NULL);
     if ((_lastUpdateAt == 0) || (_lastUpdateAt + _updateIntervalMin * 60 < now)) {
        _lastUpdateAt = now;
        needsUpdate = true;
        }
     }

  if (needsUpdate)
     Start();
}


bool epg2timer::cFilterFile::Load(void)
{
  Lock();

  cNestedItemList filters;
  if (!filters.Load(*_filename))
     return false;

  cFilterContext *newContext = new cFilterContext();
  cList<cEventFilter> *newFilters = new cList<cEventFilter>();
  int newUpdateIntervalMin = _updateIntervalMin;

  for (cNestedItem *item = filters.First(); item; item = filters.Next(item)) {
      cList<cNestedItem> *subitems = item->SubItems();

      // no subitems => global parameters
      if ((subitems == NULL) || (subitems->Count() == 0)) {
         cParameterParser globalParser(item->Text());
         const char *updateIntervalMin = globalParser.Get("updateIntervalMin");
         if ((updateIntervalMin != 0) && (*updateIntervalMin != 0) && isnumber(updateIntervalMin)) {
            newUpdateIntervalMin = atoi(updateIntervalMin);
            if (newUpdateIntervalMin < 1)
               newUpdateIntervalMin = 1; // one minute minimum
            }

         int tsCount = globalParser.Count("tagsynonym");
         for (int t = 0; t < tsCount; t++) {
            const char *tagSynonyms = globalParser.Get("tagsynonym", t);
            if ((tagSynonyms != NULL) && (*tagSynonyms != 0)) {
               isyslog("epg2timer: tag synonyms: \"%s\"", tagSynonyms);
               cStringList *synonyms = new cStringList();
               char *tsOrig = strdup(tagSynonyms);
               char *ts = tsOrig;
               char *s;
               char *next;
               while ((s = strtok_r(ts, ":", &next)) != NULL) {
                     isyslog("epg2timer: tag synonym \"%s\"", s);
                     synonyms->Append(strdup(s));
                     ts = NULL;
                     }
               newContext->AddTagSynonyms(new cTagSynonym(synonyms));
               free(tsOrig);
               }
            }
         continue;
         }

      // subitems => it's a filter
      const char *filterName = item->Text();
      if ((filterName == NULL) || (*filterName == 0))
         continue;

      cEventFilter::eFilterActions action = cEventFilter::faRecord;
      cString filename;
      int marginStart = -1;
      int marginStop = -1;
      int priority = Setup.DefaultPriority;
      int lifetime = Setup.DefaultLifetime;
      cEventFilterBase *filter = NULL;

      for (cNestedItem *subitem = subitems->First(); subitem; subitem = subitems->Next(subitem)) {
          const char *line = subitem->Text();
          if ((line == NULL) || (*line == 0))
             continue;

          if (startswith(line, "type=")) {
             if (filter != NULL)
                esyslog("epg2timer: error while loading filters: multiple types");
             else
                filter = ParseFilterLine(*newContext, line, subitem->SubItems());
             }
          else {
             cParameterParser lineParser(line);
             const char *value;

             value = lineParser.Get("action");
             if (value != NULL) {
                if (strcmp(value, "record") == 0)
                   action = cEventFilter::faRecord;
                else if (strcmp(value, "inactive") == 0)
                   action = cEventFilter::faInactive;
                }

             value = lineParser.Get("filename");
             if ((value != NULL) && (*value != 0))
                filename = value;

             value = lineParser.Get("marginStart");
             if ((value != 0) && (*value != 0) && isnumber(value))
                marginStart = atoi(value);

             value = lineParser.Get("marginStop");
             if ((value != 0) && (*value != 0) && isnumber(value))
                marginStop = atoi(value);

             value = lineParser.Get("priority");
             if ((value != 0) && (*value != 0) && isnumber(value))
                priority = atoi(value);

             value = lineParser.Get("lifetime");
             if ((value != 0) && (*value != 0) && isnumber(value))
                lifetime = atoi(value);
             }
          }

      if (filter != NULL)
         newFilters->Add(new cEventFilter(*newContext, filterName, action, *filename, filter, marginStart, marginStop, priority, lifetime));
      }

  if (newFilters->Count() == 0) {
     esyslog("epg2timer: no filters in file %s", *_filename);
     delete newFilters;
     delete newContext;
     return false;
     }

  delete _filters;
  delete _context;
  _context = newContext;
  _filters = newFilters;
  _updateIntervalMin = newUpdateIntervalMin;

  Unlock();
  return true;
}


void epg2timer::cFilterFile::Action(void)
{
  Lock();

  // Order of locks:
  // Timers, Channels, Recordings, Schedules

  // get write lock on timer
  // and read locks on channels and schedules
#if APIVERSNUM < 20301
  cTimers *timers = &Timers;
  bool hasTimerWriteLock = !Timers.BeingEdited();
  timers->IncBeingEdited();

  _context->SetChannels(&Channels);

  cSchedulesLock schedulesLock;
  const cSchedules *schedules = cSchedules::Schedules(schedulesLock);
#else
  LOCK_TIMERS_WRITE;
  cTimers *timers = Timers;
  bool hasTimerWriteLock = true;
  timers->SetExplicitModify();

  LOCK_CHANNELS_READ;
  _context->SetChannels(Channels);

  LOCK_SCHEDULES_READ;
  const cSchedules *schedules = Schedules;
#endif

  int eventCount = 0;
  int foundCount = 0;
  int updatedCount = 0;
  int createdCount = 0;
  uint64_t start = cTimeMs::Now();

  if (schedules) {
     time_t now = time(NULL);

     for (const cSchedule *s = schedules->First(); Running() && s; s = schedules->Next(s)) {
         const cList<cEvent> *events = s->Events();
         if (events) {
            for (const cEvent *e = events->First(); Running() && e; e = events->Next(e)) {
                if (e->EndTime() < now)
                   continue;

                eventCount++;
                for (const cEventFilter *filter = _filters->First(); Running() && filter; filter = _filters->Next(filter)) {
                    if (filter->Matches(*_context, e)) {
                       foundCount++;
                       if (hasTimerWriteLock) {
                          cTimer *t = cTimerTools::FindTimer(timers, schedules, e);
                          if (t) {
                             if (filter->UpdateTimer(t, e)) {
                                updatedCount++;
                                timers->SetModified();
                                }
                             }
                          else {
                             cTimer *nt = filter->CreateTimer(e);
                             if (nt != NULL) {
                                timers->Add(nt);
                                timers->SetModified();
                                createdCount++;
                                }
                             }
                          }
                       }
                   }
                }
            }
         }
     }

#if APIVERSNUM < 20301
  // release write-lock on timer
  timers->DecBeingEdited();
#endif
  _context->SetChannels(NULL);

  uint64_t end = cTimeMs::Now();
  isyslog("epg2timer: %d events parsed, %d matched, %d timers updated, %d timers created in %lums", eventCount, foundCount, updatedCount, createdCount, end - start);

  Unlock();
}
