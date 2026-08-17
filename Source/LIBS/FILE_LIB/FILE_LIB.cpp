#include <fstream>   
#include <string>
#include <iostream>
#include <vector>
#include "FILE_LIB.hpp"

//Робата с TXT
bool GET_INFO(std::vector <FTP_CONFIG>& VECTOR)
{
	int num_el = 0;
	std::string line;
	std::string PATH = GetExePath() + "VECTOR.txt";
	//std::cout << PATH << "\n";
	FTP_CONFIG OBJ("","","","");	
	std::ifstream VECTOR_F(PATH);

	if(VECTOR_F.is_open())
	{
		while(getline(VECTOR_F,line))
		{
			//std::cout << line << "\n";
			if(line == "#79")
			{
				num_el = 79;
			}

			switch(num_el)
			{
				case(0):
					OBJ.URL = CHANGE_PATH(line);
					break;
				case(1):
					OBJ.USERNAME = line;
					break;
				case(2):
					OBJ.PASSWORD = line;
					break;
				case(3): 
					OBJ.CERT_PATH = CHANGE_PATH(line);
					VECTOR.push_back(OBJ);
					num_el = -1;
					break;
				case(79):
					//std::cout << size << "1\n";
					VECTOR_F.close();
					return true;
					break;
				default:
					//std::cout << size << "2\n";	
					VECTOR_F.close();
					return false;
		 			break;

			}
			num_el++;
		}
	}
	else
	{
		std::cout << "FATAL_ERROR_OF_MEMORY" << "\n";
		VECTOR_F.close();
		return false;
	}
	//std::cout << size << "4\n";	
	VECTOR_F.close();
	return true;
}

bool WRITE_INFO(std::vector <FTP_CONFIG>& VECTOR)
{
	std::string PATH = GetExePath() + "VECTOR.txt";
	std::ofstream VECTOR_f(PATH);
	if(!VECTOR_f.is_open()) 
	{
		std::cout << "FATAL_ERROR_OF_MEMORY" << "\n";
		return false;
	}

	for(int EL{0};EL < VECTOR.size();EL++)
	{
		VECTOR_f << VECTOR[EL].URL << "\n";
		VECTOR_f << VECTOR[EL].USERNAME << "\n";
		VECTOR_f << VECTOR[EL].PASSWORD << "\n";
		VECTOR_f << VECTOR[EL].CERT_PATH << "\n";
	}

	VECTOR_f << "#79";
	VECTOR_f.close();
	return true;
}


