// MainTab.cpp : implementation of the CMainTab class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "MainTab.h"

#include "DevManager.h"

LRESULT CMainTab::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    _dev_combo.Attach(GetDlgItem(IDC_DRV_COMBO));
    _dev_dectiv.Attach(GetDlgItem(IDDEACTIVATE));

	return TRUE;
}

LRESULT CMainTab::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    return 0;
}

LRESULT CMainTab::OnActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if(!DevManager::specific()->ActivateDevice())
        MessageBox(_T("Driver activation error!"), _T("Error"), MB_OK);

	return 0;
}

LRESULT CMainTab::OnDeactivate(WORD /* wNotifyCode */, WORD /* wID */, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if(!DevManager::specific()->DeactivateDevice())
        MessageBox(_T("Driver deactivation error!"), _T("Error"), MB_OK);

	return 0;
}

LRESULT CMainTab::OnCbnSelchangeDrvCombo(WORD /* wNotifyCode */, WORD /* wID */, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int cur = _dev_combo.GetCurSel();
    DevManager::specific()->SetActiveDevice(cur);
    return 0;
}

void CMainTab::onDevChange()
{
    _dev_combo.ResetContent();

    if(0 == DevManager::specific()->GetDevCount())
    {
        _dev_combo.EnableWindow(FALSE);
        _dev_dectiv.EnableWindow(FALSE);
        _dev_combo.SetWindowText(_T(""));
    }
    else
    {
        _dev_combo.EnableWindow(TRUE);
        _dev_dectiv.EnableWindow(TRUE);

        for(int i = 0; i < DevManager::specific()->GetDevCount(); ++i)
        {
            _dev_combo.InsertString(i,
                (std::tstring(_T("VirtualGPS - ")) + DevManager::specific()->GetDevInfo(i)->port).c_str());
        }

        _dev_combo.SetCurSel(DevManager::specific()->GetActiveDevice());
    }

    _dev_combo.UpdateWindow();
}
