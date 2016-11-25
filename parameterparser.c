#include "parameterparser.h"


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


const char *epg2timer::cParameterParser::At(int Index, cString &Name) const
{
  Name = "";
  if ((Index < 0) || (Index >= _list.Size()))
     return NULL;

  const char *text = _list[Index];
  if (text == NULL)
     return NULL;

  int pos = 0;
  int len = strlen(text);
  while ((pos < len) && (text[pos] != '='))
        pos++;
  Name = cString(text, text + pos);
  if (pos < len - 1)
     return text + pos + 1;

  return NULL;
}
