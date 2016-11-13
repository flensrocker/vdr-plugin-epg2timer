/*
 * epg2timer.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>

#include "epgtools.h"
#include "eventfilter.h"
#include "filtertools.h"
#include "timertools.h"

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "auto-create timer from epg";
static const char *MAINMENUENTRY  = "epg2timer";

class cPluginEpg2timer : public cPlugin {
private:
  // Add any member variables or functions you may need here.
  epg2timer::cEpgTestHandler *_epgTestHandler;

public:
  cPluginEpg2timer(void);
  virtual ~cPluginEpg2timer();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cString Active(void);
  virtual time_t WakeupTime(void);
  virtual const char *MainMenuEntry(void) { return MAINMENUENTRY; }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

cPluginEpg2timer::cPluginEpg2timer(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
  _epgTestHandler = NULL;
}

cPluginEpg2timer::~cPluginEpg2timer()
{
  // Clean up after yourself!
}

const char *cPluginEpg2timer::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return NULL;
}

bool cPluginEpg2timer::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}

bool cPluginEpg2timer::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.
  return true;
}

bool cPluginEpg2timer::Start(void)
{
  // Start any background activities the plugin shall perform.
  return true;
}

void cPluginEpg2timer::Stop(void)
{
  // Stop any background activities the plugin is performing.
}

void cPluginEpg2timer::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginEpg2timer::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}

cString cPluginEpg2timer::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}

time_t cPluginEpg2timer::WakeupTime(void)
{
  // Return custom wakeup time for shutdown script
  return 0;
}

cOsdObject *cPluginEpg2timer::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  return NULL;
}

cMenuSetupPage *cPluginEpg2timer::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return NULL;
}

bool cPluginEpg2timer::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

bool cPluginEpg2timer::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginEpg2timer::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  return NULL;
}

cString cPluginEpg2timer::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  if (Command == NULL)
     return NULL;

  // Process SVDRP commands this plugin implements
  if (strcasecmp(Command, "seto") == 0) {
     if (_epgTestHandler == NULL)
        _epgTestHandler = new epg2timer::cEpgTestHandler();

     int offset = 0;
     if ((Option != NULL) && isnumber(Option))
        offset = atoi(Option);
     _epgTestHandler->SetStartOffset(offset);
     ReplyCode = 250;
     return cString::sprintf("Set EPG start offset to: %d", offset);
     }
  else
  if (strcasecmp(Command, "test") == 0) {
     if ((Option == NULL) || (*Option == 0)) {
        ReplyCode = 501;
        return "Missing filename";
        }

     cList<epg2timer::cEventFilter> filters;
     if (!epg2timer::cFilterTools::LoadFilterFile(Option, filters)) {
        ReplyCode = 501;
        return cString::sprintf("Error in file %s", Option);
        }
     if (filters.Count() == 0) {
        ReplyCode = 501;
        return cString::sprintf("No filters in file %s", Option);
        }

     // get write lock on timer
     bool hasTimerWriteLock = !Timers.BeingEdited();
     Timers.IncBeingEdited();

     int eventCount = 0;
     int foundCount = 0;

     cString msg = cString::sprintf("Matching Events for filters in '%s':", Option);
     ReplyCode = 250;

     // get read lock on schedules
     cSchedulesLock schedulesLock;
     const cSchedules *schedules = cSchedules::Schedules(schedulesLock);
     if (schedules) {
        for (const cSchedule *s = schedules->First(); s; s = schedules->Next(s)) {
            const cList<cEvent> *events = s->Events();
            if (events) {
               for (const cEvent *e = events->First(); e; e = events->Next(e)) {
                   eventCount++;
                   for (const epg2timer::cEventFilter *filter = filters.First(); filter; filter = filters.Next(filter)) {
                       if (filter->Matches(e)) {
                          foundCount++;
                          msg = cString::sprintf("%s\nFilter: %s\n(%u) %s: %s", *msg, filter->Name(), e->EventID(), *TimeToString(e->StartTime()), e->Title());
                          if (hasTimerWriteLock) {
                             cTimer *t = epg2timer::cTimerTools::FindTimer(&Timers, schedules, e);
                             if (t) {
                                msg = cString::sprintf("%s\n has timer", *msg);
                                if (filter->UpdateTimer(t, e)) {
                                   msg = cString::sprintf("%s (updated)", *msg);
                                   Timers.SetModified();
                                   }
                                }
                             else {
                                cTimer *nt = filter->CreateTimer(e);
                                if (nt != NULL) {
                                   Timers.Add(nt);
                                   Timers.SetModified();
                                   msg = cString::sprintf("%s\n created timer on %s", *msg, *e->ChannelID().ToString());
                                   }
                                }
                             }
                          }
                      }
                   }
               }
            }
        }

     Timers.DecBeingEdited();

     msg = cString::sprintf("%s\n%d events processed, found %d", *msg, eventCount, foundCount);
     return msg;
     }
  return NULL;
}

VDRPLUGINCREATOR(cPluginEpg2timer); // Don't touch this!
