#include <UI/DebugTools.h>
#include <UI/Widget.h>

#include <cmdlib.h>
#include <imgui.h>
#include <string>
#include <Delegates/ILuaDelegate.h>

namespace DriverNG
{
    namespace Internals
    {
        static ILuaDelegate& g_luaDelegate = ILuaDelegate::GetInstance();
    }

    DebugTools::DebugTools()
    {

    }

    WidgetID DebugTools::ToolbarWidget()
    {
        WidgetID activeID = WidgetID::COUNT;
        ImGui::SameLine();
        if (ImGui::Button("Console"))
            activeID = WidgetID::CONSOLE;
        //ImGui::SameLine();
        //if (ImGui::Button("Hotkeys"))
        //    activeID = WidgetID::HOTKEYS;
        ImGui::SameLine();
        if (ImGui::Button("Settings"))
            activeID = WidgetID::SETTINGS;
        ImGui::Spacing();
        return activeID;
    }

    void DebugTools::Update()
    {
        /*
        
    	
    	if(m_bIsTitleVisible)
    	{
            ImGui::Begin("HelpFrame", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiInputTextFlags_NoHorizontalScroll |
                ImGuiWindowFlags_NoSavedSettings | ImGuiColorEditFlags_NoInputs);
    		
	            ImGui::SetWindowPos(ImVec2(0, 0));
	            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.25f, 1.0f), "DriverNG Hook / Tools by SoapyMan");
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Alpha 0.3a");

    			if(callLuaFunction)
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Press F3 to toggle menu");
                else
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "--- Please wait ---");
            ImGui::End();
    	}
    	*/

        Internals::g_luaDelegate.DoRenderUpdate();

        auto callLuaFunction = Internals::g_luaDelegate.GetCallLuaFunction();
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

        if (ImGui::Begin("Driver NG Tools"))
        {
            const ImVec2 zeroVec = { 0, 0 };
            const DriverNG::WidgetID selectedID = ToolbarWidget();

            //if (!m_options.IsFirstLaunch)
            {
                if (selectedID < WidgetID::COUNT)
                {
                    if (m_activeWidgetID != selectedID)
                    {
                        // [A] temporary meausure
                        if (selectedID == WidgetID::CONSOLE)
                            m_console.OnEnable();
                        else
                            m_console.OnDisable();

                        m_activeWidgetID = selectedID;
                    }

                    
                }

                if (m_activeWidgetID == WidgetID::CONSOLE)
                {
                    if (ImGui::BeginChild("Console", zeroVec, true))
                        m_console.Update();

                    ImGui::EndChild();
                }
                /*
                if (m_activeWidgetID == WidgetID::HOTKEYS)
                {
                    if (ImGui::BeginChild("Keybinds", zeroVec, true))
                        m_hotkeys.Update();
                    ImGui::EndChild();
                }
                */
            }
            if (m_activeWidgetID == WidgetID::SETTINGS)
            {
                if (ImGui::BeginChild("Settings", zeroVec, true))
                {
                    //m_settings.Update();
                    ImGui::Checkbox("Auto pause game", &m_bPauseEnabled);
                }

                ImGui::EndChild();
            }
        }
        ImGui::End();
    }

    void DebugTools::OnEnable() 
    {
    }

    void DebugTools::OnDisable() 
    {
    }

    void DebugTools::ToggleVisibility()
    {
        if (!m_bIsVisible && !Internals::g_luaDelegate.IsDeveloperConsoleAllowed())
        {
            return;
        }

        m_bIsTitleVisible = false;
        m_bIsVisible = !m_bIsVisible;

        if (m_bIsVisible)
        {
            if (m_activeWidgetID == WidgetID::CONSOLE)
                m_console.OnEnable();
        }
        else
        {
            if (m_activeWidgetID == WidgetID::CONSOLE)
                m_console.OnDisable();
        }

    }

    bool DebugTools::IsVisible() const
    {
        return m_bIsVisible;
    }

    void DebugTools::LogGameToConsole(const std::string& acpText)
    {
        if(m_console.GameLogEnabled())
            m_console.Log(acpText);
    }
}