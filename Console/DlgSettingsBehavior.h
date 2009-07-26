
#pragma once

#include "DlgSettingsBase.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsBehavior 
	: public DlgSettingsBase
{
	public:

		DlgSettingsBehavior(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsBehavior)
			DDX_CHECK(IDC_CHECK_COPY_ON_SELECT, m_nCopyOnSelect)
			DDX_CHECK(IDC_CHECK_CLEAR_ON_COPY, m_nClearOnCopy)
			DDX_CHECK(IDC_CHECK_NO_WRAP, m_nNoWrap)
			DDX_CHECK(IDC_CHECK_TRIM_SPACES, m_nTrimSpaces)
			DDX_RADIO(IDC_RADIO_COPY_NEWLINE_CHAR, m_nCopyNewlineChar)
			DDX_RADIO(IDC_PAGE_SCROLL, m_nScrollPageType)
			DDX_UINT(IDC_SCROLL_PAGE_ROWS, m_behaviorSettings.scrollSettings.dwPageScrollRows)
			DDX_CHECK(IDC_CHECK_FLASH_TAB, m_nFlashInactiveTab)
			DDX_UINT(IDC_TAB_FLASHES, m_behaviorSettings.tabHighlightSettings.dwFlashes)
			DDX_CHECK(IDC_CHECK_LEAVE_HIGHLIGHTED, m_nLeaveHighlighted)

			DDX_CHECK(IDC_ALLOW_MULTIPLE_INSTANCES, m_nAllowMultipleInstances)
			DDX_CHECK(IDC_REUSE_TAB, m_nReuseTab)
			DDX_CHECK(IDC_REUSE_BUSY_TAB, m_nReuseBusyTab)
			DDX_CHECK(IDC_INTEGRATE_WITH_EXPLORER, m_nIntegrateWithExplorer)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsBehavior)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_RANGE_CODE_HANDLER(IDC_PAGE_SCROLL, IDC_PAGE_SCROLL2, BN_CLICKED, OnClickedScrollType)
			COMMAND_HANDLER(IDC_CHECK_FLASH_TAB, BN_CLICKED, OnClickedFlashTab)

			COMMAND_HANDLER(IDC_REUSE_TAB, BN_CLICKED, OnClickedReuseTab)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedScrollType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedFlashTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnClickedReuseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/); // vds:

	private:

		void EnableScrollControls();
		void EnableFlashTabControls();
		void EnableOnInstanceControls(); // vds:

	private:

		BehaviorSettings	m_behaviorSettings;

		int					m_nCopyOnSelect;
		int					m_nClearOnCopy;
		int					m_nNoWrap;
		int					m_nTrimSpaces;

		int					m_nCopyNewlineChar;

		int					m_nScrollPageType;

		int					m_nFlashInactiveTab;
		int					m_nLeaveHighlighted;

		// vds: >>
		int					m_nAllowMultipleInstances;
		int					m_nReuseTab;
		int					m_nReuseBusyTab;

		int					m_nIntegrateWithExplorer;
		// vds: <<

};

//////////////////////////////////////////////////////////////////////////////
