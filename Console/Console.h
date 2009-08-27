// Console.h
#include "SettingsHandler.h"

extern shared_ptr<SettingsHandler>	g_settingsHandler;
extern shared_ptr<ImageHandler>		g_imageHandler;

//////////////////////////////////////////////////////////////////////////////
/// Flags received from command line
extern const unsigned int CLF_REUSE_PREV_INSTANCE;		// if another instance of console is already running open tabs in it even if "allow multiple instances" is enabled
extern const unsigned int CLF_FORCE_NEW_INSTANCE;		// start another instance of console even if "allow multiple instances" is disabled
extern const unsigned int CLF_REUSE_PREV_TAB;			// ...
extern const unsigned int CLF_FORCE_NEW_TAB;			// create another tab in console even if "reuse tab" is on

//////////////////////////////////////////////////////////////////////////////

// vds: >>
void ParseCommandLine(
	LPTSTR lptstrCmdLine, 
	wstring& strConfigFile, 
	wstring& strWindowTitle, 
	vector<wstring>& startupTabs, 
	vector<wstring>& startupDirs, 
	vector<wstring>& startupCmds, 
	int& nMultiStartSleep, 
	wstring& strDbgCmdLine,
	WORD& iFlags
);

wstring BuildCommandLine(
	wstring strConfigFile, 
	wstring strWindowTitle, 
	vector<wstring>& startupTabs, 
	vector<wstring>& startupDirs, 
	vector<wstring>& startupCmds, 
	int nMultiStartSleep, 
	wstring strDbgCmdLine
);
// vds: <<
