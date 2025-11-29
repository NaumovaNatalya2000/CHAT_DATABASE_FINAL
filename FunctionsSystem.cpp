#include "FunctionsSystem.h"

void clearScreen()
{
	std::wcout << L"\033[2J\033[H" << std::flush;
}

void pause()
{
	std::wcout << L"Нажмите Enter для продолжения...";
	std::wcin.clear(); // Очищаем флаги ошибок
	std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
}

// Функция для получения текущей даты и времени как std::wstring
std::wstring getCurrentDateTime() {
	auto now = std::chrono::system_clock::now();
	auto time_t = std::chrono::system_clock::to_time_t(now);
	std::tm timeinfo = {};
	localtime_s(&timeinfo, &time_t);  // Безопасная версия для Windows
	std::wstringstream ss;
	ss << std::put_time(&timeinfo, L"%Y-%m-%d %H:%M:%S");
	return ss.str();
}