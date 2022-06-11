#include "d3d9_common.h"
#include "d3d9_wrap.h"
#include "Logger.h"

unsigned int RefCounter9::SoftRef(WrappedD3DDevice9* device)
{
    unsigned int ret = AddRef();
    if (device)
        device->SoftRef();
    else
        MsgWarning("No device pointer, is a deleted resource being AddRef()d?\n");
    return ret;
}

unsigned int RefCounter9::SoftRelease(WrappedD3DDevice9* device)
{
    unsigned int ret = Release();
    if (device)
        device->SoftRelease();
    else
		MsgWarning("No device pointer, is a deleted resource being Release()d?\n");
    return ret;
}

void RefCounter9::AddDeviceSoftref(WrappedD3DDevice9* device)
{
    if (device)
        device->SoftRef();
}

void RefCounter9::ReleaseDeviceSoftref(WrappedD3DDevice9* device)
{
    if (device)
        device->SoftRelease();
}