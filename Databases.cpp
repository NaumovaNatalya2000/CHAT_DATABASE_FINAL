#include "Databases.h"

//конструктор создания
DBStruct::DBStruct() :sqlenv(SQL_NULL_HANDLE), sqldbc(SQL_NULL_HANDLE) {}
//деструктор
DBStruct::~DBStruct()
{
	if (this->sqldbc != SQL_NULL_HANDLE) {
		//отключение от БД
		SQLDisconnect(this->sqldbc);
		//освобождение дескриптора SQL_HANDLE_DBC
		SQLFreeHandle(SQL_HANDLE_DBC, this->sqldbc);
	}
	if (this->sqlenv != SQL_NULL_HANDLE) {
		//освобождение дескриптора SQL_HANDLE_ENV
		SQLFreeHandle(SQL_HANDLE_ENV, this->sqlenv);
	}
}

//функция обработки ошибок
std::wstring DBStruct::GetErrorFunc(SQLSMALLINT handleType, SQLHANDLE sqlHan) {
	if (sqlHan == SQL_NULL_HANDLE) {
		return L"Дескриптор не определен";
	}
	//переменная для текста ошибки
	SQLWCHAR msg[SQL_MAX_MESSAGE_LENGTH];
	SQLSMALLINT len = 0;
	SQLRETURN res = SQLGetDiagRecW(handleType, sqlHan, 1, NULL, NULL, msg, SQL_MAX_MESSAGE_LENGTH, &len);
	if (!SQL_SUCCEEDED(res)) {
		return L"Не удалось получить сообщение об ошибке!";
	}
	else {
		return std::wstring(msg);
	}
}

//функция SQLAllocHandle
SQLRETURN DBStruct::CreateAlloc(SQLSMALLINT handleType, SQLHANDLE sqlHanParent, SQLHANDLE& sqlHan)
{
	return SQLAllocHandle(handleType, sqlHanParent, &sqlHan);
}

//функция, объединяющая создание дескрипторов и подключение к БД - возвращаем -1 при ошибке, 0 - успех
int DBStruct::ALL_CONNECT_DB() {
	//выделение дескриптора окружения SQL_HANDLE_ENV
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_ENV, SQL_NULL_HANDLE, this->sqlenv))) {
		std::wcout << L"Произошла ошибка при выделении дескриптора окружения (SQL_HANDLE_ENV):\n" << GetErrorFunc(SQL_HANDLE_ENV, this->sqlenv) << std::endl;
		return -1;
	}
	//определение параметров дескриптора окружения - версия ODBC
	if (!SQL_SUCCEEDED(SQLSetEnvAttr(this->sqlenv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))) {
		std::wcout << L"Произошла ошибка при установке версии ODBC (SQLSetEnvAttr):\n" << GetErrorFunc(SQL_HANDLE_ENV, this->sqlenv) << std::endl;
		return -1;
	}
	//выделение дескриптора SQL_HANDLE_DBC
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_DBC, this->sqlenv, this->sqldbc))) {
		std::wcout << L"Произошла ошибка при подключении к источнику данных (SQL_HANDLE_DBC):\n" << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return -1;
	}
	//Коннект к БД
	if (!SQL_SUCCEEDED(SQLDriverConnectW(this->sqldbc, NULL, (SQLWCHAR*)L"DRIVER={SQLite3 ODBC Driver};DATABASE=chat.db;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))) {
		std::wcout << L"Произошла ошибка при подключении к БД (SQLDriverConnect):\n" << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return -1;
	}
	return 0;
}

//функция создания Таблиц
void DBStruct::CreateTables()
{
	//выделение дескриптора SQL_HANDLE_STMT
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return;
	}
	//создание таблицы Users
	if (!SQL_SUCCEEDED(SQLExecDirectW(sqlStmt, (SQLWCHAR*)SQLQueries::CREATE_TABLE_USERS, SQL_NTS))) {
		std::wcout << L"Произошла ошибка при создании таблицы Users:\n" << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
	}
	//создание таблицы Passwords
	if (!SQL_SUCCEEDED(SQLExecDirectW(sqlStmt, (SQLWCHAR*)SQLQueries::CREATE_TABLE_PASSWORDS, SQL_NTS))) {
		std::wcout << L"Произошла ошибка при создании таблицы Passwords:\n" << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
	}
	//создание таблицы Messages
	if (!SQL_SUCCEEDED(SQLExecDirectW(sqlStmt, (SQLWCHAR*)SQLQueries::CREATE_TABLE_MESSAGES, SQL_NTS))) {
		std::wcout << L"Произошла ошибка при создании таблицы Messages:\n" << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
	}
	//создание таблицы Message_participants
	if (!SQL_SUCCEEDED(SQLExecDirectW(sqlStmt, (SQLWCHAR*)SQLQueries::CREATE_TABLE_MESSAGE_PARTICIPANTS, SQL_NTS))) {
		std::wcout << L"Произошла ошибка при создании таблицы Message_participants:\n" << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
	}
	//освобождение дескриптора SQL_HANDLE_STMT
	SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
}

//функция Проверка уникальности логина
int DBStruct::DB_Check_Login(const std::wstring& login) {
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	//лямбда-функция - для удобного освобождения дескриптора и получения кода возврата
	auto cleanup = [&sqlStmt](int ret) {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
		return ret;
	};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return cleanup(-1);
	}
	//подготовка строки SQL для выполнения
	if (!SQL_SUCCEEDED(SQLPrepareW(sqlStmt, (SQLWCHAR*)SQLQueries::CHECK_LOGIN_EXISTS, SQL_NTS))) {
		std::wcout << L"Произошла ошибка SQLPrepareW: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(-1);
	}
	//bind параметра
	SQLLEN loginLen = SQL_NTS;
	if (!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, login.length(), 0, (SQLPOINTER)login.c_str(), 0, &loginLen))) {
		std::wcout << L"Произошла ошибка SQLBindParameter: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(-1);
	}
	//выполнение запроса
	if (!SQL_SUCCEEDED(SQLExecute(sqlStmt))) {
		std::wcout << L"Произошла ошибка SQLExecute: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(-1);
	}
	SQLSMALLINT exists_result = 0;
	SQLLEN ind_exists = 0;
	//привязка буфера данных приложения к столбцу в результирующем наборе
	if (!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 1, SQL_C_SSHORT, &exists_result, 0, &ind_exists))) {
		std::wcout << L"Произошла ошибка SQLBindCol: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(-1);
	}
	//получение данных для привязанных столбцов
	SQLRETURN fetch_res = SQLFetch(sqlStmt);
	if (SQL_SUCCEEDED(fetch_res) && fetch_res != SQL_NO_DATA) {
		return (exists_result == 1) ? 1 : 0;
	}
	return cleanup(0);
}

//функция вывода всех логинов из БД
void DBStruct::SelectLogin() {
	//размер буфера для логина
	constexpr int BUFFERSTR = 512;
	//выделение дескриптора SQL_HANDLE_STMT
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << this->GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return;
	}
	SQLRETURN res;
	//переменная для хранения логинов
	SQLWCHAR V_od_login[BUFFERSTR] = { 0 };
	SQLLEN ind_login = 0;
	//отправка инструкции для однократного выполнения
	res = SQLExecDirectW(sqlStmt, (SQLWCHAR*)SQLQueries::SELECT_LOGIN, SQL_NTS);
	//привязка буфера данных приложения к столбцу в результирующем наборе
	if (!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 1, SQL_C_WCHAR, V_od_login, BUFFERSTR, &ind_login))) {
		std::wcout << L"Произошла ошибка SQLBindCol: " << this->GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
	}
	while (true) {
		//получение данных для привязанного столбца
		res = SQLFetch(sqlStmt);
		if (res == SQL_NO_DATA) break;
		if (!SQL_SUCCEEDED(res)) break;
		//вывод логинов
		std::wcout << V_od_login << std::endl;
	}
	//освобождение дескриптора SQL_HANDLE_STMT
	SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
}

//функция добавления user-а
bool DBStruct::InsertUsers(const std::wstring& login, const std::wstring& name, const std::wstring& surname, const std::wstring& email)
{
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	auto cleanup = [&sqlStmt]() {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
	};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return false;
	}
	//подготовка строки SQL для выполнения
	if (!SQL_SUCCEEDED(SQLPrepareW(sqlStmt, (SQLWCHAR*)SQLQueries::INSERT_USER, SQL_NTS))) {
		std::wcout << L"Произошла ошибка SQLPrepareW: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//bind параметров
	SQLLEN loginLen = SQL_NTS;
	SQLLEN nameLen = SQL_NTS;
	SQLLEN surnameLen = SQL_NTS;
	SQLLEN emailLen = SQL_NTS;
	if (!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, login.length(), 0, (SQLPOINTER)login.c_str(), 0, &loginLen)) ||
		!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, name.length(), 0, (SQLPOINTER)name.c_str(), 0, &nameLen)) ||
		!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, surname.length(), 0, (SQLPOINTER)surname.c_str(), 0, &surnameLen)) ||
		!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, email.length(), 0, (SQLPOINTER)email.c_str(), 0, &emailLen))) {
		std::wcout << L"Ошибка биндинга параметров: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//выполнение запроса
	if (!SQL_SUCCEEDED(SQLExecute(sqlStmt))) {
		std::wcout << L"Ошибка выполнения запроса: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}

	cleanup();
	return true;
}

//функция для нахождения user_id по логину
int DBStruct::GetUserIdByLogin(const std::wstring& login)
{
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	//лямбда-функция - для удобного освобождения дескриптора и получения кода возврата
	auto cleanup = [&sqlStmt](int ret) {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
		return ret;
	};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Ошибка создания STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return cleanup(-1);
	}
	//подготовка строки SQL для выполнения
	if (!SQL_SUCCEEDED(SQLPrepareW(sqlStmt, (SQLWCHAR*)SQLQueries::GET_USER_ID, SQL_NTS))) {
		std::wcout << L"Ошибка подготовки запроса: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(-1);
	}
	//bind параметра
	SQLLEN loginLen = SQL_NTS;
	if (!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, login.length(), 0, (SQLPOINTER)login.c_str(), 0, &loginLen))) {
		std::wcout << L"Ошибка биндинга параметра: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(-1);
	}
	//выполнение запроса
	if (!SQL_SUCCEEDED(SQLExecute(sqlStmt))) {
		std::wcout << L"Ошибка выполнения запроса: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(-1);
	}
	//привязка буфера данных приложения к столбцу в результирующем наборе
	SQLINTEGER user_id = 0;
	SQLLEN ind_user_id = 0;
	if (!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 1, SQL_C_SLONG, &user_id, 0, &ind_user_id))) {
		std::wcout << L"Ошибка биндинга колонки: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(-1);
	}
	//получение данных для привязанных столбцов
	SQLRETURN fetch_res = SQLFetch(sqlStmt);
	if (fetch_res == SQL_NO_DATA) {
		std::wcout << L"Пользователь с логином '" << login << L"' не найден" << std::endl;
		return cleanup(-1);
	}
	else if (!SQL_SUCCEEDED(fetch_res)) {
		std::wcout << L"Ошибка получения данных: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(-1);
	}
	return cleanup(static_cast<int>(user_id));
}

//добавление пароля
bool DBStruct::InsertPassword(int user_id, const std::vector<uint8_t>& password_hash)
{
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	//лямбда-функция - для удобного освобождения дескриптора и получения кода возврата
	auto cleanup = [&sqlStmt](bool ret) {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
		return ret;
	};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Ошибка создания STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return cleanup(false);
	}
	//подготовка строки SQL для выполнения
	if (!SQL_SUCCEEDED(SQLPrepareW(sqlStmt, (SQLWCHAR*)SQLQueries::INSERT_PASSWORD, SQL_NTS))) {
		std::wcout << L"Ошибка подготовки запроса: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(false);
	}
	//bind параметров
	SQLLEN user_id_len = 0;
	SQLLEN password_len = password_hash.size();
	if (!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&user_id, 0, &user_id_len)) ||
		!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_VARBINARY, password_hash.size(), 0, (SQLPOINTER)password_hash.data(), password_hash.size(), &password_len))) {
		std::wcout << L"Ошибка биндинга параметров: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(false);
	}
	//выполнение запроса
	if (!SQL_SUCCEEDED(SQLExecute(sqlStmt))) {
		std::wcout << L"Ошибка выполнения запроса: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		return cleanup(false);
	}
	return cleanup(true);
}

//функция объединение добавления user-а и пароля
bool DBStruct::RegisterUserWithPassword(const std::wstring& login, const std::wstring& name, const std::wstring& surname, const std::wstring& email, const std::vector<uint8_t>& password_hash)
{
	// 1. Добавление пользователя
	if (!InsertUsers(login, name, surname, email)) {
		std::wcout << L"Ошибка при добавлении пользователя" << std::endl;
		return false;
	}
	// 2. Получение user_id созданного пользователя
	int user_id = GetUserIdByLogin(login);
	if (user_id == -1) {
		std::wcout << L"Ошибка при получении ID пользователя" << std::endl;
		return false;
	}
	// 3. Добавление пароля
	if (!InsertPassword(user_id, password_hash)) {
		std::wcout << L"Ошибка при добавлении пароля" << std::endl;
		return false;
	}
	return true;
}

//функция проверки введенного пароля
bool DBStruct::IsCorrectPassword(const std::vector<uint8_t>& password_hash, const std::wstring& login)
{
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	auto cleanup = [&sqlStmt]() {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
	};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return false;
	}
	//подготовка строки SQL для выполнения
	if (!SQL_SUCCEEDED(SQLPrepareW(sqlStmt, (SQLWCHAR*)SQLQueries::GET_PASSWORD_HASH, SQL_NTS))) {
		std::wcout << L"Произошла ошибка SQLPrepareW: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//bind параметра
	SQLLEN loginLen = SQL_NTS;
	if (!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, login.length(), 0, (SQLPOINTER)login.c_str(), 0, &loginLen))) {
		std::wcout << L"Ошибка биндинга параметров: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//выполнение запроса
	if (!SQL_SUCCEEDED(SQLExecute(sqlStmt))) {
		std::wcout << L"Ошибка выполнения запроса: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//привязка буфера данных приложения к столбцу в результирующем наборе
	std::vector<uint8_t> buffer(password_hash.size());
	SQLLEN bytesReturned = 0;
	if (!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 1, SQL_C_BINARY, buffer.data(), buffer.size(), &bytesReturned))) {
		std::wcout << L"Ошибка биндинга колонки: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//получение данных для привязанных столбцов
	SQLRETURN fetch_res = SQLFetch(sqlStmt);
	if (fetch_res == SQL_NO_DATA) {
		std::wcout << L"Пользователь с логином '" << login << L"' не найден" << std::endl;
		cleanup();
		return false;
	}
	else if (!SQL_SUCCEEDED(fetch_res)) {
		std::wcout << L"Ошибка получения данных: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	if (bytesReturned == SQL_NULL_DATA || static_cast<size_t>(bytesReturned) != password_hash.size()) {
		cleanup();
		return false;
	}
	cleanup();
	return buffer == password_hash;
}

//функция объединение добавление message и message_participants
bool DBStruct::RegisterMessagesAll(const std::wstring& sender, const std::wstring& receiver, const std::wstring& text_message)
{
	//1. Добавление сообщения в таблицу message
	if (!AddMessage(text_message)) {
		std::wcout << L"Ошибка при добавлении сообщения" << std::endl;
		return false;
	}
	//2. Получение ID добавленного сообщения
	int message_id = GetMessageIdLast();
	if (message_id == -1) {
		std::wcout << L"Ошибка при получении ID сообщения" << std::endl;
		return false;
	}
	//3. Получение ID sender
	int sender_id = GetUserIdByLogin(sender);
	if (sender_id == -1) {
		std::wcout << L"Ошибка при получении ID отправителя (логин: " << sender << L")" << std::endl;
		return false;
	}
	//4. Получение ID receiver
	int receiver_id = GetUserIdByLogin(receiver);
	if (receiver_id == -1) {
		std::wcout << L"Ошибка при получении ID получателя (логин: " << receiver << L")" << std::endl;
		return false;
	}
	//5. Добавление записи в message_participants
	if (!AddMessageParticipants(message_id, sender_id, receiver_id)) {
		std::wcout << L"Ошибка при добавлении участников сообщения" << std::endl;
		return false;
	}

	return true;
}


//функция добавления сообщения
bool DBStruct::AddMessage(const std::wstring& text_message)
{
	//нахождение текущей даты-времени
	std::wstring date_time = getCurrentDateTime();
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	//лямбда-функция - для удобного освобождения дескриптора
	auto cleanup = [&sqlStmt]() {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
	};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return false;
	}
	//подготовка строки SQL для выполнения
	if (!SQL_SUCCEEDED(SQLPrepareW(sqlStmt, (SQLWCHAR*)SQLQueries::INSERT_MESSAGE, SQL_NTS))) {
		std::wcout << L"Произошла ошибка SQLPrepareW: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//bind параметров
	SQLLEN text_message_len = SQL_NTS;
	SQLLEN date_time_len = SQL_NTS;
	if (!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, text_message.length(), 0, (SQLPOINTER)text_message.c_str(), 0, &text_message_len)) ||
		!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, date_time.length(), 0, (SQLPOINTER)date_time.c_str(), 0, &date_time_len))) {
		std::wcout << L"Ошибка биндинга параметров: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//выполнение запроса
	if (!SQL_SUCCEEDED(SQLExecute(sqlStmt))) {
		std::wcout << L"Ошибка выполнения запроса: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	cleanup();
	return true;
}
//функция добавления данных в message_participants
bool DBStruct::AddMessageParticipants(int message_id, int sender_id, int receiver_id)
{
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	//лямбда-функция - для удобного освобождения дескриптора
	auto cleanup = [&sqlStmt]() {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
		};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return false;
	}
	//подготовка строки SQL для выполнения
	if (!SQL_SUCCEEDED(SQLPrepareW(sqlStmt, (SQLWCHAR*)SQLQueries::INSERT_MESSAGE_PARTICIPANTS, SQL_NTS))) {
		std::wcout << L"Произошла ошибка SQLPrepareW: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//bind параметров
	SQLLEN message_id_len = 0;
	SQLLEN sender_id_len = 0;
	SQLLEN receiver_id_len = 0;

	SQLBIGINT msg_id = message_id;
	SQLBIGINT snd_id = sender_id;
	SQLBIGINT rcv_id = receiver_id;
	if (!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 1, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_INTEGER, 0, 0, &msg_id, 0, &message_id_len)) ||
		!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 2, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_INTEGER, 0, 0, &snd_id, 0, &sender_id_len)) ||
		!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 3, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_INTEGER, 0, 0, &rcv_id, 0, &receiver_id_len))) {
		std::wcout << L"Ошибка биндинга параметров для message_participants: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}
	//выполнение запроса
	if (!SQL_SUCCEEDED(SQLExecute(sqlStmt))) {
		std::wcout << L"Ошибка выполнения INSERT в message_participants: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return false;
	}

	cleanup();
	return true;
}
//функция для нахождения последнего message_id
int DBStruct::GetMessageIdLast()
{
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	//лямбда-функция - для удобного освобождения дескриптора
	auto cleanup = [&sqlStmt]() {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
		};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return -1;
	}
	//отправка инструкции для однократного выполнения
	if (!SQL_SUCCEEDED(SQLExecDirectW(sqlStmt, (SQLWCHAR*)SQLQueries::GET_MESSAGE_ID, SQL_NTS))) {
		std::wcout << L"Произошла ошибка при получении message_id:\n" << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return -1;
	}
	SQLBIGINT message_id = 0;
	SQLLEN message_id_len = 0;
	if (SQLFetch(sqlStmt) == SQL_SUCCESS) {
		SQLGetData(sqlStmt, 1, SQL_C_SBIGINT, &message_id, 0, &message_id_len);
	}
	else {
		std::wcout << L"Ошибка получения ID из результата" << std::endl;
		cleanup();
		return -1;
	}
	cleanup();
	return static_cast<int>(message_id);
}
//функция вывода полученных сообщений
void DBStruct::SelectMessageReceived(const std::wstring& user)
{
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	//лямбда-функция - для удобного освобождения дескриптора
	auto cleanup = [&sqlStmt]() {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
		};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return;
	}
	//подготовка строки SQL для выполнения
	if (!SQL_SUCCEEDED(SQLPrepareW(sqlStmt, (SQLWCHAR*)SQLQueries::SELECT_MESSAGES, SQL_NTS))) {
		std::wcout << L"Произошла ошибка SQLPrepareW: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return;
	}
	//вычисление user_id по логину
	int user_id = GetUserIdByLogin(user);
	//bind параметра
	SQLLEN user_id_len = 0;
	SQLINTEGER uid = user_id;
	if (!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &uid, 0, &user_id_len))) {
		std::wcout << L"Ошибка биндинга параметра: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return;
	}
	//выполнение запроса
	if (!SQL_SUCCEEDED(SQLExecute(sqlStmt))) {
		std::wcout << L"Ошибка выполнения запроса: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return;
	}
	//привязка буферов данных приложения к столбцам в результирующем наборе
	constexpr int BUFFER_SIZE = 512;
	SQLWCHAR date_sent[BUFFER_SIZE] = { 0 };
	SQLWCHAR message_text[BUFFER_SIZE] = { 0 };
	SQLWCHAR message_status[BUFFER_SIZE] = { 0 };
	SQLWCHAR sender_login[BUFFER_SIZE] = { 0 };
	SQLLEN date_sent_len = 0;
	SQLLEN message_text_len = 0;
	SQLLEN message_status_len = 0;
	SQLLEN sender_login_len = 0;
	if (!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 1, SQL_C_WCHAR, date_sent, BUFFER_SIZE, &date_sent_len)) ||
		!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 2, SQL_C_WCHAR, message_text, BUFFER_SIZE, &message_text_len)) ||
		!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 3, SQL_C_WCHAR, message_status, BUFFER_SIZE, &message_status_len)) ||
		!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 4, SQL_C_WCHAR, sender_login, BUFFER_SIZE, &sender_login_len))) {
		std::wcout << L"Ошибка биндинга колонок: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return;
	}
	//вывод заголовка
	std::wcout << L"\n=== Входящие сообщения ===" << std::endl;
	//получение и вывод данных
	int message_count = 0;
	while (SQLFetch(sqlStmt) == SQL_SUCCESS) {
		message_count++;
		std::wcout << L"\n[" << message_count << L"] " << std::endl;
		std::wcout << L"Дата: " << date_sent << std::endl;
		std::wcout << L"От: " << sender_login << std::endl;
		std::wcout << L"Сообщение: " << message_text << std::endl;
		std::wcout << L"Статус сообщения: " << message_status << std::endl;
		std::wcout << L"-----------------" << std::endl;
	}
	if (message_count == 0) {
		std::wcout << L"Входящих сообщений нет." << std::endl;
	}
	else {
		std::wcout << L"\nВсего сообщений: " << message_count << std::endl;
	}
	if (message_count > 0) {
		SQLHANDLE updateStmt = SQL_NULL_HANDLE;

		if (SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, updateStmt))) {
			if (SQL_SUCCEEDED(SQLPrepareW(updateStmt, (SQLWCHAR*)SQLQueries::UPDATE_MESSAGE_STATUS_READ, SQL_NTS))) {
				SQLLEN user_id_len = 0;
				SQLINTEGER uid = user_id;

				if (SQL_SUCCEEDED(SQLBindParameter(updateStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &uid, 0, &user_id_len))) {
					if (!SQL_SUCCEEDED(SQLExecute(updateStmt))) {
						std::wcout << L"Ошибка обновления статуса сообщений: " << GetErrorFunc(SQL_HANDLE_STMT, updateStmt) << std::endl;
					}
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, updateStmt);
		}
	}
	cleanup();
}
//функция вывода отправленных сообщений
void DBStruct::SelectMessageSender(const std::wstring& user)
{
	SQLHANDLE sqlStmt = SQL_NULL_HANDLE;
	//лямбда-функция - для удобного освобождения дескриптора
	auto cleanup = [&sqlStmt]() {
		if (sqlStmt != SQL_NULL_HANDLE) {
			SQLFreeHandle(SQL_HANDLE_STMT, sqlStmt);
		}
		};
	//выделение дескриптора SQL_HANDLE_STMT
	if (!SQL_SUCCEEDED(this->CreateAlloc(SQL_HANDLE_STMT, this->sqldbc, sqlStmt))) {
		std::wcout << L"Произошла ошибка при подключении к STMT: " << GetErrorFunc(SQL_HANDLE_DBC, this->sqldbc) << std::endl;
		return;
	}
	//подготовка строки SQL для выполнения
	if (!SQL_SUCCEEDED(SQLPrepareW(sqlStmt, (SQLWCHAR*)SQLQueries::SELECT_MESSAGES_SENDER, SQL_NTS))) {
		std::wcout << L"Произошла ошибка SQLPrepareW: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return;
	}
	//вычисление user_id по логину
	int user_id = GetUserIdByLogin(user);
	//bind параметра
	SQLLEN user_id_len = 0;
	SQLINTEGER uid = user_id;
	if (!SQL_SUCCEEDED(SQLBindParameter(sqlStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &uid, 0, &user_id_len))) {
		std::wcout << L"Ошибка биндинга параметра: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return;
	}
	//выполнение запроса
	if (!SQL_SUCCEEDED(SQLExecute(sqlStmt))) {
		std::wcout << L"Ошибка выполнения запроса: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return;
	}
	//привязка буферов данных приложения к столбцам в результирующем наборе
	constexpr int BUFFER_SIZE = 512;
	SQLWCHAR date_sent[BUFFER_SIZE] = { 0 };
	SQLWCHAR message_text[BUFFER_SIZE] = { 0 };
	SQLWCHAR message_status[BUFFER_SIZE] = { 0 };
	SQLWCHAR receiver_login[BUFFER_SIZE] = { 0 };
	SQLLEN date_sent_len = 0;
	SQLLEN message_text_len = 0;
	SQLLEN message_status_len = 0;
	SQLLEN receiver_login_len = 0;
	if (!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 1, SQL_C_WCHAR, date_sent, BUFFER_SIZE, &date_sent_len)) ||
		!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 2, SQL_C_WCHAR, message_text, BUFFER_SIZE, &message_text_len)) ||
		!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 3, SQL_C_WCHAR, message_status, BUFFER_SIZE, &message_status_len)) ||
		!SQL_SUCCEEDED(SQLBindCol(sqlStmt, 4, SQL_C_WCHAR, receiver_login, BUFFER_SIZE, &receiver_login_len))) {
		std::wcout << L"Ошибка биндинга колонок: " << GetErrorFunc(SQL_HANDLE_STMT, sqlStmt) << std::endl;
		cleanup();
		return;
	}
	//вывод заголовка
	std::wcout << L"\n=== Отправленные сообщения ===" << std::endl;
	//получение и вывод данных
	int message_count = 0;
	while (SQLFetch(sqlStmt) == SQL_SUCCESS) {
		message_count++;
		std::wcout << L"\n[" << message_count << L"] " << std::endl;
		std::wcout << L"Дата: " << date_sent << std::endl;
		std::wcout << L"Кому: " << receiver_login << std::endl;
		std::wcout << L"Сообщение: " << message_text << std::endl;
		std::wcout << L"Статус сообщения: " << message_status << std::endl;
		std::wcout << L"-----------------" << std::endl;
	}
	if (message_count == 0) {
		std::wcout << L"Отправленных сообщений нет." << std::endl;
	}
	else {
		std::wcout << L"\nВсего сообщений: " << message_count << std::endl;
	}
	cleanup();
}