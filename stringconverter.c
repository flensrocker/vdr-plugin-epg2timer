#include "stringconverter.h"

#include <unicode/utypes.h>
#include <unicode/unistr.h>


epg2timer::cStringConverter::cStringConverter(void)
{
  _status = U_ZERO_ERROR;
  _converter = Transliterator::createInstance("NFD; [:M:] Remove; NFC; [:Punctuation:] Remove; Lower", UTRANS_FORWARD, _status);
}


epg2timer::cStringConverter::~cStringConverter(void)
{
  delete _converter;
}


cString epg2timer::cStringConverter::Convert(const char *text) const
{
  UnicodeString source = UnicodeString::fromUTF8(StringPiece(text));
  _converter->transliterate(source);
  // TODO check status
  std::string result;
  source.toUTF8String(result);
  return cString(result.c_str());
}
