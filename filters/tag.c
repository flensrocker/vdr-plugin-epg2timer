#include "tag.h"

#include "../tools/epg.h"
#include "../tools/stringconverter.h"
#include "../filtercontext.h"


epg2timer::cEventFilterTag::cTagFilter::cTagFilter(const cFilterContext& Context, const char *Tag, eTagFilterOperator Op, const char *Comp, bool Missing)
{
  _tag = Tag;
  _op = Op;
  _intComp = 0;
  _strComp = "";
  _strCompLen = 0;
  _missing = Missing;

  if ((*_tag == NULL) || (**_tag == 0))
     _op = tfoInvalid;
  else if ((_op > tfoInvalid) && (_op < tfoIsInt)) {
     if (!isnumber(Comp))
        _op = tfoInvalid;
     else
        _intComp = (int)StrToNum(Comp);
     }
  else {
     _strComp = Context.Converter()->Convert(Comp);
     _strCompLen = strlen(*_strComp);
     }
}


bool epg2timer::cEventFilterTag::cTagFilter::Matches(const char *Value) const
{
  if (_op == tfoInvalid)
     return false;
  if (Value == NULL)
     return _missing;

  if (_op < tfoIsInt) {
     if (!isnumber(Value))
        return false;

     int intval = (int)StrToNum(Value);
     switch (_op) {
       case tfoIntEqual:
         return (intval == _intComp);
       case tfoIntNotEqual:
         return (intval != _intComp);
       case tfoIntLesser:
         return (intval < _intComp);
       case tfoIntLesserOrEqual:
         return (intval <= _intComp);
       case tfoIntGreater:
         return (intval > _intComp);
       case tfoIntGreaterOrEqual:
         return (intval >= _intComp);
       default:
         break;
       }
     }
  else {
     switch (_op) {
       case tfoStrEqual:
         return (strcmp(Value, _strComp) == 0);
       case tfoStrNotEqual:
         return (strcmp(Value, _strComp) != 0);
       case tfoStrLesser:
         return (strcmp(Value, _strComp) < 0);
       case tfoStrLesserOrEqual:
         return (strcmp(Value, _strComp) <= 0);
       case tfoStrGreater:
         return (strcmp(Value, _strComp) > 0);
       case tfoStrGreaterOrEqual:
         return (strcmp(Value, _strComp) >= 0);
       case tfoStrIsEmpty:
         return (*Value == 0);
       case tfoStrIsNotEmpty:
         return (*Value != 0);
       case tfoStrContains:
         return (strstr(Value, *_strComp) != NULL);
       case tfoStrNotContains:
         return (strstr(Value, *_strComp) == NULL);
       case tfoStrStartswith:
         return (strlen(Value) <= _strCompLen) && startswith(*_strComp, Value);
       case tfoStrEndswith:
         return (strlen(Value) <= _strCompLen) && endswith(*_strComp, Value);
       default:
         break;
       }
     }

  return false;
}


epg2timer::cEventFilterTag::cEventFilterTag(cList<cTagFilter> *TagFilters, bool Missing)
{
  _tagFilters = TagFilters;
  _missing = Missing;
  for (const cTagFilter *tf = _tagFilters->First(); tf; tf = _tagFilters->Next(tf))
      _tagNames.Append(strdup(tf->Tag()));
}


epg2timer::cEventFilterTag::~cEventFilterTag(void)
{
  delete _tagFilters;
}


bool epg2timer::cEventFilterTag::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if ((Event == NULL) || (_tagFilters->Count() == 0))
     return false;

  cStringList *values = cEpgTools::ExtractTagValues(_tagNames, Event);
  if (values == NULL)
     return _missing;

  int i = 0;
  for (const cTagFilter *tf = _tagFilters->First(); tf; tf = _tagFilters->Next(tf), i++) {
      cString value = Context.Converter()->Convert(values->At(i));
      if (!tf->Matches(*value)) {
         delete values;
         return false;
         }
      }
  delete values;
  return true;
}
