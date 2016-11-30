/*
 * epg2timer.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>

#include "filterfile.h"

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "auto-create timer from epg";
static const char *MAINMENUENTRY  = "epg2timer";

class cPluginEpg2timer : public cPlugin {
private:
  // Add any member variables or functions you may need here.
  epg2timer::cFilterFile *_filters;

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
  _filters = NULL;
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
  cString confFile = cString::sprintf("%s/%s", ConfigDirectory(Name()), "epg2timer.conf");
  _filters = epg2timer::cFilterFile::Load(*confFile);
  return true;
}

void cPluginEpg2timer::Stop(void)
{
  // Stop any background activities the plugin is performing.
  delete _filters;
}

void cPluginEpg2timer::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginEpg2timer::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!

  if (_filters != NULL)
     _filters->UpdateTimers(false);
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
  if (strcasecmp(Command, "updt") == 0) {
     if (_filters != NULL) {
        _filters->UpdateTimers(true);
        return cString::sprintf("timer-update triggered");
        }

     ReplyCode = 501;
     return cString::sprintf("no filter loaded");
     }
  else
  if (strcasecmp(Command, "test") == 0) {
     if ((Option == NULL) || (*Option == 0)) {
        ReplyCode = 501;
        return "Missing filename";
        }

     epg2timer::cFilterFile *filterFile = epg2timer::cFilterFile::Load(Option);
     if (filterFile == NULL) {
        ReplyCode = 501;
        return cString::sprintf("Error in file %s", Option);
        }
     if (filterFile->FilterCount() == 0) {
        delete filterFile;
        ReplyCode = 501;
        return cString::sprintf("No filters in file %s", Option);
        }

     filterFile->UpdateTimers(true);
     while (filterFile->Active())
           cCondWait::SleepMs(1000);
     delete filterFile;

     return "timers have been updated";
     }
  return NULL;
}

VDRPLUGINCREATOR(cPluginEpg2timer); // Don't touch this!
