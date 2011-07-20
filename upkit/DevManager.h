#pragma once

#include <ddkreg.h>

#define VGD_AC_CONFIGURATOR (0x0001L)

#define VGD_IOCTL_START     0xFF
#define VGD_IOCTL_SETLAT    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 1,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_SETLON    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 2,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_GENIDX    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 3,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_ADDGEN    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 4,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_SETSIG    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 5,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_SETFIX    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 6,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_GETINFO   CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 7,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_SETINFO   CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 8,METHOD_BUFFERED,FILE_ANY_ACCESS)

class DevHandler;

struct DevInfo
{
    std::tstring port;
    std::tstring reg_path;
    HANDLE handle;

    DevInfo()
        : handle(NULL)
    {}
};

class DevManager
{
public:
    static DevManager * specific();

    void RefreshInfo();

    int GetDevCount() const { return (int)_dev_array.size(); }
    const DevInfo * GetDevInfo(int index) const { return &(_dev_array[index]); }

    bool ActivateDevice();
    bool DeactivateDevice();
    HANDLE ConnectDevice();

    int GetActiveDevice() const { return _active; }
    void SetActiveDevice(int index);

    void RegHandler(DevHandler *handler);
    void UnregHandler(DevHandler *handler);

    std::tstring GetDevRegPath();

private:
    DevManager();

    void _handlerNotify();

    typedef std::vector<DevInfo> dev_array_t;
    typedef std::vector<DevHandler *> handler_array_t;
    dev_array_t _dev_array;
    handler_array_t _handler_array;
    int _active;

};

class DevHandler
{
    friend class DevManager;

public:
    virtual ~DevHandler()
    {
        DevManager::specific()->UnregHandler(this);
    }

protected:
    DevHandler()
    {
        DevManager::specific()->RegHandler(this);
    }

    virtual void onDevChange() {}

};
