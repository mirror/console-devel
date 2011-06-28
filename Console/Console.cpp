// Console.cpp : main source file for Console.exe
//

#include "stdafx.h"

#include "resource.h"

#include "ConsoleView.h"
#include "aboutdlg.h"
#include "MainFrame.h"
#include "Console.h"

#include <sys/types.h> // vds:
#include <sys/stat.h> // vds:

// vds: >>
const unsigned int CLF_REUSE_PREV_INSTANCE = 0x0001;
const unsigned int CLF_FORCE_NEW_INSTANCE = 0x0002;
const unsigned int CLF_REUSE_PREV_TAB = 0x0004;
const unsigned int CLF_FORCE_NEW_TAB = 0x0008;
// vds: <<

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Global variables

CAppModule					_Module;

shared_ptr<SettingsHandler>	g_settingsHandler;
shared_ptr<ImageHandler>	g_imageHandler;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Invisible parent window class for the main window if no taskbar should be shown

class NoTaskbarParent
	: public CWindowImpl<NoTaskbarParent, CWindow, CWinTraits<WS_POPUP, WS_EX_TOOLWINDOW> >
{
	public:
		DECLARE_WND_CLASS(L"NoTaskbarParent")

		NoTaskbarParent() {}
		~NoTaskbarParent() {}

		BEGIN_MSG_MAP(NoTaskbarParent)
		END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

// vds: >>
std::wstring GetDir(std::wstring path)
{
	struct _stat stat;

	_wstat(path.c_str(), &stat);

	if (stat.st_mode & _S_IFDIR)
		return path;

	std::size_t pos_backslash = path.find_last_of(L'\\');
	if (pos_backslash != std::string::npos) {
		return path.substr(0, pos_backslash);
	}

	std::size_t pos_slash = path.find_last_of('/');
	if (pos_slash != std::string::npos)
		return path.substr(0, pos_slash);

	return path;
}
// vds: <<

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ParseCommandLine
(
	LPTSTR lptstrCmdLine, 
	wstring& strConfigFile, 
	wstring& strWindowTitle, 
	vector<wstring>& startupTabs, 
	vector<wstring>& startupDirs, 
	vector<wstring>& startupCmds, 
	int& nMultiStartSleep, 
	wstring& strDbgCmdLine,
	WORD& iFlags // vds
)
{
	int						argc = 0;
	shared_array<LPWSTR>	argv(::CommandLineToArgvW(lptstrCmdLine, &argc), ::GlobalFree);

	if (argc < 1) return;

	for (int i = 0; i < argc; ++i)
	{
		if (wstring(argv[i]) == wstring(L"-c"))
		{
			// custom config file
			++i;
			if (i == argc) break;
			strConfigFile = argv[i];
		}
		else if (wstring(argv[i]) == wstring(L"-w"))
		{
			// startup tab name
			++i;
			if (i == argc) break;
			strWindowTitle = argv[i];
		}
		else if (wstring(argv[i]) == wstring(L"-t"))
		{
			// startup tab name
			++i;
			if (i == argc) break;
			startupTabs.push_back(argv[i]);
		}
		else if (wstring(argv[i]) == wstring(L"-d"))
		{
			// startup dir
			++i;
			if (i == argc) break;
			std::wstring startupDir(std::wstring(GetDir(argv[i]))); // vds:
			startupDirs.push_back(startupDir);
		}
		else if (wstring(argv[i]) == wstring(L"-r"))
		{
			// startup cmd
			++i;
			if (i == argc) break;
			startupCmds.push_back(argv[i]);
		}
		else if (wstring(argv[i]) == wstring(L"-ts"))
		{
			// startup tab sleep for multiple tabs
			++i;
			if (i == argc) break;
			nMultiStartSleep = _wtoi(argv[i]);
			if (nMultiStartSleep < 0) nMultiStartSleep = 500;
		}
		// vds: >>
		else if (wstring(argv[i]) == wstring(L"-reuse"))
		{
			// reuse existing console instance if running, mutual exclusive with -new
			iFlags |= CLF_REUSE_PREV_INSTANCE;
		}
		else if (wstring(argv[i]) == wstring(L"-new"))
		{
			// force creation of new console instance, mutual exclusive with -reuse
			iFlags |= CLF_FORCE_NEW_INSTANCE;
		}
		else if (wstring(argv[i]) == wstring(L"-reuse-tab"))
		{
			// reuse existing console instance if running, mutual exclusive with -new
			iFlags |= CLF_REUSE_PREV_TAB;
		}
		else if (wstring(argv[i]) == wstring(L"-new-tab"))
		{
			// force creation of new console instance, mutual exclusive with -reuse
			iFlags |= CLF_FORCE_NEW_TAB;
		}
		// TODO: not working yet, need to investigate
#if 0
		else if (wstring(argv[i]) == wstring(L"-dbg"))
		{
			// console window replacement option (see Tip 1 in the help file)
			++i;
			if (i == argc) break;
			strDbgCmdLine = argv[i];
		}
#endif
		// vds: <<
	}

	// make sure that startupDirs and startupCmds are at least as big as startupTabs
	if (startupDirs.size() < startupTabs.size()) startupDirs.resize(startupTabs.size());
	if (startupCmds.size() < startupTabs.size()) startupCmds.resize(startupTabs.size());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

// vds: >>
wstring BuildCommandLine(
	wstring strConfigFile, 
	wstring strWindowTitle, 
	vector<wstring>& startupTabs, 
	vector<wstring>& startupDirs, 
	vector<wstring>& startupCmds, 
	int nMultiStartSleep, 
	wstring strDbgCmdLine,
	WORD iFlags
)
{
	wstring ret;

	if (strConfigFile != _T("")) {
		ret += _T(" -c ") + strConfigFile;
	}
	if (strWindowTitle != _T("")) {
		ret += _T(" -w ") + strWindowTitle;
	}

	if (startupTabs.size() == 0) {
		TabSettings& tabSettings = g_settingsHandler->GetTabSettings();
		wstring title = tabSettings.tabDataVector[0]->strTitle;

		startupTabs.push_back(title);
	}
	if (startupDirs.size() < startupTabs.size()) startupDirs.resize(startupTabs.size());
	if (startupCmds.size() < startupTabs.size()) startupCmds.resize(startupTabs.size());

	TCHAR workingDirectory[_MAX_PATH + 1];
	GetCurrentDirectory(_MAX_PATH, workingDirectory);

	for (unsigned int i = 0; i < startupDirs.size(); ++i) {
		if (startupDirs[i] != _T(""))
			continue;

		startupDirs[i] = wstring(workingDirectory);
	}

	for (unsigned int i = 0; i < startupTabs.size(); ++i) {
		wstring tab = startupTabs[i];
		wstring dir = startupDirs[i];
		wstring cmd = startupCmds[i];

		ret += _T(" -t \"") + tab + _T("\"");
		ret += _T(" -d \"") + dir + _T("\"");
		ret += _T(" -r \"") + cmd + _T("\"");
	}
	for (unsigned int i = startupTabs.size(); i < startupDirs.size(); ++i) {
		wstring dir = startupDirs[i];

		ret += _T(" -d \"") + dir + _T("\"");
	}
	for (unsigned int i = startupTabs.size(); i < startupCmds.size(); ++i) {
		wstring cmd = startupCmds[i];

		ret += _T(" -r \"") + cmd + _T("\"");
	}

	if (nMultiStartSleep != 0) {
		TCHAR szMultiStartSleep[256];
		_sntprintf_s(szMultiStartSleep, 256, sizeof(szMultiStartSleep) / sizeof(TCHAR), _T(" -ts %d"), nMultiStartSleep);
		ret += szMultiStartSleep;
	}

	if (iFlags & CLF_REUSE_PREV_INSTANCE)
		ret += _T(" -reuse");

	if (iFlags & CLF_FORCE_NEW_INSTANCE)
		ret += _T(" -new");

	if (iFlags & CLF_REUSE_PREV_TAB)
		ret += _T(" -reuse-tab");

	if (iFlags & CLF_FORCE_NEW_TAB)
		ret += _T(" -new-tab");

	return ret;
}
// vds: <<

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	// vds: >>
	HANDLE hOneInstanceMutex = CreateMutex(NULL, FALSE, _T("ConsoleOnInstanceMutex"));
	DWORD oneInstanceResult = GetLastError();
	bool bOneInstance = oneInstanceResult == ERROR_ALREADY_EXISTS; // vds:
	HWND hPrevConsole = FindWindow(MainFrame::GetWndClassInfo().m_wc.lpszClassName, NULL);
	// vds: <<

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	wstring			strConfigFile(L"");
	wstring			strWindowTitle(L"");
	vector<wstring>	startupTabs;
	vector<wstring>	startupDirs;
	vector<wstring>	startupCmds;
	int				nMultiStartSleep = 0;
	wstring			strDbgCmdLine(L"");
	WORD			iFlags = 0; // vds

	ParseCommandLine(
		lpstrCmdLine, 
		strConfigFile, 
		strWindowTitle, 
		startupTabs, 
		startupDirs, 
		startupCmds, 
		nMultiStartSleep, 
		strDbgCmdLine,
		iFlags); // vds

	if (strConfigFile.length() == 0)
	{
		strConfigFile = wstring(L"console.xml"); // vds
//		strConfigFile = wstring(L"%APPDATA%\Console\console.xml");
//		strConfigFile = Helpers::GetModulePath(NULL) + wstring(L"console.xml");
//		strConfigFile = wstring(::_wgetenv(L"APPDATA")) + wstring(L"\\Console\\console.xml"); // vds
	}

	if (!g_settingsHandler->LoadSettings(Helpers::ExpandEnvironmentStrings(strConfigFile)))
	{
		//TODO: error handling
		return -1;
	}
	// vds: >>
	if (g_settingsHandler->GetBehaviorSettings().oneInstanceSettings.bAllowMultipleInstances && !(iFlags & CLF_REUSE_PREV_INSTANCE))
		bOneInstance = false;

	if (iFlags & CLF_FORCE_NEW_INSTANCE)
		bOneInstance = false;
	// vds: <<

	// create main window
	NoTaskbarParent noTaskbarParent;
	MainFrame wndMain(strWindowTitle, startupTabs, startupDirs, startupCmds, nMultiStartSleep, bOneInstance, strDbgCmdLine); // vds:

	if (!g_settingsHandler->GetAppearanceSettings().stylesSettings.bTaskbarButton && !bOneInstance) // vds:
	{
		noTaskbarParent.Create(NULL);
	}

	if (wndMain.CreateEx(noTaskbarParent.m_hWnd) == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 1;
	}

	// vds: >>
	wstring rebuildCommandLine;
	if (!bOneInstance) {
		wndMain.ShowWindow(nCmdShow);
	}
	else {
		// Give 3 seconds to exchange the DDE messages.
		wndMain.SetTimer(TIMER_TIMEOUT, 3000, NULL);
		rebuildCommandLine = BuildCommandLine(
			strConfigFile, 
			strWindowTitle, 
			startupTabs, 
			startupDirs, 
			startupCmds, 
			nMultiStartSleep, 
			strDbgCmdLine,
			iFlags);

#ifdef USE_COPYDATA_MSG
		// Search for running instance of console
		if (hPrevConsole) {
			COPYDATASTRUCT stCopyData;
			stCopyData.dwData = eEC_NewTab;
			stCopyData.cbData = (rebuildCommandLine.size() + 1) * sizeof(wchar_t);
			stCopyData.lpData = reinterpret_cast<void*>(const_cast<wchar_t*>(rebuildCommandLine.c_str()));

			SendMessage(hPrevConsole, WM_COPYDATA, 0, (LPARAM)(LPVOID)&stCopyData);
		}
#else
		::PostMessage(wndMain.m_hWnd, WM_SEND_DDE_COMMAND, reinterpret_cast<WPARAM>(hPrevConsole), reinterpret_cast<LPARAM>(rebuildCommandLine.c_str()));
#endif
	}
	// vds: <<
	int nRet = theLoop.Run();

	if (noTaskbarParent.m_hWnd != NULL) noTaskbarParent.DestroyWindow();

	_Module.RemoveMessageLoop();

	// vds: >>
	if (hOneInstanceMutex) {
		CloseHandle(hOneInstanceMutex);
		hOneInstanceMutex = NULL;
	}
	// vds: <<

	return nRet;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	g_settingsHandler.reset(new SettingsHandler());
	g_imageHandler.reset(new ImageHandler());

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	g_settingsHandler.reset();

	::CoUninitialize();

	return nRet;
}

//////////////////////////////////////////////////////////////////////////////
