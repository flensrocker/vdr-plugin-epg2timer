#include "filenametools.h"

#include "epgtools.h"


namespace epg2timer
{
  class cFilenameTag : public cListObject
  {
  public:
    cString _tag;
    int _pos;
    int _len;
    int _padLen;
    char _padChar;

    cFilenameTag(const cString &Tag, int Pos, int Len)
    {
      _tag = Tag;
      _pos = Pos;
      _len = Len;
      _padLen = -1;
      _padChar = '0';

      const char *padPos = strrchr(*Tag, ':');
      if ((padPos != NULL) && (*(padPos + 1) !=0)) {
         _tag = cString(*Tag, padPos);
         padPos++;
         const char *sepPos = strchr(padPos, ',');
         if ((sepPos != NULL) && (*(sepPos + 1) != 0)) {
            _padChar = sepPos[1];
            cString tmp(padPos, sepPos);
            if (isnumber(*tmp))
               _padLen = StrToNum(*tmp);
            }
         else if (isnumber(padPos))
            _padLen = StrToNum(padPos);
         }
    };

    cString FormatValue(const char *Value) const
    {
      if (Value == NULL)
         return cString("");

      cString val = Value;
      if (_padLen < 0)
         return val;

      while ((int)strlen(*val) < _padLen)
            val = cString::sprintf("%c%s", _padChar, *val);
      return val;
    };
  };
}


cString epg2timer::cFilenameTools::ReplaceTags(const char *Filename, const cEvent *Event)
{
  if ((Filename == NULL) || (*Filename == 0) || (Event == NULL))
     return NULL;

  cList<cFilenameTag> tags;
  const char *startPos = strchr(Filename, '%');
  while (startPos != NULL) {
        const char *endPos = strchr(startPos + 1, '%');
        if (endPos == NULL)
           break;

        if (endPos > startPos + 1) {
           cString tag(startPos + 1, endPos);
           tags.Add(new cFilenameTag(tag, startPos - Filename, endPos - startPos + 1));
           }
        startPos = strchr(endPos + 1, '%');
        }

  if (tags.Count() == 0)
     return cString(Filename);

  cStringList tagNames(tags.Count());
  for (cFilenameTag *tag = tags.First(); tag; tag = tags.Next(tag))
      tagNames.Append(strdup(*(tag->_tag)));

  cStringList *values = cEpgTools::ExtractTagValues(tagNames, Event);
  if (values == NULL)
     return cString(Filename);

  cString newFilename = "";
  // tags are sorted in order of appearance, start at end
  int i = tags.Count() - 1;
  int lastTagPos = strlen(Filename);
  for (cFilenameTag *tag = tags.Last(); tag; tag = tags.Prev(tag), i--) {
      // prepend string between current and previous tag
      if (tag->_pos + tag->_len < lastTagPos)
         newFilename = cString::sprintf("%s%s", *cString(Filename + tag->_pos + tag->_len, Filename + lastTagPos), *newFilename);

      // prepend tag value
      cString value = tag->FormatValue(values->At(i));
      newFilename = cString::sprintf("%s%s", *value, *newFilename);

      lastTagPos = tag->_pos;
      }
  // prepend string before first tag
  if (lastTagPos > 0)
     newFilename = cString::sprintf("%s%s", *cString(Filename, Filename + lastTagPos), *newFilename);

  delete values;
  isyslog("epg2timer: convert filename from %s to %s", Filename, *newFilename);
  return newFilename;
}
