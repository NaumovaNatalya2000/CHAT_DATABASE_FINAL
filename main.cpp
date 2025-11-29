#include "IncludeLibrary.h"
#include <io.h>
#include <fcntl.h>
#include <iomanip>

int main() {
	int prev_mode_out = _setmode(_fileno(stdout), _O_U16TEXT);
	int prev_mode_in = _setmode(_fileno(stdin), _O_U16TEXT);
	int prev_mode_err = _setmode(_fileno(stderr), _O_U16TEXT);

	// Проверяем на ошибки (возвращает -1 при ошибке)
	if (prev_mode_out == -1 || prev_mode_in == -1 || prev_mode_err == -1) {
		std::wcerr << L"Ошибка настройки консоли для UTF-16" << std::endl;
		return -1;
	}
	//Коннект к БД
	DBStruct DB;
	if (DB.ALL_CONNECT_DB() == -1) {
		return -1;
	}
	std::wcout << L"Подключение к БД прошло успешно!" << std::endl;
	//создаем таблицы
	DB.CreateTables();

	pause();

	//Открываем базовое меню
	MainMenu(DB);
	
	return 0;
}