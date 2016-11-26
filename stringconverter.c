#include "stringconverter.h"


epg2timer::cStringConverter::cStringConverter(void)
{
  _systemIsUtf8 = !cCharSetConv::SystemCharacterTable() || strcmp(cCharSetConv::SystemCharacterTable(), "UTF-8") == 0;
  _status = U_ZERO_ERROR;
  _converter = icu::Transliterator::createInstance("NFD; [:M:] Remove; NFC; [:Punctuation:] Remove; Lower", UTRANS_FORWARD, _status);
  if (U_FAILURE(_status))
     esyslog("epg2timer: error while creating the icu-transliterator: %d", _status);
}


epg2timer::cStringConverter::~cStringConverter(void)
{
  delete _converter;
}


cString epg2timer::cStringConverter::Convert(const char *Text) const
{
  if (Text == NULL)
     return cString(NULL);

  icu::UnicodeString source;
  if (_systemIsUtf8)
     source = icu::UnicodeString::fromUTF8(icu::StringPiece(Text));
  else {
     cCharSetConv *charsetConverter = new cCharSetConv(NULL, "UTF-8");
     source = icu::UnicodeString::fromUTF8(icu::StringPiece(charsetConverter->Convert(Text)));
     delete charsetConverter;
     }

  _converter->transliterate(source);
  std::string tmp;
  source.toUTF8String(tmp);

  cString result;
  if (_systemIsUtf8)
     result = tmp.c_str();
  else {
     cCharSetConv *charsetConverter = new cCharSetConv("UTF-8", cCharSetConv::SystemCharacterTable());
     result = charsetConverter->Convert(tmp.c_str());
     delete charsetConverter;
     }

  return result;
}
