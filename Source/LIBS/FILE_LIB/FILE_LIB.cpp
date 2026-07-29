#include <fstream>   
#include <string>
#include <iostream>
#include <vector>
#include "FILE_LIB.h"

//Робата с TXT
bool GET_INFO(std::vector <FTP_CONFIG>& VECTOR)
{
	int num_el = 0;
	std::string line;
	FTP_CONFIG OBJ("","","","");	
	std::ifstream VECTOR_F("VECTOR.txt");

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
		//std::cout << size << "3\n";
		VECTOR_F.close();
		return false;
	}
	//std::cout << size << "4\n";	
	VECTOR_F.close();
	return true;
}

bool WRITE_INFO(std::vector <FTP_CONFIG>& VECTOR)
{
	std::ofstream VECTOR_f("VECTOR.txt");
	if(!VECTOR_f.is_open()) 
	{
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


