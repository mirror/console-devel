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
const char		szVerbRunConsoleA[] = "run";
const wchar_t	szVerbRunConsoleW[] = L"run";
const char		szDescrRunConsoleA[] = "Run Console here";
const wchar_t	szDescrRunConsoleW[] = L"Run Console here";

TCHAR			szItemPostConsole[] = _T("Post Console");
const char		szVerbPostConsoleA[] = "post";
const wchar_t	szVerbPostConsoleW[] = L"post";
const char		szDescrPostConsoleA[] = "Post Console here";
const wchar_t	szDescrPostConsoleW[] = L"Post Console here";

TCHAR			szItemRunConsoleWithTab[] = _T("Run Console Tab");
const char		szVerbRunConsoleWithTabA[] = "run_tab";
const wchar_t	szVerbRunConsoleWithTabW[] = L"run_tab";
const char		szDescrRunConsoleWithTabA[] = "Run Console Tab here";
const wchar_t	szDescrRunConsoleWithTabW[] = L"Run Console Tab here";

TCHAR			szItemPostConsoleWithTab[] = _T("Post Console Tab");
const char		szVerbPostConsoleWithTabA[] = "post_tab";
const wchar_t	szVerbPostConsoleWithTabW[] = L"post_tab";
const char		szDescrPostConsoleWithTabA[] = "Post Console Tab here";
const wchar_t	szDescrPostConsoleWithTabW[] = L"Post Console Tab here";

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

	// reload global config if required
	if(hConfigChanged)
		if(WaitForSingleObject(hConfigChanged,0)==WAIT_OBJECT_0)
		{
			LoadConsoleSettings();
//			::MessageBox(NULL,L"Reloading console settings",L"Info",MB_ICONINFORMATION|MB_OK);		// DEBUG
		}

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

STDMETHODIMP CContextMenuHandler::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
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

	m_idCmdFirst = idCmdFirst;
	DWORD lastId = idCmdFirst;

	// Synchronization
	syncAutoLock	oLock(g_oSync);			// This lock is REQUIRED to protect shared configuration access

	{
		MENUITEMINFO miSeparator;
		miSeparator.fMask = MIIM_TYPE;
		miSeparator.fType = MFT_SEPARATOR;
		InsertMenuItem(hmenu, indexMenu++, TRUE, &miSeparator);
	}

	if (g_settingsHandler->GetBehaviorSettings().shellSettings.bRunConsoleMenItem)
	{
		// get item name
		wstring sItemName = g_settingsHandler->GetInternationalizationSettings().strExplorerMenuRunItem;
		if (sItemName.size() <= 0)
			sItemName = szItemRunConsole;

		// Fill main menu first item info
		MENUITEMINFO miRunConsole;
		memset(&miRunConsole, 0, sizeof(MENUITEMINFO));
		miRunConsole.cbSize = sizeof(MENUITEMINFO);
		miRunConsole.fMask = 0;

		miRunConsole.fMask |= MIIM_ID;
		miRunConsole.wID = idCmdFirst + eMC_RunConsole;

		miRunConsole.fMask |= MIIM_TYPE;
		miRunConsole.fType = MFT_STRING;
		miRunConsole.dwTypeData = const_cast<LPTSTR>(sItemName.c_str());
		miRunConsole.cch = sItemName.size();

		miRunConsole.fMask |= MIIM_STATE;
		miRunConsole.fState = MFS_ENABLED;

		miRunConsole.fMask |= MIIM_CHECKMARKS;
		miRunConsole.hbmpChecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
		miRunConsole.hbmpUnchecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);

		// Insert menu item
		BOOL r = InsertMenuItem(hmenu, indexMenu, TRUE, &miRunConsole);
		indexMenu++;

		lastId = idCmdFirst + eMC_RunConsole;
	}

	if (g_settingsHandler->GetBehaviorSettings().shellSettings.bPostConsoleMenItem)
	{
		// get item name
		wstring sItemName = g_settingsHandler->GetInternationalizationSettings().strExplorerMenuPostItem;
		if (sItemName.size() <= 0)
			sItemName = szItemPostConsole;

		// Fill main menu first item info
		MENUITEMINFO miPostConsole;
		memset(&miPostConsole, 0, sizeof(MENUITEMINFO));
		miPostConsole.cbSize = sizeof(MENUITEMINFO);

		miPostConsole.fMask = 0;

		miPostConsole.fMask |= MIIM_ID;
		miPostConsole.wID = idCmdFirst + eMC_PostConsole;

		miPostConsole.fMask |= MIIM_TYPE;
		miPostConsole.fType = MFT_STRING;
		miPostConsole.dwTypeData = const_cast<LPTSTR>(sItemName.c_str());
		miPostConsole.cch = sItemName.size();

		miPostConsole.fMask |= MIIM_STATE;
		miPostConsole.fState = MFS_ENABLED;

		miPostConsole.fMask |= MIIM_CHECKMARKS;
		miPostConsole.hbmpChecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
		miPostConsole.hbmpUnchecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);

		// Insert menu item
		BOOL r = InsertMenuItem(hmenu, indexMenu, TRUE, &miPostConsole);
		indexMenu++;

		lastId = idCmdFirst + eMC_PostConsole;
	}

	if (g_settingsHandler->GetBehaviorSettings().shellSettings.bRunConsoleTabMenuItem) {
		HMENU hSubMenu = CreateMenu();
		if (!hSubMenu)
			return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));

		// Create submenu
		for (DWORD i = 0, lim = g_vTabs.size(); i < lim; ++i) {
			wstring sTabName = g_vTabs[i]->sName;
			HBITMAP hTabIcon = g_vTabs[i]->hIconBmp;

			// Fill main menu item info
			MENUITEMINFO miRunTabConsole;
			memset(&miRunTabConsole, 0, sizeof(MENUITEMINFO));
			miRunTabConsole.cbSize = sizeof(MENUITEMINFO);
			miRunTabConsole.fMask = MIIM_STRING | MIIM_ID | (hTabIcon ? MIIM_CHECKMARKS : 0);
			miRunTabConsole.wID = idCmdFirst + eMC_RunConsoleWithTab + i;
			miRunTabConsole.dwTypeData = const_cast<LPTSTR>(sTabName.c_str());
			miRunTabConsole.cch = sTabName.size();
			if (hTabIcon)
				miRunTabConsole.hbmpChecked = miRunTabConsole.hbmpUnchecked = hTabIcon;
			// Insert menu item
			InsertMenuItem(hSubMenu, i, TRUE, &miRunTabConsole);

			lastId = miRunTabConsole.wID;
		}
		// get item name
		wstring sItemName = g_settingsHandler->GetInternationalizationSettings().strExplorerMenuRunWithItem;
		if (sItemName.size() <= 0)
			sItemName = szItemRunConsoleWithTab;

		// Fill main menu item info
		MENUITEMINFO miRunTabConsoleMenu;
		memset(&miRunTabConsoleMenu, 0, sizeof(MENUITEMINFO));
		miRunTabConsoleMenu.cbSize = sizeof(MENUITEMINFO);
		miRunTabConsoleMenu.fMask = MIIM_CHECKMARKS|MIIM_STRING|MIIM_ID|MIIM_SUBMENU;
		miRunTabConsoleMenu.wID = idCmdFirst + eMC_RunConsoleWithTabFake;
		miRunTabConsoleMenu.dwTypeData = const_cast<LPTSTR>(sItemName.c_str());
		miRunTabConsoleMenu.cch = sItemName.size();
		miRunTabConsoleMenu.hSubMenu = hSubMenu;
		miRunTabConsoleMenu.hbmpChecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
		miRunTabConsoleMenu.hbmpUnchecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);

		// Insert menu item
		InsertMenuItem(hmenu, indexMenu++, TRUE, &miRunTabConsoleMenu);

		lastId = idCmdFirst + eMC_RunConsoleWithTab + g_vTabs.size() - 1;
	}

	if (g_settingsHandler->GetBehaviorSettings().shellSettings.bPostConsoleTabMenuItem) {
		HMENU hSubMenu = CreateMenu();
		if (!hSubMenu)
			return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));

		// Create submenu
		for (DWORD i = 0, lim = g_vTabs.size(); i < lim; ++i) {
			wstring sTabName = g_vTabs[i]->sName;
			HBITMAP hTabIcon = g_vTabs[i]->hIconBmp;
			
			// Fill main menu item info
			MENUITEMINFO miPostTabConsole;
			memset(&miPostTabConsole, 0, sizeof(MENUITEMINFO));
			miPostTabConsole.cbSize = sizeof(MENUITEMINFO);
			miPostTabConsole.fMask = MIIM_STRING | MIIM_ID | (hTabIcon ? MIIM_CHECKMARKS : 0);
			miPostTabConsole.wID = idCmdFirst + eMC_PostConsoleWithTab + i;
			miPostTabConsole.dwTypeData = const_cast<LPTSTR>(sTabName.c_str());
			miPostTabConsole.cch = sTabName.size();
			if (hTabIcon)
				miPostTabConsole.hbmpChecked = miPostTabConsole.hbmpUnchecked = hTabIcon;
			// Insert menu item
			InsertMenuItem(hSubMenu, i, TRUE, &miPostTabConsole);

			lastId = miPostTabConsole.wID;
		}

		// get item name
		wstring sItemName = g_settingsHandler->GetInternationalizationSettings().strExplorerMenuPostWithItem;
		if (sItemName.size() <= 0)
			sItemName = szItemPostConsoleWithTab;

		// Fill main menu item info
		MENUITEMINFO miPostTabConsoleMenu;
		memset(&miPostTabConsoleMenu, 0, sizeof(MENUITEMINFO));
		miPostTabConsoleMenu.cbSize = sizeof(MENUITEMINFO);
		miPostTabConsoleMenu.fMask = MIIM_CHECKMARKS|MIIM_STRING|MIIM_ID|MIIM_SUBMENU;
		miPostTabConsoleMenu.wID = idCmdFirst + eMC_PostConsoleWithTabFake;
		miPostTabConsoleMenu.dwTypeData = const_cast<LPTSTR>(sItemName.c_str());
		miPostTabConsoleMenu.cch = sItemName.size();
		miPostTabConsoleMenu.hSubMenu = hSubMenu;
		miPostTabConsoleMenu.hbmpChecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
		miPostTabConsoleMenu.hbmpUnchecked = (HBITMAP)LoadImage(_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_MENU_PICTURE),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
		// Insert menu item
		InsertMenuItem(hmenu, indexMenu++, TRUE, &miPostTabConsoleMenu);

		lastId = idCmdFirst + eMC_PostConsoleWithTab + g_vTabs.size() - 1;
	}

#if 0
	{
		MENUITEMINFO miSeparator;
		miSeparator.fMask = MIIM_TYPE;
		miSeparator.fType = MFT_SEPARATOR;
		InsertMenuItem(hmenu, indexMenu++, TRUE, &miSeparator);
	}
#endif

	return ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(lastId - idCmdFirst + 1)));
	//return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(lastId - idCmdFirst + 1));
}

STDMETHODIMP CContextMenuHandler::InvokeCommand(CMINVOKECOMMANDINFO *pici)
{
	f_log(__FUNCTION__);

	BOOL fEx = FALSE;
	BOOL fUnicode = FALSE;

	if (pici->cbSize = sizeof(CMINVOKECOMMANDINFOEX)) {
		fEx = TRUE;
		if ((pici->fMask & CMIC_MASK_UNICODE)) {
			fUnicode = TRUE;
		}
	}

	if (!fUnicode && HIWORD(pici->lpVerb)) {
		if (StrCmpIA(pici->lpVerb, szVerbRunConsoleA) == 0) {
			if (!sPath.IsEmpty()) {
				RunConsole(sPath, false);
			}
			return S_OK;
		}
		else if (StrCmpIA(pici->lpVerb, szVerbPostConsoleA) == 0) {
			if (!sPath.IsEmpty()) {
				RunConsole(sPath, true);
			}
			return S_OK;
		}
		return E_FAIL;
	}
	
	if (fUnicode && HIWORD(reinterpret_cast<CMINVOKECOMMANDINFOEX*>(pici)->lpVerbW)) {
		if (StrCmpIW(reinterpret_cast<CMINVOKECOMMANDINFOEX*>(pici)->lpVerbW, szVerbRunConsoleW) == 0) {
			if (!sPath.IsEmpty()) {
				RunConsole(sPath, false);
			}
			return S_OK;
		}
		else if (StrCmpIW(reinterpret_cast<CMINVOKECOMMANDINFOEX*>(pici)->lpVerbW, szVerbRunConsoleW) == 0) {
			if (!sPath.IsEmpty()) {
				RunConsole(sPath, true);
			}
			return S_OK;
		}
		return E_FAIL;
	}

	if (LOWORD(pici->lpVerb) == eMC_RunConsole) {
		// Run console with default tab
		if(!sPath.IsEmpty())
		{
			RunConsole(sPath, false);
		}
	}
	else if (LOWORD(pici->lpVerb) == eMC_PostConsole) {
		// Run console with default tab
		if(!sPath.IsEmpty())
		{
			RunConsole(sPath, true);
		}
	}
	else if (LOWORD(pici->lpVerb) >= eMC_RunConsoleWithTab && LOWORD(pici->lpVerb) < eMC_RunConsoleWithTab + 50) {
		// Run console with specified tab
		if (!sPath.IsEmpty()) {
			wstring sTabName;

			// get tab name
			{
				// synchronization
				syncAutoLock oLock(g_oSync);

				if ((LOWORD(pici->lpVerb) - eMC_RunConsoleWithTab) < (int)g_vTabs.size())
					sTabName = g_vTabs[LOWORD(pici->lpVerb) - eMC_RunConsoleWithTab]->sName;
			}

			// run
			RunConsole(sPath, false, sTabName.c_str());
		}
	}
	else if (LOWORD(pici->lpVerb) >= eMC_PostConsoleWithTab && LOWORD(pici->lpVerb) < eMC_PostConsoleWithTab + 50) {
		// Run console with specified tab
		if (!sPath.IsEmpty()) {
			wstring sTabName;

			// get tab name
			{
				// synchronization
				syncAutoLock oLock(g_oSync);

				if ((LOWORD(pici->lpVerb) - eMC_RunConsoleWithTab) < (int)g_vTabs.size())
					sTabName = g_vTabs[LOWORD(pici->lpVerb) - eMC_PostConsoleWithTab]->sName;
			}

			// run
			RunConsole(sPath, true, sTabName.c_str());
		}
	}

	return S_OK;
}

STDMETHODIMP CContextMenuHandler::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pReserved, LPSTR pszName,UINT cchMax)
{
#ifdef DEBUG_TO_LOG_FILES
	char	tbuf[200];
	sprintf_s(tbuf,200,__FUNCTION__ ": cmd=%lu, type=%u, name=%s",(DWORD)idCmd,uType,pszName);
	f_log(tbuf);
#endif

	HRESULT  hr = E_INVALIDARG;

	const char *aHelp = "";
	size_t aHelpSize = 0;
	const wchar_t *wHelp = L"";
	size_t wHelpSize = 0;

	const char *aVerb = "";
	size_t aVerbSize = 0;
	const wchar_t *wVerb = L"";
	size_t wVerbSize = 0;

	if (idCmd == m_idCmdFirst + eMC_RunConsole) {
		aHelp = szDescrRunConsoleA;
		aHelpSize = sizeof(szDescrRunConsoleA);

		wHelp = szDescrRunConsoleW;
		wHelpSize = sizeof(szDescrRunConsoleW);

		aVerb = szVerbRunConsoleA;
		aVerbSize = sizeof(szVerbRunConsoleA);

		wVerb = szVerbRunConsoleW;
		wVerbSize = sizeof(szVerbRunConsoleW);
	}
	else if (idCmd == m_idCmdFirst + eMC_PostConsole) {
		aHelp = szDescrPostConsoleA;
		aHelpSize = sizeof(szDescrPostConsoleA);

		wHelp = szDescrPostConsoleW;
		wHelpSize = sizeof(szDescrPostConsoleW);

		aVerb = szVerbPostConsoleA;
		aVerbSize = sizeof(szVerbPostConsoleA);

		wVerb = szVerbPostConsoleW;
		wVerbSize = sizeof(szVerbPostConsoleW);
	}
	if (idCmd == m_idCmdFirst + eMC_RunConsoleWithTabFake) {
		aHelp = szDescrRunConsoleWithTabA;
		aHelpSize = sizeof(szDescrRunConsoleWithTabA);

		wHelp = szDescrRunConsoleWithTabW;
		wHelpSize = sizeof(szDescrRunConsoleWithTabW);

		aVerb = szVerbRunConsoleWithTabA;
		aVerbSize = sizeof(szVerbRunConsoleWithTabA);

		wVerb = szVerbRunConsoleW;
		wVerbSize = sizeof(szVerbRunConsoleWithTabW);
	}
	if (idCmd == m_idCmdFirst + eMC_PostConsoleWithTabFake) {
		aHelp = szDescrPostConsoleWithTabA;
		aHelpSize = sizeof(szDescrPostConsoleWithTabA);

		wHelp = szDescrPostConsoleWithTabW;
		wHelpSize = sizeof(szDescrPostConsoleWithTabW);

		aVerb = szVerbPostConsoleWithTabA;
		aVerbSize = sizeof(szVerbPostConsoleWithTabA);

		wVerb = szVerbPostConsoleWithTabW;
		wVerbSize = sizeof(szVerbPostConsoleWithTabW);
	}
	else {
	}

	switch(uType) {
	case GCS_HELPTEXTA:
		hr = StringCbCopyNA(pszName, cchMax, aHelp, aHelpSize);
		break; 

	case GCS_HELPTEXTW:
		hr = StringCbCopyNW((LPWSTR)pszName, cchMax * sizeof(wchar_t), wHelp, wHelpSize);
		break; 

	case GCS_VERBA:
		hr = StringCbCopyNA(pszName, cchMax, aVerb, aVerbSize);
		break; 

	case GCS_VERBW:
		hr = StringCbCopyNW((LPWSTR)pszName, cchMax * sizeof(wchar_t), wVerb, wVerbSize);
		break; 

	default:
		hr = S_OK;
		break; 
	}
	return hr;
}

int CContextMenuHandler::RunConsole(LPCTSTR sQueriedPath, bool post, LPCTSTR sQueriedTab)
{
	// We cannot run without path
	if (!sQueriedPath)
		return E_FAIL;

	// is path folder or file?
	CString queriedPath = sQueriedPath; // vds: poasted command

	CString	sCheckedPath = sQueriedPath;
	CString sCheckedFilePath = L""; // vds: posted command

	DWORD iFA = GetFileAttributes(sQueriedPath);
	if (iFA == 0xFFFFFFFF) {
		f_log("Cannot get path attributes - assume it is folder");
	}
	else if(!(iFA & FILE_ATTRIBUTE_DIRECTORY)) {
		// It is file - trim it
		int	dir_index = queriedPath.ReverseFind(_T('\\'));  // vds: posted command
		if (dir_index == -1) {
			// Hmm, there is no folder separator so path is relative to current dir, get it as path
			sCheckedPath = _T(".");
		}
		else {
			sCheckedPath = queriedPath.Left(dir_index);  // vds: posted command
			sCheckedFilePath = queriedPath.Mid(dir_index + 1); // vds: posted command
		}
	}

	// Construct command line
	CString	sCmdLine = _T("\"");

	// Construct executable name (quoted to avoid problems with spaces in path)
	sCmdLine += sExePath;
	sCmdLine += szExeName;
	sCmdLine += _T("\" ");

	// Construct parameters
	if (sQueriedTab && (_tcslen(sQueriedTab) > 0)) {
		sCmdLine += _T("-t ");
//		sCmdLine += _T("-t \"");
		sCmdLine += sQueriedTab;
		sCmdLine += _T(" ");
//		sCmdLine += _T("\" ");
	}

	sCmdLine += _T("-d \"");
	sCmdLine += sCheckedPath;
	sCmdLine += _T("\"");

	// vds: posted command >>
	if (post && sCheckedFilePath != "") {
		sCmdLine += _T(" -p \"");
		sCmdLine += sCheckedFilePath;
		sCmdLine += _T("\"");
	}
	// vds: posted command <<

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

	if (CreateProcess(NULL, sCmdLine.GetBuffer(), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi)) {
		// Success
		// Close handles
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else {
		f_log("Cannot start console process");
	}

	return S_OK;
}