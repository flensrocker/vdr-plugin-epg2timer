#include "eventfilter.h"

#include "epgtools.h"
#include "filenametools.h"
#include "filtercontext.h"


epg2timer::cEventFilter::cEventFilter(const char *Name, eFilterActions Action, const char *Filename, const cEventFilterBase *Filter)
{
  _name = Name;
  _action = Action;
  _filename = Filename;
  _filter = Filter;
}


bool epg2timer::cEventFilter::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if (_filter == NULL)
     return false;

  return _filter->Matches(Context, Event);
}


const char *epg2timer::cEventFilter::Name() const
{
  return *_name;
}


bool epg2timer::cEventFilter::AuxMatches(const cTimer *Timer) const
{
  const char *aux = Timer->Aux();
  if ((aux == NULL) || (strlen(aux) < 10) || !startswith(aux, "epg2timer="))
     return false;

  // workaround for epgd's xml-tags
  const char *epgd = strstr(aux, "<epgd>");
  if (epgd != NULL) {
     cString tmpAux(aux, epgd);
     return (strcasecmp(*_name, tmpAux + 10) == 0);
     }

  return (strcasecmp(*_name, aux + 10) == 0);
}


cTimer *epg2timer::cEventFilter::CreateTimer(const cEvent *Event) const
{
  cTimer *timer = new cTimer(Event);

  // mark timer
  timer->SetAux(*cString::sprintf("epg2timer=%s", Name()));

  if ((*_filename != NULL) && (**_filename != 0))
     timer->SetFile(*cFilenameTools::ReplaceTags(*_filename, Event));

  if (_action == faInactive)
     timer->ClrFlags(tfActive);

  return timer;
}


bool epg2timer::cEventFilter::UpdateTimer(cTimer *Timer, const cEvent *Event) const
{
  if (!AuxMatches(Timer))
     return false;

  bool updated = false;
  // active flag should not be updated, it's under the user's control
  if ((*_filename != NULL) && (**_filename != 0)) {
     cString filename = cFilenameTools::ReplaceTags(*_filename, Event);
     if (strcmp(*filename, Timer->File()) != 0) {
        Timer->SetFile(*filename);
        updated = true;
        }
     }

  return updated;
}


epg2timer::cEventFilter::eFilterActions epg2timer::cEventFilter::Action() const
{
  return _action;
}


epg2timer::cEventFilterList::cEventFilterList(cList<cEventFilterBase> *Filters)
{
  _filters = Filters;
}


epg2timer::cEventFilterList::~cEventFilterList(void)
{
  delete _filters;
  _filters = NULL;
}


bool epg2timer::cEventFilterList::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  return false;
}


epg2timer::cEventFilterAnd::cEventFilterAnd(cList<cEventFilterBase> *Filters)
 :cEventFilterList(Filters)
{
}

bool epg2timer::cEventFilterAnd::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  for (cEventFilterBase *f = _filters->First(); f; f = _filters->Next(f)) {
      if (!f->Matches(Context, Event))
         return false;
      }
  return true;
}


epg2timer::cEventFilterOr::cEventFilterOr(cList<cEventFilterBase> *Filters)
 :cEventFilterList(Filters)
{
}

bool epg2timer::cEventFilterOr::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  for (cEventFilterBase *f = _filters->First(); f; f = _filters->Next(f)) {
      if (f->Matches(Context, Event))
         return true;
      }
  return false;
}


epg2timer::cEventFilterChannel::cEventFilterChannel(tChannelID FromChannel, tChannelID ToChannel)
{
  _fromChannel = FromChannel;
  _toChannel = ToChannel;
}

bool epg2timer::cEventFilterChannel::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if (Event == NULL)
     return false;
  if ((_fromChannel == tChannelID::InvalidID) && (_toChannel == tChannelID::InvalidID))
     return true;

  tChannelID eventChannelID = Event->ChannelID();
  if (eventChannelID == tChannelID::InvalidID)
     return false;

#if APIVERSNUM < 20301
  cChannels *channels = &Channels;
#else
  // TODO vdr 2.3: get read-only channels
#endif
  if (channels == NULL)
     return false;

  const cChannel *eventChannel = channels->GetByChannelID(eventChannelID);
  if (eventChannel == NULL)
     return false;

  int eventChannelNumber = eventChannel->Number();

  if (_fromChannel == tChannelID::InvalidID) {
     const cChannel *to = channels->GetByChannelID(_toChannel);
     if (to == NULL)
        return false;

     return (eventChannelNumber <= to->Number());
     }
  else if (_toChannel == tChannelID::InvalidID) {
     const cChannel *from = channels->GetByChannelID(_fromChannel);
     if (from == NULL)
        return false;

     return (from->Number() <= eventChannelNumber);
     }
  else {
     const cChannel *from = channels->GetByChannelID(_fromChannel);
     const cChannel *to = channels->GetByChannelID(_toChannel);
     if ((from == NULL) || (to == NULL))
        return false;

     int fromNumber = from->Number();
     int toNumber = to->Number();

     // swap channels, fromNumber must be lesser than toNumber
     if (fromNumber > toNumber) {
        int swap = fromNumber;
        fromNumber = toNumber;
        toNumber = swap;
        }

     return (fromNumber <= eventChannelNumber) && (eventChannelNumber <= toNumber);
     }

  return true;
}


epg2timer::cEventFilterContains::cEventFilterContains(const cFilterContext& Context, const char *Needle, int Fields)
{
  _needle = Context.Converter()->Convert(Needle);
  _fields = Fields;
}

bool epg2timer::cEventFilterContains::Matches(const cFilterContext& Context, const cEvent *Event) const
{
  if ((Event == NULL) || (_fields == 0) || (*_needle == NULL) || (**_needle == 0))
     return false;

  if ((_fields & efTitle) && Matches(Context, Event->Title()))
     return true;
  if ((_fields & efShortText) && Matches(Context, Event->ShortText()))
     return true;
  if ((_fields & efDescription) && Matches(Context, Event->Description()))
     return true;
  return false;
}

bool epg2timer::cEventFilterContains::Matches(const cFilterContext& Context, const char *Text) const
{
  if ((Text == NULL) || (*Text == 0))
     return false;
  cString t = Context.Converter()->Convert(Text);
  return (strcasestr(t, *_needle) != NULL);
}


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
