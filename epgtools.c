#include "epgtools.h"

#include <unicode/utypes.h>
#include <unicode/unistr.h>


cStringList *epg2timer::cEpgTools::ExtractTagValues(const cStringList &tags, const char *description)
{
  if ((description == NULL) || (*description == 0))
     return NULL;

  cVector<int> lines;
  lines.Append(0);
  int len = strlen(description) - 1; // ignore last newline if present
  for (int pos = 0; pos < len; pos++) {
      if (description[pos] == '\n')
         lines.Append(pos + 1);
      }
  lines.Append(len + 1);

  cStringList *values = new cStringList(tags.Size());

  for (int i = 0; i < tags.Size(); i++) {
      const char *tag = tags[i];
      if ((tag == NULL) || (*tag == 0))
         values->Append(NULL);
      else {
         bool found = false;
         int tagLen = strlen(tag);
         for (int l = 0; l < lines.Size() - 1; l++) {
             int lineLen = lines[l + 1] - lines[l];
             if ((lineLen > tagLen + 1)
             && startswith(description + lines[l], tag)
             && (description[tagLen] == ':')) {
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
         if (!found)
            values->Append(NULL);
         }
      }

  return values;
}



epg2timer::cEpgTools::cStringConverter::cStringConverter(void)
{
  _status = U_ZERO_ERROR;
  _converter = Transliterator::createInstance("NFD; [:M:] Remove; NFC; [:Punctuation:] Remove; Lower", UTRANS_FORWARD, _status);
}


epg2timer::cEpgTools::cStringConverter::~cStringConverter(void)
{
  delete _converter;
}


cString epg2timer::cEpgTools::cStringConverter::Convert(const char *text) const
{
  UnicodeString source = UnicodeString::fromUTF8(StringPiece(text));
  _converter->transliterate(source);
  // TODO check status
  std::string result;
  source.toUTF8String(result);
  return cString(result.c_str());
}

