#pragma once

#include <UI/Widget.h>
#include <algorithm>
#include <mutex>

namespace DriverNG
{
	struct Console : Widget
	{
		Console();
		~Console() override = default;

		void OnEnable() override;
		void OnDisable() override;
		void Update() override;

		void Log(const std::string& acpText);
		bool GameLogEnabled() const;

	private:
		std::recursive_mutex m_outputLock{ };
		std::vector<std::string> m_outputLines{ };
		bool m_outputShouldScroll{ true };
		bool m_outputScroll{ false };
		bool m_inputClear{ true };
		bool m_disabledGameLog{ false };
		bool m_focusConsoleInput{ false };

		char m_Command[0x10000]{ 0 };

		std::vector<std::string> m_execHistory;
		int m_execHistoryIndex{ -1 };
	};

}