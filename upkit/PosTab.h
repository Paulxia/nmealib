// CPosTab.h : interface of the CPosTab class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DevManager.h"

class CPosTab
    : public CDialogImpl<CPosTab>
    , public DevHandler
{
public:
	enum { IDD = IDD_POSTAB };

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_HANDLER(IDC_BUTTON1, BN_CLICKED, OnBnClickedButton1)
    END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

protected:
    virtual void onDevChange();

};
