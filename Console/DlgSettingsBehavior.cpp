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

	m_nCopyOnSelect	= m_behaviorSettings.copyPasteSettings.bCopyOnSelect ? 1 : 0;
	m_nClearOnCopy	= m_behaviorSettings.copyPasteSettings.bClearOnCopy ? 1 : 0;
	m_nNoWrap		= m_behaviorSettings.copyPasteSettings.bNoWrap ? 1 : 0;
	m_nTrimSpaces	= m_behaviorSettings.copyPasteSettings.bTrimSpaces ? 1 : 0;
	m_nCopyNewlineChar= static_cast<int>(m_behaviorSettings.copyPasteSettings.copyNewlineChar);

	m_nScrollPageType= m_behaviorSettings.scrollSettings.dwPageScrollRows ? 1 : 0;

	m_nFlashInactiveTab= m_behaviorSettings.tabHighlightSettings.dwFlashes > 0 ? 1 : 0;
	m_nLeaveHighlighted= m_behaviorSettings.tabHighlightSettings.bStayHighlighted ? 1 : 0;

	// vds: >>
	m_nAllowMultipleInstances = m_behaviorSettings.oneInstanceSettings.bAllowMultipleInstances ? 1 : 0;
	m_nReuseTab = m_behaviorSettings.oneInstanceSettings.bReuseTab ? 1 : 0;
	m_nReuseBusyTab = m_behaviorSettings.oneInstanceSettings.bReuseBusyTab ? 1 : 0;

	m_nIntegrateWithExplorer = m_behaviorSettings.oneInstanceSettings.IsConsoleIntegratedWithExplorer();
	// vds: <<

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
	EnableOnInstanceControls();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsBehavior::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		DoDataExchange(DDX_SAVE);

		m_behaviorSettings.copyPasteSettings.bCopyOnSelect	= (m_nCopyOnSelect > 0);
		m_behaviorSettings.copyPasteSettings.bClearOnCopy	= (m_nClearOnCopy > 0);
		m_behaviorSettings.copyPasteSettings.bNoWrap		= (m_nNoWrap > 0);
		m_behaviorSettings.copyPasteSettings.bTrimSpaces	= (m_nTrimSpaces > 0);
		m_behaviorSettings.copyPasteSettings.copyNewlineChar= static_cast<CopyNewlineChar>(m_nCopyNewlineChar);

		if (m_nScrollPageType == 0) m_behaviorSettings.scrollSettings.dwPageScrollRows = 0;

		if (m_nFlashInactiveTab == 0) m_behaviorSettings.tabHighlightSettings.dwFlashes = 0;
		m_behaviorSettings.tabHighlightSettings.bStayHighlighted = (m_nLeaveHighlighted > 0);

		// vds: >>
		m_behaviorSettings.oneInstanceSettings.bAllowMultipleInstances = (m_nAllowMultipleInstances > 0);
		m_behaviorSettings.oneInstanceSettings.bReuseTab = (m_nReuseTab > 0);
		m_behaviorSettings.oneInstanceSettings.bReuseBusyTab = (m_nReuseBusyTab > 0);

		if (m_nIntegrateWithExplorer) {
			if (!m_behaviorSettings.oneInstanceSettings.IsConsoleIntegratedWithExplorer())
				m_behaviorSettings.oneInstanceSettings.IntegrateConsoleWithExplorer(true);
		}
		else {
			if (m_behaviorSettings.oneInstanceSettings.IsConsoleIntegratedWithExplorer())
				m_behaviorSettings.oneInstanceSettings.IntegrateConsoleWithExplorer(false);
		}
		// vds: <<

		BehaviorSettings& behaviorSettings = g_settingsHandler->GetBehaviorSettings();

		behaviorSettings = m_behaviorSettings;
		m_behaviorSettings.Save(m_pOptionsRoot);
	}

	DestroyWindow();
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

LRESULT DlgSettingsBehavior::OnClickedReuseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableOnInstanceControls();
	return 0;
}

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

	if (m_nFlashInactiveTab > 0)
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
}
// vds: <<

//////////////////////////////////////////////////////////////////////////////


