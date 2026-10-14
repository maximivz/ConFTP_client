#include <iostream>
#include <string>
#include <fstream>
#include <curl/curl.h>
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
	
	if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
	{
		std::cerr << "CURL_GLOBAL_ERROR_INIT!!!";
		return 1;
	}

	//выбор действий - управление
	ADMIN(VECTOR);

	//ввод данных в файл
	WRITE_INFO(VECTOR);
	
	curl_global_cleanup();
	return 0;
}


