#include "filterparser.h"

#include "eventfilter.h"
#include "parameterparser.h"


namespace epg2timer
{
  static cEventFilterBase *ParseFilterLine(const char *FilterLine, cList<cNestedItem> *SubItems)
  {
    if ((FilterLine == NULL) || (*FilterLine == 0))
       return NULL;

    cParameterParser parser(FilterLine);
    const char *type = parser.Get("type");

    if ((strcmp(type, "and") == 0) || (strcmp(type, "or") == 0)) {
       if ((SubItems == NULL) || (SubItems->Count() == 0))
          esyslog("epg2timer, parse filter \"%s\": missing subtimes", type);
       else {
          cList<cEventFilterBase> *subFilters = NULL;
          for (cNestedItem *si = SubItems->First(); si; si = SubItems->Next(si)) {
              cEventFilterBase *subFilter = ParseFilterLine(si->Text(), si->SubItems());
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
    else if (strcmp(type, "contains") == 0) {
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
       return new cEventFilterContains(needle, fields);
       }
    else if (strcmp(type, "tag") == 0) {
       //return new cEventFilterTag();
       }
    
    return NULL;
  }
}


bool epg2timer::cFilterParser::LoadFilterFile(const char *FileName, cList<cEventFilter> &Filters)
{
  Filters.Clear();

  cNestedItemList filters;
  if (!filters.Load(FileName))
     return false;

  for (cNestedItem *item = filters.First(); item; item = filters.Next(item)) {
      cList<cNestedItem> *subitems = item->SubItems();
      if ((subitems == NULL) || (subitems->Count() == 0))
         continue;

      const char *filterName = item->Text();
      if ((filterName == NULL) || (*filterName == 0))
         continue;

      cEventFilter::eFilterActions action = cEventFilter::faRecord;
      cString filename;
      cEventFilterBase *filter = NULL;

      for (cNestedItem *subitem = subitems->First(); subitem; subitem = subitems->Next(subitem)) {
          const char *line = subitem->Text();
          if ((line == NULL) || (*line == 0))
             continue;

          if (startswith(line, "type=")) {
             if (filter != NULL)
                esyslog("epg2timer, load filters: multiple types");
             else
                filter = ParseFilterLine(line, subitem->SubItems());
             }
          else if (startswith(line, "action=")) {
             cParameterParser actionParser(line);
             const char *actionText = actionParser.Get("action");
             if (actionText != NULL) {
                if (strcmp(actionText, "record") == 0)
                   action = cEventFilter::faRecord;
                else if (strcmp(actionText, "inactive") == 0)
                   action = cEventFilter::faInactive;
                }
             }
          else if (startswith(line, "filename=")) {
             cParameterParser filenameParser(line);
             const char *filenameText = filenameParser.Get("filename");
             if ((filenameText != NULL) && (*filenameText != 0))
                filename = filenameText;
             }
          }

      if (filter != NULL)
         Filters.Add(new cEventFilter(filterName, action, *filename, filter));
      }

  return true;
}
