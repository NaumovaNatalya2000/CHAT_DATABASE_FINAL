#pragma once

namespace SQLQueries {
	////////////////CREATE////////////////
	//запрос создания таблицы users
	constexpr const wchar_t* CREATE_TABLE_USERS =
		L"CREATE TABLE IF NOT EXISTS Users("
		L"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		L"login TEXT NOT NULL UNIQUE, "
		L"name TEXT NOT NULL, "
		L"surname TEXT NOT NULL, "
		L"email TEXT NOT NULL"
		L")";
	//запрос создания таблицы passwords
	constexpr const wchar_t* CREATE_TABLE_PASSWORDS =
		L"CREATE TABLE IF NOT EXISTS Passwords("
		L"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		L"user_id INTEGER NOT NULL, "
		L"password_hash BLOB NOT NULL, "
		L"FOREIGN KEY(user_id) REFERENCES Users(id) ON DELETE CASCADE ON UPDATE CASCADE"
		L")";
	//запрос создания таблицы messages
	constexpr const wchar_t* CREATE_TABLE_MESSAGES =
		L"CREATE TABLE IF NOT EXISTS Messages("
		L"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		L"message_text TEXT, "
		L"date_send TEXT NOT NULL, "
		L"message_status TEXT NOT NULL, "
		L"CHECK(message_status IN('READ', 'NOTREAD'))"
		L")";
	//запрос создания таблицы message_participants
	constexpr const wchar_t* CREATE_TABLE_MESSAGE_PARTICIPANTS =
		L"CREATE TABLE IF NOT EXISTS Message_participants("
		L"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		L"message_id INTEGER NOT NULL, "
		L"sender_id INTEGER NOT NULL, "
		L"receiver_id INTEGER NOT NULL, "
		L"FOREIGN KEY(message_id) REFERENCES Messages(id) ON DELETE CASCADE ON UPDATE CASCADE, "
		L"FOREIGN KEY(sender_id) REFERENCES Users(id) ON DELETE CASCADE ON UPDATE CASCADE, "
		L"FOREIGN KEY(receiver_id) REFERENCES Users(id) ON DELETE CASCADE ON UPDATE CASCADE"
		L")";
	////////////////SELECT////////////////
	//select логинов
	constexpr const wchar_t* SELECT_LOGIN =
		L"SELECT login "
		L"FROM Users "
		L"ORDER BY login";
	//select exists login - проверка логина на уникальность
	constexpr const wchar_t* CHECK_LOGIN_EXISTS =
		L"SELECT EXISTS("
		L"SELECT 1 "
		L"FROM Users "
		L"WHERE login = ?"
		L")";
	//select id по логину
	constexpr const wchar_t* GET_USER_ID =
		L"SELECT ID "
		L"FROM Users "
		L"WHERE login = ?";
	//select password_hash по логину
	constexpr const wchar_t* GET_PASSWORD_HASH =
		L"SELECT password_hash "
		L"FROM Passwords "
		L"WHERE user_id = ("
		L"SELECT id "
		L"FROM Users "
		L"WHERE login = ? )";
	//select последнего добавленного id message
	constexpr const wchar_t* GET_MESSAGE_ID =
		L"SELECT last_insert_rowid()";
	//select полученных сообщений
	constexpr const wchar_t* SELECT_MESSAGES =
		L"SELECT "
		L"m.date_send, "
		L"m.message_text, "
		L"m.message_status, "
		L"sender.login "
		L"FROM messages as m "
		L"JOIN message_participants as mp ON m.id = mp.message_id "
		L"JOIN users as sender ON mp.sender_id = sender.id "
		L"WHERE mp.receiver_id = ? "
		L"ORDER BY m.date_send DESC";
	//select отправленных сообщений
	constexpr const wchar_t* SELECT_MESSAGES_SENDER =
		L"SELECT "
		L"m.date_send, "
		L"m.message_text, "
		L"m.message_status, "
		L"receiver.login "
		L"FROM messages as m "
		L"JOIN message_participants as mp ON m.id = mp.message_id "
		L"JOIN users as receiver ON mp.receiver_id = receiver.id "
		L"WHERE mp.sender_id = ? "
		L"ORDER BY m.date_send DESC";


	////////////////INSERT////////////////
	//insert УЗ
	constexpr const wchar_t* INSERT_USER =
		L"INSERT INTO Users("
		L"login, name, surname, email) "
		L"VALUES (?, ?, ?, ?)";
	//insert пароля
	constexpr const wchar_t* INSERT_PASSWORD =
		L"INSERT INTO Passwords("
		L"user_id, password_hash) "
		L"VALUES (?, ?)";
	//insert message
	constexpr const wchar_t* INSERT_MESSAGE =
		L"INSERT INTO Messages("
		L"message_text, date_send, message_status) "
		L"VALUES (?, ?, 'NOTREAD')";
	//insert message_participants
	constexpr const wchar_t* INSERT_MESSAGE_PARTICIPANTS =
		L"INSERT INTO Message_participants("
		L"message_id, sender_id, receiver_id) "
		L"VALUES (?, ?, ?)";
	

	////////////////UPDATE////////////////
	constexpr const wchar_t* UPDATE_MESSAGE_STATUS_READ =
		L"UPDATE Messages "
		L"SET message_status = 'READ' "
		L"WHERE EXISTS ("
		L"SELECT 1 "
		L"FROM message_participants as mp "
		L"WHERE mp.message_id = Messages.id "
		L"AND mp.receiver_id = ?"
		L")";
}