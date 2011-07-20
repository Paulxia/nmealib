// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"

#include "MainTab.h"
#include "PosTab.h"
#include "SignalTab.h"

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

    _tabctrl.Attach(GetDlgItem(IDC_TAB));
    _tabctrl.ShowWindow(SW_SHOW);

    _main_tab = new CMainTab;
    _main_tab->Create(_tabctrl);

    _pos_tab = new CPosTab;
    _pos_tab->Create(_tabctrl);

    _sig_tab = new CSignalTab;
    _sig_tab->Create(_tabctrl);
    
    {
        std::tstring main_tab_name = _T("Driver");

        TCITEM item;
        item.mask = TCIF_TEXT;
        item.pszText = &main_tab_name[0];
        item.cchTextMax = main_tab_name.length();

        _tabctrl.InsertItem(0, &item);
    }
    {
        std::tstring main_tab_name = _T("Position");

        TCITEM item;
        item.mask = TCIF_TEXT;
        item.pszText = &main_tab_name[0];
        item.cchTextMax = main_tab_name.length();

        _tabctrl.InsertItem(1, &item);
    }
    {
        std::tstring main_tab_name = _T("Signal");

        TCITEM item;
        item.mask = TCIF_TEXT;
        item.pszText = &main_tab_name[0];
        item.cchTextMax = main_tab_name.length();

        _tabctrl.InsertItem(2, &item);
    }    

    _selectTab(0);

    DevManager::specific()->RefreshInfo();

	return TRUE;
}

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    EndDialog(0);
    return 0;
}

LRESULT CMainDlg::OnTcnSelchangeTab(int idCtrl, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
    int cur = _tabctrl.GetCurSel();
    _selectTab(cur);
    return 0;
}

void CMainDlg::_selectTab(int id)
{
    WTL::CRect rect; _tabctrl.GetClientRect(rect);

    CWindow *wnd = 0;

    _main_tab->ShowWindow(SW_HIDE);
    _pos_tab->ShowWindow(SW_HIDE);
    _sig_tab->ShowWindow(SW_HIDE);

    switch(id)
    {
    case 0:
        wnd = _main_tab;
        break;
    case 1:
        wnd = _pos_tab;
        break;
    case 2:
        wnd = _sig_tab;
        break;
    };        

    wnd->ShowWindow(SW_SHOW);
    wnd->MoveWindow(5, 25, rect.Width() - 10, rect.Height() - 30);
}
