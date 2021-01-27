#pragma once

#include <dinput.h>

class WrapDirectInputDevice : public IDirectInputDevice8A
{
public:
    WrapDirectInputDevice(IDirectInputDevice8A* device, REFGUID devguid) : m_device(device), m_devguid(devguid), m_enabled(true)
    {
    }

    // Hook functions

    void Enable(bool enable) { m_enabled = enable; }

    /*** IUnknown methods ***/
    HRESULT _stdcall QueryInterface(REFIID riid, void** ppvObj) {
        return m_device->QueryInterface(riid, ppvObj);
    }
    ULONG _stdcall AddRef() {
        return m_device->AddRef();
    }
    ULONG _stdcall Release() {
        return m_device->Release();
    }


    /*** IDirectInputDevice8W methods ***/
    HRESULT _stdcall GetCapabilities(LPDIDEVCAPS a) {
        return m_device->GetCapabilities(a);
    }
    HRESULT _stdcall EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA a, void* b, DWORD c) {
        return m_device->EnumObjects(a, b, c);
    }
    HRESULT _stdcall GetProperty(REFGUID a, DIPROPHEADER* b) {
        return m_device->GetProperty(a, b);
    }
    HRESULT _stdcall SetProperty(REFGUID a, const DIPROPHEADER* b) {
        return m_device->SetProperty(a, b);
    }
    HRESULT _stdcall Acquire() {
        return m_device->Acquire();
    }
    HRESULT _stdcall Unacquire() {
        return m_device->Unacquire();
    }

    HRESULT _stdcall GetDeviceState(DWORD a, void* b);
    HRESULT _stdcall GetDeviceData(DWORD a, DIDEVICEOBJECTDATA* b, DWORD* c, DWORD d);


    HRESULT _stdcall SetDataFormat(const DIDATAFORMAT* a) {
        return m_device->SetDataFormat(a);
    }
    HRESULT _stdcall SetEventNotification(HANDLE a) {
        return m_device->SetEventNotification(a);
    }
    HRESULT _stdcall SetCooperativeLevel(HWND a, DWORD b) {
        return m_device->SetCooperativeLevel(a, b);
    }
    HRESULT _stdcall GetObjectInfo(DIDEVICEOBJECTINSTANCEA* a, DWORD b, DWORD c) {
        return m_device->GetObjectInfo(a, b, c);
    }
    HRESULT _stdcall GetDeviceInfo(DIDEVICEINSTANCEA* a) {
        return m_device->GetDeviceInfo(a);
    }
    HRESULT _stdcall RunControlPanel(HWND a, DWORD b) {
        return m_device->RunControlPanel(a, b);
    }
    HRESULT _stdcall Initialize(HINSTANCE a, DWORD b, REFGUID c) {
        return m_device->Initialize(a, b, c);
    }
    HRESULT _stdcall CreateEffect(REFGUID a, const DIEFFECT* b, LPDIRECTINPUTEFFECT* c, IUnknown* d) {
        return m_device->CreateEffect(a, b, c, d);
    }
    HRESULT _stdcall EnumEffects(LPDIENUMEFFECTSCALLBACKA a, void* b, DWORD c) {
        return m_device->EnumEffects(a, b, c);
    }
    HRESULT _stdcall GetEffectInfo(DIEFFECTINFOA* a, REFGUID b) {
        return m_device->GetEffectInfo(a, b);
    }
    HRESULT _stdcall GetForceFeedbackState(DWORD* a) {
        return m_device->GetForceFeedbackState(a);
    }
    HRESULT _stdcall SendForceFeedbackCommand(DWORD a) {
        return m_device->SendForceFeedbackCommand(a);
    }
    HRESULT _stdcall EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK a, void* b, DWORD c) {
        return m_device->EnumCreatedEffectObjects(a, b, c);
    }
    HRESULT _stdcall Escape(DIEFFESCAPE* a) {
        return m_device->Escape(a);
    }
    HRESULT _stdcall Poll() {
        return m_device->Poll();
    }
    HRESULT _stdcall SendDeviceData(DWORD a, const DIDEVICEOBJECTDATA* b, DWORD* c, DWORD d) {
        return m_device->SendDeviceData(a, b, c, d);
    }
    HRESULT _stdcall EnumEffectsInFile(const char* a, LPDIENUMEFFECTSINFILECALLBACK b, void* c, DWORD d) {
        return m_device->EnumEffectsInFile(a, b, c, d);
    }
    HRESULT _stdcall WriteEffectToFile(const char* a, DWORD b, DIFILEEFFECT* c, DWORD d) {
        return m_device->WriteEffectToFile(a, b, c, d);
    }
    HRESULT _stdcall BuildActionMap(DIACTIONFORMATA* a, const char* b, DWORD c) {
        return m_device->BuildActionMap(a, b, c);
    }
    HRESULT _stdcall SetActionMap(DIACTIONFORMATA* a, const char* b, DWORD c) {
        return m_device->SetActionMap(a, b, c);
    }
    HRESULT _stdcall GetImageInfo(DIDEVICEIMAGEINFOHEADERA* a) {
        return m_device->GetImageInfo(a);
    }

private:
    LPDIRECTINPUTDEVICE8A m_device;
    REFGUID m_devguid;
    bool m_enabled;
};
