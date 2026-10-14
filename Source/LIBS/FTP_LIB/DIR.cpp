#include "FTP_LIB.h"
#include <curl/curl.h>
#include <sstream>   
#include <string>
#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>

void PARSER_DIR(FILE_INFO& DIR_OBJ, std::string DIR_LINE)
{
	CLEAN_STRING(DIR_LINE);
	
	DIR_OBJ.type = DIR_LINE[0];
	DIR_LINE.erase(0,1);

	std::stringstream stream_dir(DIR_LINE);

	stream_dir >> DIR_OBJ.prava
		   >> DIR_OBJ.links
        	   >> DIR_OBJ.user
        	   >> DIR_OBJ.group
        	   >> DIR_OBJ.size
        	   >> DIR_OBJ.month
        	   >> DIR_OBJ.day
                   >> DIR_OBJ.time_or_year;

	std::getline(stream_dir, DIR_OBJ.name);

	if(!DIR_OBJ.name.empty() && DIR_OBJ.name[0] == ' ')
		DIR_OBJ.name.erase(0, 1);
}

//ВЫВОД директории c сервер (просмотр дир сервера)
CURLcode FTP_CONFIG::DIRGET(std::vector <FILE_INFO>& VD, std::string ADD_PATH)
{
	VD.clear();
	std::string DIRLIST{""};
	std::string line{""};

	curl_easy_setopt(curl, CURLOPT_URL, (URL + ADD_PATH + "/").c_str());	
	curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());

	// получаем данные через callback
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WRITE_CALLB_DIR);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &DIRLIST);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_reset(curl);
	

	if(res != CURLE_OK)
		return res;
	
	std::istringstream stream(DIRLIST);
	int num{0};
	while(getline(stream, line))
	{
		FILE_INFO OBJ;
		PARSER_DIR(OBJ,line);
		
		OBJ.PATH = ADD_PATH + "/" + OBJ.name;		
		VD.push_back(OBJ);	
	}
	return res;
}


