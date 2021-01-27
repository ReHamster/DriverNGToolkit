
#include "dinput_wrap.h"

#include <Delegates/ILuaDelegate.h>


#include "spdlog/spdlog.h"

namespace Globals {
	DriverNG::ILuaDelegate& g_luaDelegate = DriverNG::ILuaDelegate::GetInstance();
}


HRESULT _stdcall WrapDirectInputDevice::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
	HRESULT hr = m_device->GetDeviceState(cbData, lpvData);

	if (hr != DI_OK) 
		return hr;    //Error getting device state, so return

	if (m_devguid == GUID_SysMouse)
	{

	}
	else
	{

	}


	return DI_OK;
}

HRESULT _stdcall WrapDirectInputDevice::GetDeviceData(DWORD cbObjectData, DIDEVICEOBJECTDATA* rgdod, DWORD* pdwInOut, DWORD dwFlags)
{
	HRESULT hr = m_device->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
	
	if (rgdod == NULL || hr != DI_OK)
		return hr;

	Globals::g_luaDelegate.DoCommands();
	
	if(!m_enabled)
	{
		if (m_devguid == GUID_SysKeyboard)
		{
			rgdod->dwOfs = 0;
			rgdod->dwData = 0;
		}
		else if (m_devguid == GUID_SysMouse)
		{
			rgdod->dwData = 0;
		}
	}

	return hr;
}