// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "ExplorerIntegration_i.h"
#include "dllmain.h"
#include "../shared/Constants.h"

//////////////////////////////////////////////////////////////////////////
// constants
//////////////////////////////////////////////////////////////////////////
const TCHAR	szConfigFile[] = L"console.xml";

CExplorerIntegrationModule _AtlModule;
HINSTANCE	hModuleInst = NULL;
HANDLE		hConfigChanged = NULL;		///< this event is set when configuration in console.xml is changed

// protected shared configuration data
syncCriticalSection					g_oSync;
shared_ptr<SettingsHandler>			g_settingsHandler;
vector< shared_ptr<CTabSettings> >	g_vTabs;


//////////////////////////////////////////////////////////////////////////////
// trace function and TRACE macro

#ifdef _DEBUG
#include <stdio.h>

void Trace(const wchar_t* pszFormat, ...)
{
	wchar_t szOutput[1024];
	va_list	vaList;

	va_start(vaList, pszFormat);
	vswprintf(szOutput, _countof(szOutput), pszFormat, vaList);
	::OutputDebugString(szOutput);
}

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////////


// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// store instance
	hModuleInst = hInstance;

	// on start load settings
	if(dwReason==DLL_PROCESS_ATTACH)
	{// initialization
		// create global objects
		if(!hConfigChanged)
			hConfigChanged=CreateEvent(NULL,FALSE,FALSE,szConfigNotifyEventName);
		// load settings
		LoadConsoleSettings();
	}
	else if(dwReason==DLL_PROCESS_DETACH)
	{// cleanup
		if(hConfigChanged) CloseHandle(hConfigChanged);
	}

	return _AtlModule.DllMain(dwReason, lpReserved); 
}

HBITMAP	ConvertIconToBitmap(wstring& sIconFile);

void LoadConsoleSettings()
{
	// synchronization
	syncAutoLock	oLock(g_oSync);

	// clear previous settings
	g_vTabs.clear();
	g_settingsHandler.reset(new SettingsHandler());
	if(!g_settingsHandler)
	{// cannot create settings object, exit
		TRACE(_T("[") _T(__FUNCTION__) _T("] Cannot create settings object"));
		return;
	}

	// load settings from config file
	if (!g_settingsHandler->LoadSettings(Helpers::ExpandEnvironmentStrings(wstring(szConfigFile))))
	{
		// try to load config from module path
		wstring	sMyConfigPath = Helpers::GetModulePath(hModuleInst);
		sMyConfigPath += szConfigFile;
		if (!g_settingsHandler->LoadSettings(sMyConfigPath))
		{// cannot load setiings, exit
			TRACE(_T("[") _T(__FUNCTION__) _T("] Cannot load settings"));
			return;
		}
	}

	// fill tab settings
	TabSettings	&oTS = g_settingsHandler->GetTabSettings();
	DWORD	i,lim;
	for(i=0,lim=oTS.tabDataVector.size();i<lim;++i)
	{
		shared_ptr<CTabSettings>	newTS(new CTabSettings());
		if(newTS)
		{
			newTS->sName = oTS.tabDataVector[i]->strTitle;
			newTS->sIconFile = oTS.tabDataVector[i]->strIcon;
			if(newTS->sIconFile.size()>0)
				newTS->hIconBmp=ConvertIconToBitmap(newTS->sIconFile);
			g_vTabs.push_back(newTS);
		}
	}
}

HBITMAP ConvertIconToBitmap( wstring& sIconFile )
{
	// load icon
	HICON	hIcon = static_cast<HICON>(::LoadImage(NULL,Helpers::ExpandEnvironmentStrings(sIconFile).c_str(),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_LOADFROMFILE));
	if(!hIcon) return NULL;		// cannot load

	HDC	hScreenDC = NULL;
	HDC	hMemDC = NULL;
	HBITMAP	hResultBMP = NULL;
	HBITMAP	hMemBMP = NULL;
	HGDIOBJ	hOrgBMP = NULL;

	for(;;)
	{
		// prepare DC
		hScreenDC = GetDC(NULL);
		if(!hScreenDC) break;
		hMemDC = CreateCompatibleDC(hScreenDC);
		hMemBMP = CreateCompatibleBitmap(hScreenDC,16,16);
		if(!(hMemDC&&hMemBMP)) break;
		hOrgBMP = SelectObject(hMemDC,hMemBMP);
		if(!hOrgBMP) break;

		// draw icon to DC
		if(!DrawIconEx(hMemDC,0,0,hIcon,16,16,0,NULL,DI_NORMAL|DI_COMPAT)) break;

		// set result bitmap
		hResultBMP = hMemBMP;
		hMemBMP = NULL;

		// end
		break;
	}

	// cleanup
	if(hOrgBMP) SelectObject(hMemDC,hOrgBMP);
	if(hMemBMP) DeleteObject(hMemBMP);
	if(hMemDC) DeleteDC(hMemDC);
	if(hScreenDC) ReleaseDC(NULL,hScreenDC);
	if(hIcon) DestroyIcon(hIcon);

	return hResultBMP;
}