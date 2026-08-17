#pragma once 

#ifdef _WIN32
	#include <windows.h>
#else
	#include <clocale>
#endif

inline void init_utf8()
{
	#ifdef _WIN32
		SetConsoleCP(CP_UTF8); // Установка кодировки ввода консоли            
		SetConsoleOutputCP(CP_UTF8); // Установка кодировки вывода консоли
	#else
		std::setlocale(LC_ALL, "");
	#endif
}
