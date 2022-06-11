#pragma once

#include "Delegates/IDirect3DDelegate.h"

namespace DriverNG
{
    class DX9Delegate final : public IDirect3DDelegate
    {
    public:
        void OnInitialised(IDirect3DDevice9* device, HWND focusWindow) override;
        void OnBeginScene(IDirect3DDevice9* device) override;
        void OnEndScene(IDirect3DDevice9* device) override;
        void OnDeviceLost() override;
        void OnReset(IDirect3DDevice9* device) override;
        void OnDeviceRestored(IDirect3DDevice9* device) override;
    };
}