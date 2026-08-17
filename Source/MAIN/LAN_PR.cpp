#include <iostream>
#include <string>
#include <fstream>
#include "locale.hpp"
#include "FTP_LIB.h"
#include "FILE_LIB.hpp"
#include "A_PC_LIB.h"

int main()
{
	//локализация
	init_utf8();

	std::vector<FTP_CONFIG> VECTOR;
	
	//std::cout << "get" << "\n";
	//сбор данных из файла
	GET_INFO(VECTOR);	
	
	//std::cout << "admin" << "\n";
	//выбор действий - управление
	ADMIN(VECTOR);

	//ввод данных в файл
	WRITE_INFO(VECTOR);
}


