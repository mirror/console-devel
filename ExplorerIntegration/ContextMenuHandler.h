// ContextMenuHandler.h : Declaration of the CContextMenuHandler

#pragma once
#include "resource.h"       // main symbols

#include "ExplorerIntegration_i.h"
#include <ShlObj.h>
#include <vector>

// Command IDs
enum eContextMenuCommands{
	eMC_RunConsole = 0,			///< run console with first tab
	eMC_RunConsoleWithTabFake,
	eMC_RunConsoleWithTab,		///< run console with selected tab
};

// Used registry structure
// HKEY_LOCAL_MACHINE\Software\Console\Explorer Integration\
// (REG_SZ)Path - path to Console executable
// HKEY_CURRENT_USER\Software\Console\Explorer Integration\<tab name>\
// (REG_SZ,optional)Icon - path to icon


// CContextMenuHandler

class ATL_NO_VTABLE CContextMenuHandler :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CContextMenuHandler, &CLSID_ContextMenuHandler>,
	public ISupportErrorInfo,
	public IShellExtInit,
	public IContextMenu
{
public:
	CContextMenuHandler();

DECLARE_REGISTRY_RESOURCEID(IDR_CONTEXTMENUHANDLER)

DECLARE_NOT_AGGREGATABLE(CContextMenuHandler)

BEGIN_COM_MAP(CContextMenuHandler)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();

	void FinalRelease();

protected:
	CString					sPath;			///> path to selected folder
	CString					sExePath;		///> path to console executable
//	std::vector<CString>	vTabs;			///> vector of known tabs (it is stored in registry because reparsing config on every object creation is a bad idea)
#ifdef DEBUG_TO_LOG_FILES
	HANDLE			m_log;
#endif

	int	LoadConfig();		///> load config from registry
	int	RunConsole(LPCTSTR sQueriedPath,LPCTSTR sQueriedTab=NULL);		///> run console

public:
	STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidlFolder,IDataObject *pdtobj,HKEY hkeyProgID);
	STDMETHOD(QueryContextMenu)(HMENU hmenu,UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags);
	STDMETHOD(InvokeCommand)(CMINVOKECOMMANDINFO *pici);
	STDMETHOD(GetCommandString)(UINT_PTR idCmd,UINT uType,UINT *pReserved,LPSTR pszName,UINT cchMax);
};

OBJECT_ENTRY_AUTO(__uuidof(ContextMenuHandler), CContextMenuHandler)
