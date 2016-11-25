#include "epgtools.h"


cStringList *epg2timer::cEpgTools::ExtractTagValues(const cStringList &Tags, const char *Description)
{
  if ((Description == NULL) || (*Description == 0))
     return NULL;

  cVector<int> lines;
  lines.Append(0);
  int len = strlen(Description) - 1; // ignore last newline if present
  for (int pos = 0; pos < len; pos++) {
      if (Description[pos] == '\n')
         lines.Append(pos + 1);
      }
  lines.Append(len + 1);

  cStringList *values = new cStringList(Tags.Size());

  for (int i = 0; i < Tags.Size(); i++) {
      const char *tag = Tags[i];
      if ((tag == NULL) || (*tag == 0))
         values->Append(NULL);
      else {
         bool found = false;
         int tagLen = strlen(tag);
         for (int l = 0; l < lines.Size() - 1; l++) {
             int lineLen = lines[l + 1] - lines[l];
             if ((lineLen > tagLen + 1)
             && startswith(Description + lines[l], tag)
             && (Description[tagLen] == ':')) {
                int vPos = lines[l] + tagLen + 1;
                while ((vPos < lines[l + 1]) && (Description[vPos] == ' '))
                      vPos++;
                if (vPos < lines[l + 1]) {
                   cString tmp(Description + vPos, Description + lines[l + 1] - 1);
                   values->Append(strdup(*tmp));
                   found = true;
                }
                break;
                }
             }
         if (!found)
            values->Append(NULL);
         }
      }

  return values;
}
