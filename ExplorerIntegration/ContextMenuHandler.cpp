// ContextMenuHandler.cpp : Implementation of CContextMenuHandler

#include "stdafx.h"
#include "ContextMenuHandler.h"
#include <strsafe.h>

//////////////////////////////////////////////////////////////////////////
// constants and defines
//////////////////////////////////////////////////////////////////////////
const TCHAR		szConfigPath[] = _T("Software\\Console\\Explorer Integration");
const TCHAR		szConfigVTabs[] = _T("Tabs");
const TCHAR		szConfigVPath[] = _T("Path");
const TCHAR		szExeName[] = _T("Console.exe");
TCHAR			szItemRunConsole[] = _T("Run Console");
TCHAR			szItemRunConsoleWithTab[] = _T("Run Console Tab");
const char		szVerbRunConsoleA[] = "run";
const wchar_t	szVerbRunConsoleW[] = L"run";
const char		szDescrRunConsoleA[] = "Run Console here";
const wchar_t	szDescrRunConsoleW[] = L"Run Console here";

// debug output
#ifdef DEBUG_TO_LOG_FILES
# define f_log(x)	\
	{ \
		if(m_log!=INVALID_HANDLE_VALUE)	\
		{ \
			DWORD iWr;	\
			WriteFile(m_log,x,strlen(x),&iWr,NULL); \
			WriteFile(m_log,"\r\n",2,&iWr,NULL); \
		} \
	}
#else
# define f_log(x)
#endif

// CContextMenuHandler

CContextMenuHandler::CContextMenuHandler()
{
#ifdef DEBUG_TO_LOG_FILES
	m_log = INVALID_HANDLE_VALUE;
#endif
}

HRESULT CContextMenuHandler::FinalConstruct()
{
#ifdef DEBUG_TO_LOG_FILES
	char	fname[MAX_PATH];
	ULARGE_INTEGER	iTime;
	GetSystemTimeAsFileTime((FILETIME*)&iTime);
	sprintf_s(fname,MAX_PATH,"c:\\ContextMenuHandler-%I64u.log",iTime.QuadPart);
	m_log = CreateFileA(fname,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
#endif
	f_log(__FUNCTION__);

	// load config
	LoadConfig();

	return S_OK;
}

void CContextMenuHandler::FinalRelease()
{
	f_log(__FUNCTION__);
#ifdef DEBUG_TO_LOG_FILES
	if(m_log!=INVALID_HANDLE_VALUE) CloseHandle(m_log);
#endif
}

// extern declaration
extern HINSTANCE hModuleInst;

int CContextMenuHandler::LoadConfig()
{
	TCHAR	tbuf[1024];
	HKEY	hK;
	DWORD	iSize,/*iInd,*/iType;
//	FILETIME	ftLastWrite;
	// load path as module path
	sExePath = Helpers::GetModulePath(hModuleInst).c_str();
	// load global config if present
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,szConfigPath,0,KEY_READ,&hK)==ERROR_SUCCESS)
	{
		// get path
		iSize = 1024*sizeof(TCHAR);
		if(RegQueryValueEx(hK,szConfigVPath,NULL,&iType,(LPBYTE)tbuf,&iSize)==ERROR_SUCCESS)
			if(iType==REG_SZ)
			{// ok
				sExePath = tbuf;
				// add trailing '\'
				if(sExePath.GetLength()>0)
					if(tbuf[sExePath.GetLength()-1]!=_T('\\'))
						sExePath+=_T("\\");

#ifdef DEBUG_TO_LOG_FILES
				char	_tbuf[1024];
				sprintf_s(_tbuf,1024,"Executable path: %S",tbuf);
				f_log(_tbuf);
#endif
			}
			else {f_log("Wrong executable path type");}
		else f_log("Cannot query global config value");
		// close key
		RegCloseKey(hK);
	}
	else f_log("Cannot open global config key");

/*
	// load tab list for this user
	if(RegOpenKeyEx(HKEY_CURRENT_USER,szConfigPath,0,KEY_READ,&hK)!=ERROR_SUCCESS) {f_log("Cannot open user config key");return E_FAIL;}
	// get tabs
	for(iInd=0,iSize=1024*sizeof(TCHAR);RegEnumKeyEx(hK,iInd,tbuf,&iSize,NULL,NULL,NULL,&ftLastWrite)==ERROR_SUCCESS;++iInd,iSize=1024*sizeof(TCHAR))
	{// parse tabs
		vTabs.push_back(tbuf);

#ifdef DEBUG_TO_LOG_FILES
		char	_tbuf[1024];
		sprintf_s(_tbuf,1024,"Tab: %S",tbuf);
		f_log(_tbuf);
#endif
	}
	// close key
	RegCloseKey(hK);
*/

	return S_OK;
}

STDMETHODIMP CContextMenuHandler::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IShellExtInit,
		&IID_IContextMenu
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CContextMenuHandler::Initialize(PCIDLIST_ABSOLUTE pidlFolder,IDataObject *pdtobj,HKEY hkeyProgID)
{
#ifdef DEBUG_TO_LOG_FILES
	char	tbuf[100];
	sprintf_s(tbuf,100,__FUNCTION__ ": Folder=%p, Object=%p",pidlFolder,pdtobj);
	f_log(tbuf);
#endif

	// clear path if any
	sPath.Empty();

	// get path from folder PIDL if exists - that means that we were called from background of folder
	if(pidlFolder)
	{
		TCHAR	tbuf[MAX_PATH+1];
		if(SHGetPathFromIDList(pidlFolder,tbuf)) sPath=tbuf;
		else f_log("Error: Cannot retrieve folder name");
		return S_OK;	// we get all that we need
	}

	// get path from file object
	if(pdtobj) 
	{ 
		TCHAR		tbuf[MAX_PATH+1];
		STGMEDIUM   medium;
		FORMATETC   fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		UINT        uCount;

		if(SUCCEEDED(pdtobj->GetData(&fe, &medium)))
		{
			// Get the count of files dropped.
			uCount = DragQueryFile((HDROP)medium.hGlobal,(UINT)-1,NULL,0);

			// Get the first file name from the CF_HDROP.
			if(uCount)
			{
				DragQueryFile((HDROP)medium.hGlobal,0,tbuf,sizeof(tbuf)/sizeof(TCHAR));
				sPath = tbuf;
			}

			ReleaseStgMedium(&medium);
		}
	}

	return S_OK;
}

STDMETHODIMP CContextMenuHandler::QueryContextMenu(HMENU hmenu,UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{
#ifdef DEBUG_TO_LOG_FILES
	char	tbuf[200];
	sprintf_s(tbuf,200,__FUNCTION__ ": index=%u, first=%u, last=%u, flags=%.08lX, path=%S",indexMenu,idCmdFirst,idCmdLast,uFlags,sPath.GetString());
	f_log(tbuf);
#endif

	if (CMF_DEFAULTONLY & uFlags)
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));

	if (sPath.IsEmpty())
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));

	MENUITEMINFO ii;
	DWORD lastId = idCmdFirst;

	if (g_settingsHandler->GetBehaviorSettings().shellSettings.bRunConsoleMenItem) {
		// Fill main menu first item info
		memset(&ii,0,sizeof(MENUITEMINFO));
		ii.cbSize = sizeof(MENUITEMINFO);
		ii.fMask = MIIM_CHECKMARKS|MIIM_STRING|MIIM_ID;
		ii.wID = idCmdFirst + eMC_RunConsole;
		ii.dwTypeData = szItemRunConsole;
		ii.cch = _tcslen(szItemRunConsole);
		ii.hbmpChecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
		ii.hbmpUnchecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
		// Insert menu item
		InsertMenuItem(hmenu, indexMenu++, TRUE, &ii);

		lastId = ii.wID;
	}

	if (g_settingsHandler->GetBehaviorSettings().shellSettings.bRunConsoleTabMenuItem) {
		HMENU hSubMenu = CreateMenu();
		if (!hSubMenu)
			return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));

		// Create submenu
		DWORD i, lim;
		for (i = 0, lim = g_vTabs.size(); i < lim; ++i)
		{
			wstring		sTabName = g_vTabs[i]->sName;
			HBITMAP		hTabIcon = g_vTabs[i]->hIconBmp;
			// Fill main menu item info
			memset(&ii, 0, sizeof(MENUITEMINFO));
			ii.cbSize = sizeof(MENUITEMINFO);
			ii.fMask = MIIM_STRING | MIIM_ID | (hTabIcon ? MIIM_CHECKMARKS : 0);
			ii.wID = idCmdFirst + eMC_RunConsoleWithTab + i;
			ii.dwTypeData = const_cast<LPTSTR>(sTabName.c_str());
			ii.cch = sTabName.size();
			if (hTabIcon)
				ii.hbmpChecked=ii.hbmpUnchecked=hTabIcon;
			// Insert menu item
			InsertMenuItem(hSubMenu,i,TRUE,&ii);

			lastId = ii.wID;
		}
		// Fill main menu item info
		memset(&ii,0,sizeof(MENUITEMINFO));
		ii.cbSize = sizeof(MENUITEMINFO);
		ii.fMask = MIIM_CHECKMARKS|MIIM_STRING|MIIM_SUBMENU;
		ii.dwTypeData = szItemRunConsoleWithTab;
		ii.cch = _tcslen(szItemRunConsoleWithTab);
		ii.hSubMenu = hSubMenu;
		ii.hbmpChecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
		ii.hbmpUnchecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
		// insert menu item
		InsertMenuItem(hmenu, indexMenu++, TRUE, &ii);

		lastId = idCmdFirst + eMC_RunConsoleWithTab + g_vTabs.size();
	}
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(lastId - idCmdFirst + 1));
}

STDMETHODIMP CContextMenuHandler::InvokeCommand(CMINVOKECOMMANDINFO *pici)
{
	f_log(__FUNCTION__);

	BOOL fEx = FALSE;
	BOOL fUnicode = FALSE;

	if(pici->cbSize = sizeof(CMINVOKECOMMANDINFOEX))
	{
		fEx = TRUE;
		if((pici->fMask & CMIC_MASK_UNICODE))
		{
			fUnicode = TRUE;
		}
	}

	if( !fUnicode && HIWORD(pici->lpVerb))
	{
		if(StrCmpIA(pici->lpVerb,szVerbRunConsoleA)) return E_FAIL;
	}
	else if( fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX *) pici)->lpVerbW))
	{
		if(StrCmpIW(((CMINVOKECOMMANDINFOEX *)pici)->lpVerbW,szVerbRunConsoleW)) return E_FAIL;
	}
	else if(LOWORD(pici->lpVerb) == eMC_RunConsole)
	{// run console with default tab
		if(!sPath.IsEmpty())
		{
			RunConsole(sPath);
		}
	}
	else
	{// run console with specified tab
		if(!sPath.IsEmpty())
		{
			wstring		sTabName;

			// get tab name
			{
				// synchronization
				syncAutoLock	oLock(g_oSync);

				if((LOWORD(pici->lpVerb)-eMC_RunConsoleWithTab)<(int)g_vTabs.size())
					sTabName = g_vTabs[LOWORD(pici->lpVerb)-eMC_RunConsoleWithTab]->sName;
			}

			// run
			RunConsole(sPath,sTabName.c_str());
		}
	}

	return S_OK;
}

STDMETHODIMP CContextMenuHandler::GetCommandString(UINT_PTR idCmd,UINT uType,UINT *pReserved,LPSTR pszName,UINT cchMax)
{
#ifdef DEBUG_TO_LOG_FILES
	char	tbuf[200];
	sprintf_s(tbuf,200,__FUNCTION__ ": cmd=%lu, type=%u, name=%s",(DWORD)idCmd,uType,pszName);
	f_log(tbuf);
#endif

	HRESULT  hr = E_INVALIDARG;

//	if(idCmd != eMC_RunConsole) return hr;

	switch(uType)
	{
	case GCS_HELPTEXTA: hr=StringCbCopyNA(pszName,cchMax,szDescrRunConsoleA,sizeof(szDescrRunConsoleA)); break; 
	case GCS_HELPTEXTW: hr=StringCbCopyNW((LPWSTR)pszName,cchMax*sizeof(wchar_t),szDescrRunConsoleW,sizeof(szDescrRunConsoleW)); break; 
	case GCS_VERBA: hr=StringCbCopyNA(pszName,cchMax,szVerbRunConsoleA,sizeof(szVerbRunConsoleA)); break; 
	case GCS_VERBW: hr=StringCbCopyNW((LPWSTR)pszName,cchMax*sizeof(wchar_t),szVerbRunConsoleW,sizeof(szVerbRunConsoleW)); break; 
	default:
		hr = S_OK;
		break; 
	}
	return hr;
}

int CContextMenuHandler::RunConsole(LPCTSTR sQueriedPath,LPCTSTR sQueriedTab)
{
	// we cannot run without path
	if(!sQueriedPath) return E_FAIL;

	// is path folder or file?
	CString	sCheckedPath = sQueriedPath;
	DWORD	iFA = GetFileAttributes(sQueriedPath);
	if(iFA==0xFFFFFFFF) {f_log("Cannot get path attributes - assume it is folder");}
	else if(!(iFA&FILE_ATTRIBUTE_DIRECTORY))
	{// it is file - trim it
		int	dir_index = sCheckedPath.ReverseFind(_T('\\'));
		if(dir_index==-1)
		{// hmm, there is no folder separator so path is relative to current dir, get it as path
			sCheckedPath = _T(".");
		}
		else sCheckedPath=sCheckedPath.Left(dir_index);
	}

	// construct command line
	CString	sCmdLine = _T("\"");
	// construct executable name (quoted to avoid problems with spaces in path)
	sCmdLine += sExePath;
	sCmdLine += szExeName;
	sCmdLine += _T("\" ");
	// construct parameters
	if(sQueriedTab&&(_tcslen(sQueriedTab)>0))
	{
		sCmdLine += _T("-t ");
//		sCmdLine += _T("-t \"");
		sCmdLine += sQueriedTab;
		sCmdLine += _T(" ");
//		sCmdLine += _T("\" ");
	}
	sCmdLine += _T("-d \"");
	sCmdLine += sCheckedPath;
	sCmdLine += _T("\"");

#ifdef DEBUG_TO_LOG_FILES
	char	tbuf[MAX_PATH];
	sprintf_s(tbuf,MAX_PATH,__FUNCTION__ ": cmd=%S",sCmdLine.GetString());
	f_log(tbuf);
#endif

	// Run process
	STARTUPINFO	si;
	PROCESS_INFORMATION	pi;
	memset(&si,0,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if (CreateProcess(NULL,sCmdLine.GetBuffer(),NULL,NULL,FALSE,DETACHED_PROCESS,NULL,NULL,&si,&pi))
	{
		// Success
		// Close handles
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else f_log("Cannot start console process");

	return S_OK;
}