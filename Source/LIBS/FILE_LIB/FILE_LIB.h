#pragma once
#include <cstddef>
#include <vector>
#include <string>
#include "FTP_LIB.h"


bool WRITE_INFO(std::vector <FTP_CONFIG>& VECTOR);
bool GET_INFO(std::vector <FTP_CONFIG>& VECTOR);

//построение дериктории клиента
/*
void DIRGET_CLIENT(std::vector<std::filesystem::directory_entry>& VP,std::string path);

std::string UP_PATH(std::string path);

std::string GET_ROOT();

std::string GET_FILE_TIME(const std::filesystem::directory_entry& entry);

bool CREATE_CDIR(std::string PATH_DIR_PC,
	    	 std::string PATH_DIR_SERVER);
*/
