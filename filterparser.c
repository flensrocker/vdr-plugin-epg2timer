#include "filterparser.h"

#include "eventfilter.h"
#include "filtercontext.h"
#include "parameterparser.h"


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
          esyslog("epg2timer, parse filter \"%s\": missing subtimes", type);
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
       return new cEventFilterTag(tagFilters);
       }
    
    return NULL;
  }
}


bool epg2timer::cFilterParser::LoadFilterFile(const cFilterContext& Context, const char *Filename, cList<cEventFilter> &Filters)
{
  Filters.Clear();

  cNestedItemList filters;
  if (!filters.Load(Filename))
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
                filter = ParseFilterLine(Context, line, subitem->SubItems());
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
