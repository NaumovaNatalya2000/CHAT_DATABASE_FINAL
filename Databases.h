#pragma once
#include <iostream>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "FunctionsSystem.h"
#include "SQLQueries.h"
#include <vector>

struct DBStruct {
	SQLHANDLE sqlenv;
	SQLHANDLE sqldbc;
	//конструктор создания
	DBStruct();
	//запрет конструктора копирования
	DBStruct& operator=(const DBStruct&) = delete;
	//деструктор
	~DBStruct();

	//функция для диагностики ошибок - вывода
	std::wstring GetErrorFunc(SQLSMALLINT handleType, SQLHANDLE sqlHan);
	//функция SQLAllocHandle
	SQLRETURN CreateAlloc(SQLSMALLINT handleType, SQLHANDLE sqlHanParent, SQLHANDLE& sqlHan);
	//функция, объединяющая создание дескрипторов и подключение к БД - возвращаем - 1 при ошибке, 0 - успех
	int ALL_CONNECT_DB();


	//функция создания Таблиц
	void CreateTables();
	//функция проверки уникальности логина
	int DB_Check_Login(const std::wstring& login);
	//функция вывода логинов
	void SelectLogin();
	//добавление User-а
	bool InsertUsers(const std::wstring& login, const std::wstring& name, const std::wstring& surname, const std::wstring& email);
	//функция для нахождения user_id по логину
	int GetUserIdByLogin(const std::wstring& login);
	//добавление пароля
	bool InsertPassword(int user_id, const std::vector<uint8_t>& password_hash);
	//функция объединение добавления user-а и пароля
	bool RegisterUserWithPassword(const std::wstring& login, const std::wstring& name, const std::wstring& surname, const std::wstring& email, const std::vector<uint8_t>& password_hash);

	//функция проверки введенного пароля
	bool IsCorrectPassword(const std::vector<uint8_t>& password_hash, const std::wstring& login);
	//функция добавления сообщения
	bool AddMessage(const std::wstring& text_message);
	//функция добавления данных в message_participants
	bool AddMessageParticipants(int message_id, int sender_id, int receiver_id);
	//функция объединение добавление message и message_participants
	bool RegisterMessagesAll(const std::wstring& sender, const std::wstring& receiver, const std::wstring& text_message);
	//функция для нахождения последнего message_id
	int GetMessageIdLast();

	//функция вывода полученных сообщений
	void SelectMessageReceived(const std::wstring& user);
	//функция вывода отправленных сообщений
	void SelectMessageSender(const std::wstring& user);
};