#pragma once

#include <UI/DebugWidget.h>
#include <vector>
#include <string>

namespace DriverNG
{
    class DebugTools : public DebugWidget
    {
    private:
        bool m_bIsVisible { false };
        bool m_bPauseEnabled{ true };
        bool m_bPauseIsOn{ false };
        bool m_bIsTitleVisible{ true };
        char m_luaCommandStr[4096];

        std::vector<std::string> m_execHistory;
        int m_execHistoryIndex;

    public:
        void draw() override;
        void toggleVisibility();
        [[nodiscard]] bool isVisible() const;

    private:
        void onPreDraw();
        void onPostDraw();

        void drawTopMenu();
    };
}