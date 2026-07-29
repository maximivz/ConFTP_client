#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include "FTP_LIB.h"
#include "FILE_LIB.h"
#include "A_PC_LIB.h"

int main()
{
	//локализация
	SetConsoleCP(CP_UTF8); // Установка кодировки ввода консоли            
	SetConsoleOutputCP(CP_UTF8); // Установка кодировки вывода консоли
	
	std::vector<FTP_CONFIG> VECTOR;

	//сбор данных из файла
	GET_INFO(VECTOR);	
	
	//выбор действий - управление
	ADMIN(VECTOR);

	//ввод данных в файл
	WRITE_INFO(VECTOR);
}


