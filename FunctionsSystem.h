#pragma once
#include<iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
//функция очистки экрана
void clearScreen();
//функция паузы
void pause();
// Функция для получения текущей даты и времени как std::wstring
std::wstring getCurrentDateTime();