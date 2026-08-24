#include <fstream>   
#include <string>
#include <iostream>
#include <vector>
#include "FILE_LIB.hpp"
#include <nlohmann/json.hpp>

//Робата с TXT
bool GET_INFO(std::vector <FTP_CONFIG>& VECTOR)
{
	std::string PATH = GetExePath() + "LIST.json";
	FTP_CONFIG OBJ("","","","");	
	std::ifstream JSON_FILE(PATH);

	if(!JSON_FILE.is_open())
	{
		std::cout << "FATAL_ERROR_OF_MEMORY" << "\n";
		JSON_FILE.close();
		return false;
	}

	nlohmann::json json_list;
	JSON_FILE >> json_list;
	JSON_FILE.close();

	size_t size = json_list["profiles"].size();
	size_t index{0};

	while(index < size)
	{
		OBJ.URL = CHANGE_PATH(json_list["profiles"][index]["URL"]);
		OBJ.USERNAME = json_list["profiles"][index]["USER"];
		OBJ.PASSWORD = json_list["profiles"][index]["PASSWORD"];
		OBJ.CERT_PATH = CHANGE_PATH(json_list["profiles"][index]["CERT"]);

		VECTOR.push_back(OBJ);
		index++;
	}	
	return true;
}

bool WRITE_INFO(std::vector <FTP_CONFIG>& VECTOR)
{
	std::string PATH = GetExePath() + "LIST.json";
	std::ofstream JSON_FILE(PATH);
	nlohmann::json list;
	nlohmann::json profile;
	list["profiles"] = nlohmann::json::array();	

	if(!JSON_FILE.is_open()) 
	{
		std::cout << "FATAL_ERROR_OF_MEMORY" << "\n";
		return false;
	}

	for(int EL{0};EL < VECTOR.size();EL++)
	{
		profile["URL"] = VECTOR[EL].URL;
		profile["USER"] = VECTOR[EL].USERNAME;
		profile["PASSWORD"] = VECTOR[EL].PASSWORD;
		profile["CERT"] = VECTOR[EL].CERT_PATH;

		list["profiles"].push_back(profile);
	}

	JSON_FILE << list.dump(4);
	
	return true;
}


