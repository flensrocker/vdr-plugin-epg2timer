#include "epg.h"

#include "filtercontext.h"


cStringList *epg2timer::cEpgTools::ExtractTagValues(const cFilterContext& Context, const cStringList &Tags, const cEvent *Event)
{
  if ((Event == NULL) || (Event->Description() == NULL) || (*(Event->Description()) == 0))
     return NULL;

  const char *description = Event->Description();

  cVector<int> lines;
  lines.Append(0);
  int len = strlen(description) - 1; // ignore last newline if present
  for (int pos = 0; pos < len; pos++) {
      if (description[pos] == '\n')
         lines.Append(pos + 1);
      }
  lines.Append(len + 1);

  cStringList *values = new cStringList(Tags.Size());

  for (int i = 0; i < Tags.Size(); i++) {
      const char *tag = Tags[i];
      if ((tag == NULL) || (*tag == 0))
         values->Append(NULL);
      else if (strcasecmp(tag, "title") == 0)
         values->Append(strdup(Event->Title()));
      else if (strcasecmp(tag, "shorttext") == 0)
         values->Append(strdup(Event->ShortText()));
      else {
         bool found = false;
         const cStringList *synonyms = Context.TagSynonyms(tag);
         int s = 0;
         do {
           if ((synonyms != NULL) && (s < synonyms->Size()))
              tag = synonyms->At(s);
           int tagLen = strlen(tag);
           for (int l = 0; l < lines.Size() - 1; l++) {
               int lineLen = lines[l + 1] - lines[l];
               if ((lineLen > tagLen + 1)
               && startswith(description + lines[l], tag)
               && (description[lines[l] + tagLen] == ':')) {
                  int vPos = lines[l] + tagLen + 1;
                  while ((vPos < lines[l + 1]) && (description[vPos] == ' '))
                        vPos++;
                  if (vPos < lines[l + 1]) {
                     cString tmp(description + vPos, description + lines[l + 1] - 1);
                     values->Append(strdup(*tmp));
                     found = true;
                  }
                  break;
                  }
               }
           if ((synonyms == NULL) || (s >= synonyms->Size()))
              break;
           s++;
         } while (!found);
         if (!found)
            values->Append(NULL);
         }
      }

  return values;
}
