#include "stdafx.h"
#include "DevManager.h"

#include <devload.h>

DevManager::DevManager()
: _active(-7)
{
}

DevManager * DevManager::specific()
{
    static DevManager self;
    return &self;
}

std::tstring DevManager::GetDevRegPath()
{
    return std::tstring(DEVLOAD_BUILT_IN_KEY) + _T("\\") + DEV_NAME;
}

void DevManager::RefreshInfo()
{
    TCHAR subReg[200];
    DWORD szSubReg = 200;
    HKEY subKey;

    TCHAR devReg[200];
    DWORD tpDevReg, szDevReg = sizeof(HANDLE);
    std::tstring devRegTemplate(GetDevRegPath());
    std::tstring devRegTmp;

    HKEY activeKey;
    DWORD index = 0;

    _dev_array.clear();

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, DEVLOAD_ACTIVE_KEY, 0, 0, &activeKey))
        return;

    while(TRUE)
    {
        szDevReg = szSubReg = 200;

        if(ERROR_SUCCESS != RegEnumKeyEx(activeKey, index, &subReg[0], &szSubReg, NULL, NULL, NULL, NULL))
            break;

        devRegTmp = DEVLOAD_ACTIVE_KEY;
        devRegTmp += _T("\\");
        devRegTmp += &subReg[0];

        if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, &devRegTmp[0], 0, 0, &subKey))
            break;
        if(ERROR_SUCCESS != RegQueryValueEx(subKey, DEVLOAD_DEVKEY_VALNAME, NULL, &tpDevReg, (LPBYTE)&devReg[0], &szDevReg))
            break;

        devRegTmp = &devReg[0];

        if(devRegTemplate == devRegTmp)
        {
            DWORD val_type, val_size;
            DevInfo info;

            info.reg_path = &devRegTmp[0];
            info.port.resize(1024);

            val_type = DEVLOAD_HANDLE_VALTYPE; val_size = sizeof(HANDLE);
            RegQueryValueEx(subKey, DEVLOAD_HANDLE_VALNAME, NULL, &val_type, (LPBYTE)&info.handle, &val_size);
            val_type = DEVLOAD_DEVNAME_VALTYPE; val_size = 1023;
            RegQueryValueEx(subKey, DEVLOAD_DEVNAME_VALNAME, NULL, &val_type, (LPBYTE)&info.port[0], &val_size);

            _dev_array.push_back(info);
        }

        RegCloseKey(subKey);

        ++index;
    }

    RegCloseKey(activeKey);

    if(!_dev_array.empty())
        _active = _dev_array.size() - 1;
    else
        _active = -1;

    _handlerNotify();
}

bool DevManager::ActivateDevice()
{
    HANDLE hdl = ::ActivateDevice(GetDevRegPath().c_str(), 100);

    RefreshInfo();

    return (NULL != hdl);
}

bool DevManager::DeactivateDevice()
{
    bool resv = false;

    if(_active >= 0)
        resv = (TRUE == ::DeactivateDevice(_dev_array[_active].handle));

    RefreshInfo();

    return resv;
}

HANDLE DevManager::ConnectDevice()
{
    HANDLE handle = NULL;

    if(_active < 0)
        return NULL;

    handle = ::CreateFile(
        _dev_array[_active].port.c_str(),
        VGD_AC_CONFIGURATOR, 0, NULL, OPEN_EXISTING, 0, 0
        );

    if(INVALID_HANDLE_VALUE == handle)
        handle = NULL;

    return handle;
}

void DevManager::RegHandler(DevHandler *handler)
{
    handler_array_t::const_iterator it =
        std::find(_handler_array.begin(), _handler_array.end(), handler);

    if(it == _handler_array.end())
        _handler_array.push_back(handler);
}

void DevManager::UnregHandler(DevHandler *handler)
{
    handler_array_t::iterator it =
        std::find(_handler_array.begin(), _handler_array.end(), handler);
    if(it != _handler_array.end())
        _handler_array.erase(it);
}

void DevManager::_handlerNotify()
{
    handler_array_t::iterator it =
        _handler_array.begin();
    for(; it != _handler_array.end(); ++it)
        (*it)->onDevChange();
}

void DevManager::SetActiveDevice(int index)
{
    int old_a = _active;

    _active = index;

    if(old_a != _active)
        _handlerNotify();
}
