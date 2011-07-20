// SignalTab.cpp : implementation of the CSignalTab class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "SignalTab.h"

#include "DevManager.h"

#include <nmea/tools.h>

void CSignalTab::onDevChange()
{
    EnableWindow(DevManager::specific()->GetActiveDevice() >= 0);
    ::EnableWindow(GetDlgItem(IDC_BUTTON1), DevManager::specific()->GetActiveDevice() >= 0);
    ::EnableWindow(GetDlgItem(IDC_BUTTON2), DevManager::specific()->GetActiveDevice() >= 0);
}

LRESULT CSignalTab::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CComboBox sig_box(GetDlgItem(IDC_COMBO1));
    sig_box.AddString(_T("0 - Invalid"));
    sig_box.AddString(_T("1 - Fix"));
    sig_box.AddString(_T("2 - Differential"));
    sig_box.AddString(_T("3 - Sensitive"));

    CComboBox fix_box(GetDlgItem(IDC_COMBO2));
    fix_box.AddString(_T("1 - Not available"));
    fix_box.AddString(_T("2 - 2D"));
    fix_box.AddString(_T("3 - 3D"));

	return TRUE;
}

LRESULT CSignalTab::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    return 0;
}

LRESULT CSignalTab::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CComboBox combo_box(GetDlgItem(IDC_COMBO1));

    HANDLE hdl;
    std::tstring err;
    int val = combo_box.GetCurSel();

    if(val < 0)
        return 0;

    for(;;)
    {
        if(NULL == (hdl = DevManager::specific()->ConnectDevice()))
        {
            err = _T("Can`t open device port!");
            break;
        }
        if(FALSE == DeviceIoControl(hdl, VGD_IOCTL_SETSIG, &val, sizeof(int), 0, 0, NULL, NULL))
        {
            err = _T("Can`t set parameter!");
            break;
        }
        break;
    }

    if(hdl)
        CloseHandle(hdl);
    if(!err.empty())
        MessageBox(err.c_str(), _T("Error"), MB_OK);
    else
    {
        MessageBox(_T("Parameter is set."), _T("Information"), MB_OK);
        combo_box.SetCurSel(-1);
    }

    return 0;
}

LRESULT CSignalTab::OnBnClickedButton2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CComboBox combo_box(GetDlgItem(IDC_COMBO2));

    HANDLE hdl;
    std::tstring err;
    int val = combo_box.GetCurSel() + 1;

    if(val <= 0)
        return 0;

    for(;;)
    {

        if(NULL == (hdl = DevManager::specific()->ConnectDevice()))
        {
            err = _T("Can`t open device port!");
            break;
        }
        if(FALSE == DeviceIoControl(hdl, VGD_IOCTL_SETFIX, &val, sizeof(int), 0, 0, NULL, NULL))
        {
            err = _T("Can`t set parameter!");
            break;
        }
        break;
    }

    if(hdl)
        CloseHandle(hdl);
    if(!err.empty())
        MessageBox(err.c_str(), _T("Error"), MB_OK);
    else
    {
        MessageBox(_T("Parameter is set."), _T("Information"), MB_OK);
        combo_box.SetCurSel(-1);
    }

    return 0;
}

LRESULT CSignalTab::OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if(TRUE == (BOOL)wParam)
        _checkValues();
    return 0;
}

void CSignalTab::_checkValues()
{
    HANDLE hdl;
    nmeaINFO info;
    CComboBox sig_box(GetDlgItem(IDC_COMBO1));
    CComboBox fix_box(GetDlgItem(IDC_COMBO2));

    sig_box.SetCurSel(-1);
    fix_box.SetCurSel(-1);

    if(NULL != (hdl = DevManager::specific()->ConnectDevice()) &&
        TRUE == DeviceIoControl(hdl, VGD_IOCTL_GETINFO, 0, 0, &info, sizeof(nmeaINFO), NULL, NULL))
    {
        if(info.sig >= 0 && info.sig < 4)
            sig_box.SetCurSel(info.sig);
        if(info.fix >= 1 && info.fix <= 3)
            fix_box.SetCurSel(info.fix - 1);
    }
}
