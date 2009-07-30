// Console.h
#include "SettingsHandler.h"

extern shared_ptr<SettingsHandler>	g_settingsHandler;
extern shared_ptr<ImageHandler>		g_imageHandler;

//////////////////////////////////////////////////////////////////////////////
/// Flags received from command line
#define CLF_REUSE_PREV_INSTANCE			0x0001	///< if another instance of console is already running open tabs in it even if "allow multiple instances" is enabled
#define CLF_FORCE_NEW_INSTANCE			0x0002	///< start another instance of console even if "allow multiple instances" is disabled

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
