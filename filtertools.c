#include "filtertools.h"


epg2timer::cParameterParser::cParameterParser(const char *Parameters)
{
  if (Parameters != NULL) {
     char *s = strdup(Parameters);
     size_t len = strlen(Parameters);
     size_t pos1 = 0;
     dsyslog("epg2timer, parameter parser: parameters = '%s'", s);
     // skip initial commas
     while ((pos1 < len) && (s[pos1] == ','))
           pos1++;
     size_t pos2 = pos1;
     while (pos2 < len) {
           // de-escape backslashed characters
           if ((s[pos2] == '\\') && (pos2 < (len - 1))) {
              memmove(s + pos2, s + pos2 + 1, len - pos2 - 1);
              len--;
              s[len] = 0;
              pos2++;
              continue;
              }
           if ((s[pos2] == ',') && (s[pos2 - 1] != '\\')) {
              s[pos2] = 0;
              dsyslog("epg2timer, parameter parser: add '%s'", s + pos1);
              _list.Append(strdup(s + pos1));
              pos1 = pos2 + 1;
              }
           pos2++;
           }
     if (pos2 > pos1) {
        dsyslog("epg2timer, parameter parser: add '%s'", s + pos1);
        _list.Append(strdup(s + pos1));
        }
     free(s);
     }
}


const char *epg2timer::cParameterParser::Get(const char *Name, int Number) const
{
  if (Name == NULL)
     return NULL;
  size_t name_len = strlen(Name);
  if (name_len == 0)
     return NULL;

  for (int i = 0; i < _list.Size(); i++) {
      const char *text = _list[i];
      if (text == NULL)
         continue;
      size_t len = strlen(text);
      if (len > name_len) {
         if ((strncmp(text, Name, name_len) == 0) && (text[name_len] == '=')) {
            Number--;
            if (Number < 0) {
               dsyslog("epg2timer, parameter parser: found parameter '%s'", text);
               return text + name_len + 1;
               }
            }
         }
      }
  return NULL;
}


int epg2timer::cParameterParser::Count(const char *Name) const
{
  if (Name == NULL)
     return 0;
  size_t name_len = strlen(Name);
  if (name_len == 0)
     return 0;

  int count = 0;
  for (int i = 0; i < _list.Size(); i++) {
      const char *text = _list[i];
      if (text == NULL)
         continue;
      size_t len = strlen(text);
      if (len > name_len) {
         if ((strncmp(text, Name, name_len) == 0) && (text[name_len] == '='))
            count++;
         }
      }
  return count;
}


epg2timer::cEventFilterBase *epg2timer::cFilterTools::ParseFilterLine(const char *FilterLine, cList<cNestedItem> *SubItems)
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
  
  return NULL;
}


bool epg2timer::cFilterTools::LoadFilterFile(const char *FileName, cList<cEventFilter> &Filters)
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
          }

      if (filter != NULL)
         Filters.Add(new cEventFilter(filterName, action, filter));
      }

  return true;
}
