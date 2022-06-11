#pragma once

#include <memory>
#include <vector>

namespace DriverNG
{
    enum class WidgetID
    {
        CONSOLE,
        HOTKEYS,
        SETTINGS,
        COUNT
    };

    class Widget
    {
    public:
        virtual ~Widget() = default;

        virtual void OnEnable() = 0;
        virtual void OnDisable() = 0;
        virtual void Update() = 0;
    };
}