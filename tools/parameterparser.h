#ifndef epg2timer_tools_parameterparser_h
#define epg2timer_tools_parameterparser_h

#include <vdr/tools.h>

namespace epg2timer
{
  // Instantiate with an string like "key1=value1,key2=value2,key2=value3,key3=value4,...".
  // Comma and backslashes in values must be escaped with a backslash.
  // Keys must not contain the equal sign.
  class cParameterParser
  {
  public:
    cParameterParser(const char *Parameters);

    // If Number is greater than zero, returns the "Number + 1"'th appearance of the parameter (zero based index).
    const char *Get(const char *Name, int Number = 0) const;
    // Get the number of appearances of parameter "Name".
    int Count(const char *Name) const;
    int Size(void) const { return _list.Size(); };
    const char *At(int Index, cString &Name) const;

  private:
    cStringList  _list;
  };
}

#endif
