// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DevManager.h"

class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
        NOTIFY_HANDLER(IDC_TAB, TCN_SELCHANGE, OnTcnSelchangeTab)
    END_MSG_MAP()

    LRESULT OnTcnSelchangeTab(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
    void _selectTab(int id);

    CTabCtrl _tabctrl;
    class CMainTab *_main_tab;
    class CPosTab *_pos_tab;
    class CSignalTab *_sig_tab;

};
