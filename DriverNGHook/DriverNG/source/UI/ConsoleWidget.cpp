#include <UI/ConsoleWidget.h>


#include <imgui.h>
#include <spdlog/spdlog.h>
#include <Delegates/ILuaDelegate.h>

namespace DriverNG
{
	namespace Internals
	{
		static ILuaDelegate& g_luaDelegate = ILuaDelegate::GetInstance();
	}

	//------------------------------------------------------------------
	Console::Console()
	{

	}

	void Console::OnEnable()
	{
		m_focusConsoleInput = true;
		m_execHistoryIndex = -1;
	}

	void Console::OnDisable()
	{
	}

	void Console::Update()
	{
		ImGui::Checkbox("Clear Input", &m_inputClear);
		ImGui::SameLine();
		if (ImGui::Button("Clear Output"))
		{
			std::lock_guard<std::recursive_mutex> _{ m_outputLock };
			m_outputLines.clear();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Scroll Output", &m_outputShouldScroll);
		ImGui::SameLine();
		ImGui::Checkbox("Disable Game Log", &m_disabledGameLog);
		ImGui::SameLine();

		if (ImGui::Button("Reload All Mods"))
		{
			auto callLuaFunction = Internals::g_luaDelegate.GetCallLuaFunction();

			Internals::g_luaDelegate.Push([=]() {
				callLuaFunction("checkReloadMods", ">");
				return 0;
			});
		}

		auto& style = ImGui::GetStyle();
		auto inputLineHeight = ImGui::GetTextLineHeight() + style.ItemInnerSpacing.y * 2;

		if (ImGui::ListBoxHeader("##ConsoleHeader", ImVec2(-1, -(inputLineHeight + style.ItemSpacing.y))))
		{
			std::lock_guard<std::recursive_mutex> _{ m_outputLock };

			ImGuiListClipper clipper;
			clipper.Begin(m_outputLines.size());
			while (clipper.Step())
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
				{
					auto& item = m_outputLines[i];
					ImGui::PushID(i);
					if (ImGui::Selectable(item.c_str()))
					{
						auto str = item;
						if (item[0] == '>' && item[1] == ' ')
							str = str.substr(2);

						std::strncpy(m_Command, str.c_str(), sizeof(m_Command) - 1);
						m_focusConsoleInput = true;
					}
					ImGui::PopID();
				}

			if (m_outputScroll)
			{
				if (m_outputShouldScroll)
					ImGui::SetScrollHereY();
				m_outputScroll = false;
			}

			ImGui::ListBoxFooter();
		}

		bool textChanged = false;

		// command history
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

				//strcpy_s(m_Command, m_execHistory[newIndex].c_str());
				//textChanged = true;
				std::strncpy(m_Command, m_execHistory[newIndex].c_str(), sizeof(m_Command) - 1);
				m_focusConsoleInput = true;
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

				std::strncpy(m_Command, m_execHistory[newIndex].c_str(), sizeof(m_Command) - 1);
				m_focusConsoleInput = true;
				textChanged = true;
			}
		}

		if (m_focusConsoleInput)
		{
			ImGui::SetKeyboardFocusHere();
			m_focusConsoleInput = false;
		}

		ImGui::PushItemWidth(-1);
		const auto execute = ImGui::InputText("##InputCommand", m_Command, std::size(m_Command), (textChanged ? ImGuiInputTextFlags_ReadOnly : 0) | ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::PopItemWidth();
		ImGui::SetItemDefaultFocus();

		if (execute)
		{
			auto callLuaFunction = Internals::g_luaDelegate.GetCallLuaFunction();

			std::string commandStr = m_Command;

			if (m_disabledGameLog)
			{
				Log("> " + commandStr);
			}

			Internals::g_luaDelegate.Push([=]() {
				
				callLuaFunction("driverNGHook_EvalHelper", "s", commandStr.c_str());
				return 0;
			});

			m_execHistory.push_back(m_Command);
			m_execHistoryIndex = -1;

			if (m_inputClear)
			{
				std::memset(m_Command, 0, sizeof(m_Command));
			}

			m_focusConsoleInput = true;
		}
	}

	void Console::Log(const std::string& acpText)
	{
		std::lock_guard<std::recursive_mutex> _{ m_outputLock };

		size_t first = 0;
		while (first < acpText.size())
		{
			const auto second = acpText.find_first_of('\n', first);

			if (second == std::string_view::npos)
			{
				m_outputLines.emplace_back(acpText.substr(first));
				break;
			}

			if (first != second)
				m_outputLines.emplace_back(acpText.substr(first, second - first));

			first = second + 1;
		}

		m_outputScroll = true;
	}

	bool Console::GameLogEnabled() const
	{
		return !m_disabledGameLog;
	}
}