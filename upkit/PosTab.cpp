// PosTab.cpp : implementation of the CPosTab class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PosTab.h"

#include "DevManager.h"

#include <nmea/tools.h>

LRESULT CPosTab::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return TRUE;
}

LRESULT CPosTab::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    return 0;
}

LRESULT CPosTab::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    std::tstring lat(100, _T('')); ::GetWindowText(GetDlgItem(IDC_EDIT1), &lat[0], 100);
    std::tstring lon(100, _T('')); ::GetWindowText(GetDlgItem(IDC_EDIT2), &lon[0], 100);

    if( !std::char_traits<TCHAR>::length(lat.c_str()) ||
        !std::char_traits<TCHAR>::length(lon.c_str()))
    {
        MessageBox(_T("Set position correctly please!"), _T("Warning"), MB_OK);
        return 0;
    }

    double dlat, dlon;

    { std::tsstring ss; ss << lat; ss >> dlat; }
    { std::tsstring ss; ss << lon; ss >> dlon; }

    HANDLE hdl;
    std::tstring err;

    for(;;)
    {

        if(NULL == (hdl = DevManager::specific()->ConnectDevice()))
        {
            err = _T("Can`t open device port!");
            break;
        }
        if(FALSE == DeviceIoControl(hdl, VGD_IOCTL_SETLAT, &dlat, sizeof(double), 0, 0, NULL, NULL))
        {
            err = _T("Can`t set parameter (lat)!");
            break;
        }
        if(FALSE == DeviceIoControl(hdl, VGD_IOCTL_SETLON, &dlon, sizeof(double), 0, 0, NULL, NULL))
        {
            err = _T("Can`t set parameter (lon)!");
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
        MessageBox(_T("Position is set."), _T("Information"), MB_OK);
        ::SetWindowText(GetDlgItem(IDC_EDIT1), _T(""));
        ::SetWindowText(GetDlgItem(IDC_EDIT2), _T(""));
    }

    return 0;
}

void CPosTab::onDevChange()
{
    EnableWindow(DevManager::specific()->GetActiveDevice() >= 0);
    ::EnableWindow(GetDlgItem(IDC_BUTTON1), DevManager::specific()->GetActiveDevice() >= 0);
}
