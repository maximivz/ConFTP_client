#pragma once
#include <cstddef>
#include <vector>
#include <string>
#include "FTP_LIB.h"

#ifndef _WIN32
	#include <unistd.h>   // readlink
	#include <limits.h>   // PATH_MAX
#endif

inline std::string GetExePath() 
{
	#ifdef _WIN32
		return("");
	#else
    		char buffer[PATH_MAX]; // буфер для хранения пути

    		// readlink читает содержимое символической ссылки /proc/self/exe
   		// и записывает его в buffer. Возвращает количество записанных байт
    		// или -1 в случае ошибки.
    		ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);

    		// Если readlink отработал успешно — оборачиваем результат в std::string
    		// (buffer, len) гарантирует правильную длину строки без ручного
    		// добавления нулевого символа.
    		// Если произошла ошибка — возвращаем пустую строку.
		std::string PATH = (len != -1) ? std::string(buffer, len) : std::string();
		PATH = PATH.substr(0,PATH.rfind('/')) + '/';
		return PATH;

	#endif
}


bool WRITE_INFO(std::vector <FTP_CONFIG>& VECTOR);
bool GET_INFO(std::vector <FTP_CONFIG>& VECTOR);


