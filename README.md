# Проект Чатик
На текущий момент произведена реализация БД для чата.
БД используется в проекте - SQLite. Для корректной работы программы необходимо установить драйвер ODBC "SQLite3 ODBC Driver"
БД при запуске программы создается автоматически в папке, в которой находится запускаемая программа.
Если в папке уже есть файл БД - подключение происходит к нему.
### Структура проекта:
1. Databases.h + Databases.cpp - Реализация работы с БД
2. FunctionsMenu.h + FunctionsMenu.cpp - Реализация меню приложения
3. FunctionsSystem.h + FunctionsSystem.cpp - Реализация системных функций
4. SQLQueries.h - Все SQL-запросы проекта
5. sha1.h + sha1.cpp - Реализация алгоритма SHA-1
6. IncludeLibrary.h - Общие включения заголовочных файлов
7. main.cpp - Точка входа в программу
### Структура БД:
Таблица Users:
- id - INTEGER PRIMARY KEY AUTOINCREMENT
- login - TEXT NOT NULL UNIQUE (уникальный логин пользователя)
- name - TEXT NOT NULL (имя пользователя)
- surname - TEXT NOT NULL (фамилия пользователя)
- email - TEXT NOT NULL (email пользователя)
Таблица Passwords:
- id - INTEGER PRIMARY KEY AUTOINCREMENT
- user_id - INTEGER NOT NULL (FK на Users.id, ON DELETE CASCADE)
- password_hash - BLOB NOT NULL (хеш пароля, алгоритм SHA-1)
Таблица Messages:
- id - INTEGER PRIMARY KEY AUTOINCREMENT
- message_text - TEXT (текст сообщения)
- date_send - TEXT NOT NULL (дата и время отправки в формате "YYYY-MM-DD HH:MM:SS")
- message_status - TEXT NOT NULL (статус: 'READ' или 'NOTREAD')
Таблица Message_participants:
- id - INTEGER PRIMARY KEY AUTOINCREMENT
- message_id - INTEGER NOT NULL (FK на Messages.id, ON DELETE CASCADE)
- sender_id - INTEGER NOT NULL (FK на Users.id, ON DELETE CASCADE)
- receiver_id - INTEGER NOT NULL (FK на Users.id, ON DELETE CASCADE)
### Функционал:
#### Databases:
**Переменные класса:**
SQLHANDLE sqlenv;
SQLHANDLE sqldbc;
**Методы класса:**
`DBStruct();` - конструктор создания
`DBStruct& operator=(const DBStruct&) = delete;` - запрет конструктора копирования
`~DBStruct();` - деструктор
`std::wstring GetErrorFunc(SQLSMALLINT handleType, SQLHANDLE sqlHan);` - функция для диагностики ошибок - вывода
`SQLRETURN CreateAlloc(SQLSMALLINT handleType, SQLHANDLE sqlHanParent, SQLHANDLE& sqlHan);` - функция SQLAllocHandle
`int ALL_CONNECT_DB();` - функция, объединяющая создание дескрипторов и подключение к БД - возвращаем - 1 при ошибке, 0 - успех
`void CreateTables();` - функция создания Таблиц
`int DB_Check_Login(const std::wstring& login);` - функция проверки уникальности логина
`void SelectLogin();` - функция вывода логинов
`bool InsertUsers(const std::wstring& login, const std::wstring& name, const std::wstring& surname, const std::wstring& email);` - добавление User-а
`int GetUserIdByLogin(const std::wstring& login);` - функция для нахождения user_id по логину
`bool InsertPassword(int user_id, const std::vector<uint8_t>& password_hash);` - добавление пароля
`bool RegisterUserWithPassword(const std::wstring& login, const std::wstring& name, const std::wstring& surname, const std::wstring& email, const std::vector<uint8_t>& password_hash);` - функция объединение добавления user-а и пароля
`bool IsCorrectPassword(const std::vector<uint8_t>& password_hash, const std::wstring& login);` - функция проверки введенного пароля
`bool AddMessage(const std::wstring& text_message);` - функция добавления сообщения
`bool AddMessageParticipants(int message_id, int sender_id, int receiver_id);` - функция добавления данных в message_participants
`bool RegisterMessagesAll(const std::wstring& sender, const std::wstring& receiver, const std::wstring& text_message);` - функция объединение добавление message и message_participants
`int GetMessageIdLast();` - функция для нахождения последнего message_id
`void SelectMessageReceived(const std::wstring& user);` - функция вывода полученных сообщений
`void SelectMessageSender(const std::wstring& user);` - функция вывода отправленных сообщений
#### FunctionsMenu:
**Функции:**
`void MainMenu(DBStruct& DB);` - функция основное меню
`void RegistrationMenu(DBStruct& DB);` - функция подменю регистрации
`void MessageMenu(DBStruct& DB);` - функция подменю сообщений
`void SendingMenu(DBStruct& DB, const std::wstring& login);` - функция подменю отправки и просмотра сообщений
`std::wstring NotEmptyString(const std::wstring& prompt);` - функция ввода строки с проверкой на пустоту
`wchar_t NotEmptyChar();` - функция проверки потока ввода на пустоту - управление меню
#### FunctionsSystem:
**Функции:**
`void clearScreen();` - функция очистки экрана
`void pause();` - функция паузы
`std::wstring getCurrentDateTime();` - Функция для получения текущей даты и времени как std::wstring
#### SQLQueries:
**Запросы:**
`CREATE_TABLE_USERS` - запрос создания таблицы users
`CREATE_TABLE_PASSWORDS` - запрос создания таблицы passwords
`CREATE_TABLE_MESSAGES` - запрос создания таблицы messages
`CREATE_TABLE_MESSAGE_PARTICIPANTS` - запрос создания таблицы message_participants
`SELECT_LOGIN` - select логинов
`CHECK_LOGIN_EXISTS` - select exists login - проверка логина на уникальность
`GET_USER_ID` - select id по логину
`GET_PASSWORD_HASH` - select password_hash по логину
`GET_MESSAGE_ID` - select последнего добавленного id message
`SELECT_MESSAGES` - select полученных сообщений
`SELECT_MESSAGES_SENDER` - select отправленных сообщений
