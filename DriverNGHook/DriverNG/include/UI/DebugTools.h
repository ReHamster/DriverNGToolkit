#pragma once

#include <UI/Widget.h>
#include <UI/ConsoleWidget.h>
#include <vector>
#include <string>

namespace DriverNG
{
    class DebugTools : public Widget
    {
    private:
        bool m_bIsVisible { false };
        bool m_bPauseEnabled{ true };
        bool m_bPauseIsOn{ false };
        bool m_bIsTitleVisible{ true };
        char m_luaCommandStr[4096];

    public:
        DebugTools();

        WidgetID ToolbarWidget();
        void Update() override;
        void OnEnable();
        void OnDisable();

        void ToggleVisibility();
        [[nodiscard]] bool IsVisible() const;

        void LogGameToConsole(const std::string& acpText);

    private:
        void DrawTopMenu();

        WidgetID m_activeWidgetID{ WidgetID::CONSOLE };
        Console m_console;
    };
}