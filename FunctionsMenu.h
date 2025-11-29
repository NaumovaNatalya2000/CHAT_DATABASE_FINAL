#pragma once
#include<iostream>
#include<string>
#include "Databases.h"
#include "FunctionsSystem.h"
#include "sha1.h"

//функция основное меню
void MainMenu(DBStruct& DB);

//функция подменю регистрации
void RegistrationMenu(DBStruct& DB);

//функция подменю сообщений
void MessageMenu(DBStruct& DB);

//функция подменю отправки и просмотра сообщений
void SendingMenu(DBStruct& DB, const std::wstring& login);

//функция ввода строки с проверкой на пустоту
std::wstring NotEmptyString(const std::wstring& prompt);

//функция проверки потока ввода на пустоту - управление меню
wchar_t NotEmptyChar();