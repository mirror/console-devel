// dllmain.h : Declaration of module class.

class CExplorerIntegrationModule : public CAtlDllModuleT< CExplorerIntegrationModule >
{
public :
	DECLARE_LIBID(LIBID_ExplorerIntegrationLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_EXPLORERINTEGRATION, "{58855FBC-787A-4618-8C1A-5F848DD68870}")
};

extern class CExplorerIntegrationModule _AtlModule;
