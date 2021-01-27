#include <UI/DebugTools.h>

#include <spdlog/spdlog.h>
#include <imgui.h>
#include <string>

// Widgets
#include <Delegates/ILuaDelegate.h>

namespace DriverNG
{
    namespace Internals
    {
        static ILuaDelegate& g_luaDelegate = ILuaDelegate::GetInstance();
    	
        static void QuitGame()
        {
            auto callLuaFunction = Internals::g_luaDelegate.GetCallLuaFunction();

            Internals::g_luaDelegate.Push([=]() {
                callLuaFunction("driverNGHook_QuitGame", ">");
                return 0;
            });
        }
    }

    void DebugTools::draw()
    {
        auto callLuaFunction = Internals::g_luaDelegate.GetCallLuaFunction();
    	
    	if(m_bIsTitleVisible)
    	{
            ImGui::Begin("HelpFrame", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiInputTextFlags_NoHorizontalScroll |
                ImGuiWindowFlags_NoSavedSettings | ImGuiColorEditFlags_NoInputs);
    		
	            ImGui::SetWindowPos(ImVec2(0, 0));
	            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.25f, 1.0f), "DriverNG Hook / Tools by SoapyMan");
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Alpha 0.2c");

    			if(callLuaFunction)
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Press F3 to toggle menu");
                else
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "--- Please wait ---");
            ImGui::End();
    	}
    	
        if (!callLuaFunction)
            return;
    	
        // pause the game
        int enablePause = m_bPauseEnabled && m_bIsVisible;

        if(m_bPauseIsOn != (enablePause > 0))
        {
            Internals::g_luaDelegate.Push([=]() {
                callLuaFunction("driverNGHook_SwitchPause", "b>", enablePause);
                return 0;
            });
        	
            m_bPauseIsOn = enablePause;
        }
    	
        if (!m_bIsVisible)
            return;

        // TODO: On pre draw
        onPreDraw();
        DebugWidget::draw();
        onPostDraw();
    }

    void DebugTools::toggleVisibility()
    {
        m_execHistoryIndex = -1;
        m_bIsTitleVisible = false;
        m_bIsVisible = !m_bIsVisible;
    }

    bool DebugTools::isVisible() const
    {
        return m_bIsVisible;
    }

    void DebugTools::onPreDraw()
    {
        drawTopMenu();
    }

    void DebugTools::onPostDraw()
    {}

    void DebugTools::drawTopMenu()
    {
        static bool showDebugOutput = false;
        static bool showConsoleInput = true; // enable it by default

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Main"))
            {
                ImGui::MenuItem("Auto pause game", nullptr, &m_bPauseEnabled);
                ImGui::MenuItem("Debug output", nullptr, &showDebugOutput);
                ImGui::Separator();
                if (ImGui::MenuItem("Close game"))
                {
                    Internals::QuitGame();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Lua"))
            {
                if(ImGui::MenuItem("Console input", nullptr, &showConsoleInput))
                {
                    memset(m_luaCommandStr, 0, sizeof(m_luaCommandStr));
                }

                ImGui::Separator();
            	
                if(ImGui::MenuItem("Reload mods", nullptr))
                {
                    auto callLuaFunction = Internals::g_luaDelegate.GetCallLuaFunction();

                    Internals::g_luaDelegate.Push([=]() {
                        callLuaFunction("checkReloadMods", ">");
                        return 0;
                    });
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (showConsoleInput)
        {
            bool textChanged = false;

            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow), true))
            {
                if (m_execHistoryIndex == -1)
                {
                    m_execHistoryIndex = m_execHistory.size();
                }

                if (m_execHistoryIndex != -1)
                {
                    int newIndex = --m_execHistoryIndex;

                    if (m_execHistoryIndex < 0)
                    {
	                    m_execHistoryIndex = 0;
                        newIndex = 0;
                    }

                    strcpy_s(m_luaCommandStr, m_execHistory[newIndex].c_str());
                    textChanged = true;
                }
            }
            else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow), true))
            {
                if (m_execHistoryIndex != -1)
                {
                    int newIndex = ++m_execHistoryIndex;

                    if (m_execHistoryIndex >= m_execHistory.size())
                    {
	                    m_execHistoryIndex = m_execHistory.size() - 1;
                        newIndex = m_execHistoryIndex;
                    }

                    strcpy_s(m_luaCommandStr, m_execHistory[newIndex].c_str());
                    textChanged = true;
                }
            }
            else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter), false))
            {
                if (strlen(m_luaCommandStr) > 0)
                {
                	
                    auto callLuaFunction = Internals::g_luaDelegate.GetCallLuaFunction();

                    std::string commandStr = m_luaCommandStr;
                	
                    Internals::g_luaDelegate.Push([=]() {
                        std::string commandStrCopy = commandStr;
                        callLuaFunction("driverNGHook_EvalHelper", "s", commandStr.c_str());
                        return 0;
                    });

                    m_execHistory.push_back(m_luaCommandStr);
                    m_execHistoryIndex = -1;

                    memset(m_luaCommandStr, 0, sizeof(m_luaCommandStr));
                    textChanged = true;
                }
            }

            ImGui::SetNextWindowSize(ImVec2(700, 95));
        	
            ImGui::Begin("Console input");

            if (ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
				ImGui::SetKeyboardFocusHere(0);
        	
            ImGui::InputText("Lua code (enter to execute)", m_luaCommandStr, sizeof(m_luaCommandStr), textChanged ? ImGuiInputTextFlags_ReadOnly : 0);        	
            ImGui::End();
        }
    	
        if (showDebugOutput)
        {
//            showGeomViewerWindow(&showGeomViewer);
        }
    }
}