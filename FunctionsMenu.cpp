#include "FunctionsMenu.h"

void MainMenu(DBStruct& DB) {

	wchar_t BasicMenu{}; //переменная для перехода по базовому меню

	while (BasicMenu != L'q' && BasicMenu != L'Q') {
		clearScreen(); //чистка экрана при каждой итерации цикла
		std::wcout
			<< L"------------Основное меню программы Чатик------------\n"
			<< L"-----------------------------------------------------\n"
			<< L"Выберите пункт меню:\n"
			<< L"1 - Регистрация пользователя\n"
			<< L"2 - Вход в чат\n"
			<< L"-----------------------------------------------------\n"
			<< L"Введите q для выхода из программы(все данные будут удалены!)"
			<< std::endl;

		BasicMenu = NotEmptyChar(); //считывание первого символа из потока
		switch (BasicMenu){
			case '1': RegistrationMenu(DB); break; //переход в меню регистрации

			case '2': MessageMenu(DB); break; //переход в меню сообщений
			default: break;
		}
	}
}

void RegistrationMenu(DBStruct& DB)
{
	wchar_t RegMenu{L'a'}; //переменная для перехода по меню регистрации
	std::wstring input; //переменная для считывания потока ввода

	do {
		RegMenu = L'a';
		clearScreen(); //чистка экрана при каждой итерации цикла
		std::wstring login, password, name, surname, email;
		
		int check_login = 0;
		bool login_valid = false;
		while (!login_valid) {
			login = NotEmptyString(L"Введите логин:");
			check_login = DB.DB_Check_Login(login);

			switch (check_login) {
			case -1: //произошла ошибка при проверке логина в БД
				std::wcout << L"Ошибка проверки логина в базе данных." << std::endl;
				break;
			case 1: //логин не уникален
				std::wcout << L"Введенный логин уже существует! Повторите ввод." << std::endl; break;
			case 0: //логин уникален
				login_valid = true;
				std::wcout << L"Логин доступен!" << std::endl;
				break;
			}
			pause();
		}
		
		name = NotEmptyString(L"Введите имя:");
		surname = NotEmptyString(L"Введите фамилию:");
		email = NotEmptyString(L"Введите почту:");
		password = NotEmptyString(L"Введите пароль:");
		std::vector<uint8_t> vecPass(password.begin(), password.end());

		if (DB.RegisterUserWithPassword(login, name, surname, email, SHA1(vecPass))) {
			std::wcout << L"Регистрация завершена успешно!" << std::endl;
		}
		else {
			std::wcout << L"Ошибка при регистрации!" << std::endl;
		}

		std::wcout
			<< L"Вы хотите создать еще одного пользователя?(Введите y)\n"
			<< L"-----------------------------------------------------\n"
			<< L"Если Вы хотите вернуться в главное меню, введите любой другой символ"
			<< std::endl;
		std::getline(std::wcin, input);
		if (!input.empty() && input[0]!=L'\n') {
			RegMenu = input[0]; //считывание первого символа из потока
		}
	} while (RegMenu == L'y' || RegMenu == L'Y');
}

void MessageMenu(DBStruct& DB)
{
	wchar_t MesMenu{}; //переменная для перехода по меню сообщений

	do {
		clearScreen();
		std::wstring login, password; //переменные логин, пароль

		std::wcout 
			<<L"------------Отправка сообщений------------\n"
			<<L"Выберите пункт меню:\n"
			<<L"1 - Вывести список доступных логинов\n"
			<<L"2 - Вход по логину\n"
			<<L"q - для возврата на основное меню" << std::endl;

		MesMenu = NotEmptyChar(); //считывание первого символа из потока

		if (MesMenu == '1') {
			clearScreen();
			DB.SelectLogin();
			pause();
		}
		if (MesMenu == '2') {
			int check_login = 0;
			bool login_valid = false;
			std::wcout << L"------------Вход в систему------------\n" << std::endl;
			login = NotEmptyString(L"Введите логин:");
			//поиск логина
			check_login = DB.DB_Check_Login(login);
			switch (check_login) {
			case -1: //произошла ошибка при проверке логина в БД
				std::wcout << L"Ошибка проверки логина в базе данных." << std::endl;
				break;
			case 1: //логин существует
				login_valid = true;
				std::wcout << L"Введенный логин существует" << std::endl; break;
				break;
			case 0: //логина не существует
				std::wcout << L"Логина не существует!" << std::endl;
				break;
			}
			pause();
			if (login_valid) {
				std::wcout << L"Введите пароль:" << std::endl;
				std::getline(std::wcin, password);
				std::vector<uint8_t> vecPass(password.begin(), password.end());
				if (DB.IsCorrectPassword(SHA1(vecPass), login)){
					clearScreen();
					std::wcout << L"Пароль введен верно" << std::endl;
					pause();
					SendingMenu(DB, login);
				}
				else {
					std::wcout << L"Пароль введен неверно" << std::endl;
					pause();
				}
			}
		}
	} while (MesMenu != 'q' && MesMenu != 'Q');
}

void SendingMenu(DBStruct& DB, const std::wstring& sender)
{
	wchar_t SendMenu{L'a'}; //переменная для перехода по подменю сообщений
	do {
		clearScreen();
		std::wcout << L"Вы вошли в систему под логином: " << sender << L"\n"
			<< L"-----------------------------------------------------\n"
			<< L"1 - Вывести список пользователей\n"
			<< L"2 - Отправка сообщений\n"
			<< L"3 - Вывести список сообщений для меня\n"
			<< L"4 - Вывести список отправленных мной сообщений\n"
			<< L"q - для возврата на предыдущее меню " << std::endl;
		SendMenu = NotEmptyChar();
		if (SendMenu == '1') {
			clearScreen();
			DB.SelectLogin();
			pause();
		}
		if (SendMenu == '2') {
			clearScreen();
			std::wstring receiver, text_message; //переменные получатель и текст сообщения
			int check_login = 0;
			bool login_valid = false;
			bool isMessageSend = false;
			std::wcout << L"---Отправка сообщения---" << std::endl;
			receiver = NotEmptyString(L"Введите логин, кому хотите отправить сообщение:");
			check_login = DB.DB_Check_Login(receiver);
			switch (check_login) {
			case -1: //произошла ошибка при проверке логина в БД
				std::wcout << L"Ошибка проверки логина в базе данных." << std::endl;
				break;
			case 1: //логин существует
				login_valid = true;
				std::wcout << L"Введенный логин существует" << std::endl; break;
				break;
			case 0: //логина не существует
				std::wcout << L"Логина не существует!" << std::endl;
				break;
			}
			pause();
			if (login_valid) {
				text_message = NotEmptyString(L"Введите текст сообщения:");
				isMessageSend = DB.RegisterMessagesAll(sender, receiver, text_message);
				if (isMessageSend) {
					std::wcout << L"Сообщение отправлено успешно" << std::endl;
				}
				else {
					std::wcout << L"Сообщение не было отправлено" << std::endl;
				}
				pause();
			}
			
		}
		if (SendMenu == '3') {
			clearScreen();
			DB.SelectMessageReceived(sender);
			pause();
		}
		if (SendMenu == '4') {
			clearScreen();
			DB.SelectMessageSender(sender);
			pause();
		}

	} while (SendMenu != L'q' && SendMenu != L'Q');
}


std::wstring NotEmptyString(const std::wstring& prompt)
{
	std::wstring input;
	do {
		std::wcout << prompt << std::endl;
		std::getline(std::wcin, input);
	} while (input.empty());
	return input;
}

wchar_t NotEmptyChar()
{
	std::wstring input;
	std::getline(std::wcin, input); //ввод данных до первого enter
	if (!input.empty()) {
		return input[0]; //считывание первого символа из потока
	}
	else {
		return L'a';
	}
}