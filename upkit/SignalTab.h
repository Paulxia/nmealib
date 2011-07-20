// CSignalTab.h : interface of the CSignalTab class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DevManager.h"

class CSignalTab
    : public CDialogImpl<CSignalTab>
    , public DevHandler
{
public:
	enum { IDD = IDD_SIGNALTAB };

	BEGIN_MSG_MAP(CSignalTab)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
        COMMAND_HANDLER(IDC_BUTTON1, BN_CLICKED, OnBnClickedButton1)
        COMMAND_HANDLER(IDC_BUTTON2, BN_CLICKED, OnBnClickedButton2)
    END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedButton2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

protected:
    virtual void onDevChange();

    void _checkValues();

};
