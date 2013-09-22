#include "stdafx.h"
#include "resource.h"

#include "DlgSettingsBehavior.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsBehavior::DlgSettingsBehavior(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
{
	IDD = IDD_SETTINGS_BEHAVIOR;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsBehavior::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_behaviorSettings.Load(m_pOptionsRoot);

	m_sessionsSettings.Load(m_pOptionsRoot);

	m_nCopyNewlineChar	= static_cast<int>(m_behaviorSettings.copyPasteSettings.copyNewlineChar);
	m_nScrollPageType	= m_behaviorSettings.scrollSettings.dwPageScrollRows ? 1 : 0;
	m_bFlashInactiveTab	= (m_behaviorSettings.tabHighlightSettings.dwFlashes > 0);
	// vds: >>
	m_nAllowMultipleInstances = m_behaviorSettings.oneInstanceSettings.bAllowMultipleInstances ? 1 : 0;
	m_nReuseTab = m_behaviorSettings.oneInstanceSettings.bReuseTab ? 1 : 0;
	m_nReuseBusyTab = m_behaviorSettings.oneInstanceSettings.bReuseBusyTab ? 1 : 0;

	m_nIntegrateWithExplorer = m_behaviorSettings.shellSettings.IsConsoleIntegratedWithExplorer();
	m_nRunConsoleMenuItem = m_behaviorSettings.shellSettings.bRunConsoleMenItem ? 1 : 0;
	m_nRunConsoleTabMenuItem = m_behaviorSettings.shellSettings.bRunConsoleTabMenuItem ? 1 : 0;
	m_nPostConsoleMenuItem = m_behaviorSettings.shellSettings.bPostConsoleMenItem ? 1 : 0;
	m_nPostConsoleTabMenuItem = m_behaviorSettings.shellSettings.bPostConsoleTabMenuItem ? 1 : 0;
	// vds: <<

	m_nRestoreSessions = m_sessionsSettings.bRestoreTabs ? 1 : 0; // vds: sessions

	CUpDownCtrl	spin;
	UDACCEL		udAccel;

	spin.Attach(GetDlgItem(IDC_SPIN_SCROLL_PAGE_ROWS));
	spin.SetRange(1, 500);
	udAccel.nSec = 2;
	udAccel.nInc = 1;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_TAB_FLASHES));
	spin.SetRange(1, 500);
	udAccel.nSec = 2;
	udAccel.nInc = 1;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	DoDataExchange(DDX_LOAD);

	EnableScrollControls();
	EnableFlashTabControls();

	// vds: >>
	if (!m_behaviorSettings.shellSettings.CouldIntegrateConsoleWithExplorer()) {
		GetDlgItem(IDC_INTEGRATE_WITH_EXPLORER).EnableWindow(False);
	}
	EnableOnInstanceControls();
	// vds: <<

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsBehavior::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		DoDataExchange(DDX_SAVE);

		m_behaviorSettings.copyPasteSettings.copyNewlineChar= static_cast<CopyNewlineChar>(m_nCopyNewlineChar);

		if (m_nScrollPageType == 0) m_behaviorSettings.scrollSettings.dwPageScrollRows = 0;

		if (!m_bFlashInactiveTab) m_behaviorSettings.tabHighlightSettings.dwFlashes = 0;

		// vds: >>
		m_behaviorSettings.oneInstanceSettings.bAllowMultipleInstances = (m_nAllowMultipleInstances > 0);
		m_behaviorSettings.oneInstanceSettings.bReuseTab = (m_nReuseTab > 0);
		m_behaviorSettings.oneInstanceSettings.bReuseBusyTab = (m_nReuseBusyTab > 0);

		if (m_nIntegrateWithExplorer) {
			if (!m_behaviorSettings.shellSettings.IsConsoleIntegratedWithExplorer())
				m_behaviorSettings.shellSettings.IntegrateConsoleWithExplorer(true);
		}
		else {
			if (m_behaviorSettings.shellSettings.IsConsoleIntegratedWithExplorer())
				m_behaviorSettings.shellSettings.IntegrateConsoleWithExplorer(false);
		}
		m_behaviorSettings.shellSettings.bRunConsoleMenItem = (m_nRunConsoleMenuItem > 0);
		m_behaviorSettings.shellSettings.bRunConsoleTabMenuItem = (m_nRunConsoleTabMenuItem > 0);
		m_behaviorSettings.shellSettings.bPostConsoleMenItem = (m_nPostConsoleMenuItem > 0);
		m_behaviorSettings.shellSettings.bPostConsoleTabMenuItem = (m_nPostConsoleTabMenuItem > 0);
		// vds: <<

		m_sessionsSettings.bRestoreTabs = (m_nRestoreSessions > 0); // vds: sessions

		BehaviorSettings& behaviorSettings = g_settingsHandler->GetBehaviorSettings();

		behaviorSettings = m_behaviorSettings;
		m_behaviorSettings.Save(m_pOptionsRoot);

		m_sessionsSettings.bRestoreTabs = (m_nRestoreSessions > 0); // vds: sessions

		SessionsSettings& sessionsSettings = g_settingsHandler->GetSessionsSettings();

		sessionsSettings = m_sessionsSettings;
		m_sessionsSettings.Save(m_pOptionsRoot);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsBehavior::OnClickedScrollType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableScrollControls();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsBehavior::OnClickedFlashTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableFlashTabControls();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// vds: >>
LRESULT DlgSettingsBehavior::OnClickedReuseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableOnInstanceControls();
	return 0;
}
// vds: <<
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

// vds: >>
LRESULT DlgSettingsBehavior::OnClickedIntegrateWithExplorer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableOnInstanceControls();
	return 0;
}
// vds: <<
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

void DlgSettingsBehavior::EnableScrollControls()
{
	GetDlgItem(IDC_SCROLL_PAGE_ROWS).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_SCROLL_PAGE_ROWS).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_ROWS).EnableWindow(FALSE);

	if (m_nScrollPageType > 0)
	{
		if (m_behaviorSettings.scrollSettings.dwPageScrollRows == 0)
		{
			m_behaviorSettings.scrollSettings.dwPageScrollRows = 1;
			DoDataExchange(DDX_LOAD);
		}

		GetDlgItem(IDC_SCROLL_PAGE_ROWS).EnableWindow();
		GetDlgItem(IDC_SPIN_SCROLL_PAGE_ROWS).EnableWindow();
		GetDlgItem(IDC_STATIC_ROWS).EnableWindow();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsBehavior::EnableFlashTabControls()
{
	GetDlgItem(IDC_TAB_FLASHES).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_TAB_FLASHES).EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_LEAVE_HIGHLIGHTED).EnableWindow(FALSE);

	if (m_bFlashInactiveTab)
	{
		if (m_behaviorSettings.tabHighlightSettings.dwFlashes == 0)
		{
			m_behaviorSettings.tabHighlightSettings.dwFlashes = 1;
			DoDataExchange(DDX_LOAD);
		}

		GetDlgItem(IDC_TAB_FLASHES).EnableWindow();
		GetDlgItem(IDC_SPIN_TAB_FLASHES).EnableWindow();
		GetDlgItem(IDC_CHECK_LEAVE_HIGHLIGHTED).EnableWindow();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

// vds: >>
void DlgSettingsBehavior::EnableOnInstanceControls()
{
	GetDlgItem(IDC_REUSE_BUSY_TAB).EnableWindow(FALSE);

	if (m_nReuseTab) {
		GetDlgItem(IDC_REUSE_BUSY_TAB).EnableWindow();
	}

	GetDlgItem(IDC_RUN_CONSOLE).EnableWindow(FALSE);
	GetDlgItem(IDC_RUN_CONSOLE_TAB).EnableWindow(FALSE);
	GetDlgItem(IDC_POST_CONSOLE).EnableWindow(FALSE);
	GetDlgItem(IDC_POST_CONSOLE_TAB).EnableWindow(FALSE);
	
	if (m_nIntegrateWithExplorer) {
		GetDlgItem(IDC_RUN_CONSOLE).EnableWindow();
		GetDlgItem(IDC_RUN_CONSOLE_TAB).EnableWindow();
		GetDlgItem(IDC_POST_CONSOLE).EnableWindow();
		GetDlgItem(IDC_POST_CONSOLE_TAB).EnableWindow();
	}
}
// vds: <<

//////////////////////////////////////////////////////////////////////////////


