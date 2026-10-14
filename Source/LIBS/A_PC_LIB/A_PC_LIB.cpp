#include <iostream>
#include <limits>
#include <vector>
#include <string>
#include <iomanip>
#include <curl/curl.h>
#include "A_PC_LIB.h"
#include "filesystem"


void WATCH_DIR(std::vector <FILE_INFO>& VD, std::string URL)
{
	if (VD.empty()) 
	{
    		std::cout << "Пусто\n";
	}
	else
	{
		std::cout << std::string(100, '=') << "\n";
		std::cout << "||| " << "ДИРЕКТОРИЯ(путь): "<< URL << S_UP_PATH(VD[0].PATH) << "\n";
		std::cout << std::string(100, '=') << "\n";

		std::cout << std::left
		<< std::setw(5)  << "num"    << " | "    
          	<< std::setw(4)  << "Type"   << " | "
          	<< std::setw(12) << "PRAVA"  << " | "
          	<< std::setw(6)  << "LINKS"  << " | "
          	<< std::setw(10) << "USER"   << " | "
          	<< std::setw(10) << "GROUP"  << " | "
          	<< std::setw(15) << "SIZE"   << " | "
          	<< std::setw(15) << "DATE"   << " | "
          	<< "NAME\n";

		std::cout << std::string(100, '=') << "\n"; // 100 едениц символов =
		
		int num{0};
		for(const FILE_INFO& DIR_OBJ : VD)
		{

			std::cout << std::left
			<< std::setw(5)  << num 	  << " | "
          		<< std::setw(4)  << DIR_OBJ.type  << " | "
          		<< std::setw(12) << DIR_OBJ.prava << " | "
          		<< std::setw(6)  << DIR_OBJ.links << " | "   
          		<< std::setw(10) << DIR_OBJ.user  << " | "
          		<< std::setw(10) << DIR_OBJ.group << " | "
          		<< std::setw(15) << DIR_OBJ.size  << " | "
          		<< std::setw(15) << (DIR_OBJ.month + " " + DIR_OBJ.day + " " + DIR_OBJ.time_or_year) << " | "
          		<< DIR_OBJ.name  << "\n";
			num++;
		}	
	}
}

//СПИСОК ПРОФИЛЕЙ
void LISTS_PROFILES(const std::vector<FTP_CONFIG> VECTOR)
{
	std::cout << "\n==============================\n";
	std::cout << "Количество: " << VECTOR.size() << "\n";

	std::cout << std::left
		<< std::setw(5)  << "Nomer"       << " | "
		<< std::setw(25) << "URL"         << " | "
		<< std::setw(15) << "USER"<< " | "
		<< std::setw(20) << "PASSWORD"      << " | "
		<< std::setw(20) << "SERT"  << "\n";

	for(int i = 0; i < VECTOR.size(); i++)
	{
		std::cout << std::left			
		<< std::setw(5)  << i << " | "
		<< std::setw(25) << VECTOR[i].URL << " | "
		<< std::setw(15) << VECTOR[i].USERNAME << " | "
		<< std::setw(20) << VECTOR[i].PASSWORD << " | "
		<< std::setw(20) << VECTOR[i].CERT_PATH << "\n";
	}
	std::cout << "==============================\n";
}

//НОВЫЙ ПРОФИЛЬ
void NEW_PROFILE(std::vector<FTP_CONFIG>& VECTOR)
{
	FTP_CONFIG NEW("","","","");
	
	std::cout << "\n";		
	std::cout << "URL: ";
	std::cin >> NEW.URL;
	std::cout << "USERNAME: ";
	std::cin >> NEW.USERNAME;
	std::cout << "PASSWORD: ";
	std::cin >> NEW.PASSWORD;
	std::cout << "Сертификат: ";
	std::cin >> NEW.CERT_PATH;

	VECTOR.emplace_back(NEW);
}

void help()
{
	std::cout << " 1 - выбор профиля " << "\n"
		  << " 2 - добавление нового профиля " << "\n"
		  << " 3 - просмотр сервера "  << "\n"
		  << " 4 - просмотр клиента "  << "\n"
		  << " 5 - UPLOAD " << "\n"
		  << " 6 - DOWNLOAD   " << "\n"
		  << " 7 - статус" << "\n"
		  << " 8 - завершение работы " << "\n"
		  << " 9 - удаление профиля" << "\n";

}

void clearConsole()
{
	#if defined(_WIN32) || defined(_WIN64) 
		system("cls"); 
	#else // ANSI escape-последовательность, работает в Linux и macOS 
	      	std::cout << "\033[2J\033[1;1H"; 
	#endif 
}

//ожидание
void WAIT()
{
	char smth;
	std::cout << "Wait: ";
	std::cin >> smth;
	std::cout << "\n";
}

void CHOICE_PROFILE(std::vector<FTP_CONFIG>& VECTOR, FTP_CONFIG*& T_FTP,bool& first_time)
{
    	int choice;

    	LISTS_PROFILES(VECTOR);

    	std::cout << "Выберите: ";
    	while (!(std::cin >> choice) || choice < 0 || choice >= static_cast<int>(VECTOR.size()))
    	{
        	std::cin.clear();
        	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        	std::cout << "Выберите: ";
    	}

    	// если уже был активный профиль — выключаем его
    	if (!first_time && T_FTP)
    	{
        	T_FTP->DEACTIVATION_PROFILE();
    	}

    	// переключаемся на новый
    	T_FTP = &VECTOR[choice];

    	CURLcode res = T_FTP->ACTIVATION_PROFILE();
    	if (res != CURLE_OK)
    	{
        	std::cout << "Ошибка: " << curl_easy_strerror(res) << "\n";
        	first_time = false;
        	return;
    	}

    	first_time = false;
}

void DELETE_PROFILE(std::vector<FTP_CONFIG>& VECTOR, FTP_CONFIG* T_FTP, bool& first_time)
{
    	int choice;

    	LISTS_PROFILES(VECTOR);

    	std::cout << "Выберите: ";
    	while (!(std::cin >> choice) || choice < 0 || choice >= static_cast<int>(VECTOR.size()))
    	{
        	std::cin.clear();
        	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        	std::cout << "Выберите: ";
    	}

    	if (T_FTP == &VECTOR[choice])
    	{
        	T_FTP->DEACTIVATION_PROFILE();
    		first_time = true;	
    	}

	VECTOR.erase(VECTOR.begin() + choice);
}

//серверная файловая система
void SERVER_FILE_SYSTEM(FTP_CONFIG& T_FTP,std::string& GET_OBJ,char& Tobj)
{
	//обьявление
	std::string PATH{""};
	std::vector <FILE_INFO> VD;
	CURLcode res = T_FTP.DIRGET(VD, PATH);
	if(res != CURLE_OK)
	{
		std::cout << curl_easy_strerror(res) << "\n";
	}

	bool process_dir{true};
	std::string choice_dir;

	//перый осмотр
	WATCH_DIR(VD, T_FTP.URL);

	//цикл действий
	while(process_dir)
	{
		//ввод
		std::cout<< "Введите действе: ";
		std::cin >> choice_dir; 

		//команда подьема на верх
		if(choice_dir == "u")
		{
			if(PATH != "")
			{
				PATH = S_UP_PATH(PATH);

				CURLcode res = T_FTP.DIRGET(VD, PATH);
				if(res != CURLE_OK)
				{
					std::cout << curl_easy_strerror(res) << "\n";
				}

				WATCH_DIR(VD, T_FTP.URL);	
			}
			else
			{
				std::cout << "Достигнут корень" << "\n";
			}
		}
		else if(choice_dir == "d")//команда осмотра дир
		{
			CURLcode res = T_FTP.DIRGET(VD, PATH);
			if(res != CURLE_OK)
			{
				std::cout << curl_easy_strerror(res) << "\n";
			}
	
			WATCH_DIR(VD, T_FTP.URL);	
		}
		else if(choice_dir == "q")//выход из SERVER_FILE_SYSTEM 
		{
			process_dir = false;	
		}
		else if(choice_dir == "t")
		{
			GET_OBJ = PATH;
			Tobj = 'd';
			process_dir = false;
		}
		else if(choice_dir == "help")//справка о командах
		{
			std::cout << "u   - вверх по директории" << "\n"
				  << "q    - завершение работы / выход" << "\n"
				  << "help - помощь / справка" << "\n"
				  << "t - выбор текущей директории для отправки" << "\n"
				  << "d  - просмотр текущей библиотеки" << "\n";
		}
		else//альтернатива - цифры прогулка по древу
		{
			//буфер цифр
			int num;
			//проверка на цифры
			if(StringToInt(choice_dir, num) && num >= 0 && num < VD.size())
			{
				//информирование о выбранном обьекте
				std::cout << "Выбран обьект: " << VD[num].name << "\n\n";

				//диретория ли это
				if(VD[num].type == 'd')
				{
					PATH = VD[num].PATH;	
					CURLcode res = T_FTP.DIRGET(VD, PATH);
					OK_CONNECTION(res);

					WATCH_DIR(VD, T_FTP.URL);	
				}
				else //файл как обьект передачи
				{
					std::cout << "Вы уверенны? y/n " << "\n";
					char fs;
					std::cin >> fs;
					if(fs == 'y')
					{
						GET_OBJ = VD[num].PATH;
						Tobj = 'f';
						process_dir = false;
					}
					else
					{
						WATCH_DIR(VD, T_FTP.URL);
					}	
				}
			}
			else //ошибка ввода
			{
				std::cout << "ОШИБКА_ВВОДА" << "\n";
			}
		}
	}
}

//Клиент файловая система
void WATCH_CLIENT(std::vector<std::filesystem::directory_entry>& VP, std::string PATH)
{
	std::cout << std::string(100, '=') << "\n";
	std::cout << "||| " << "DIR: "<< PATH << "\n";
	std::cout << std::string(100, '=') << "\n";
		
	std::cout << std::left
		<< std::setw(5)  << "Num"       << " | "
		<< std::setw(5)  << "TYPE"      << " | "
		<< std::setw(30) << "SIZE"      << " | "
		<< std::setw(20) << "DATE"	<< " | "
		<< std::setw(40) << "NAME"  	<< "\n";

	for(int i = 0; i < VP.size(); i++)
	{
		char type{'-'};
		std::string size = "-";
		if (VP[i].is_regular_file())
    			size = std::to_string(VP[i].file_size());

		if(VP[i].is_directory())
		{
			type = 'd';
		}
		else if(VP[i].is_regular_file())
		{
			type = 'f';
		}
		else
		{
			type = 'l';
		}

		std::cout << std::left			
		<< std::setw(5)  << i << " | "
		<< std::setw(5)  << type << " | "
		<< std::setw(30) << size << " | "
		<< std::setw(20) << GET_FILE_TIME(VP[i]) << " | "
		<< std::setw(40) << VP[i].path().filename().string()  << "\n";
	}
	std::cout << "--==============================--\n";

}


void CLIENT_FILE_SYSTEM(std::string& GET_OBJ)
{
	bool process_dir{true};
	std::string choice_dir;

	//корень
	std::string ROOT = GET_ROOT();
	std::string PATH = ROOT;
	std::vector<std::filesystem::directory_entry> V_PATH; 
	
	DIRGET_CLIENT(V_PATH,ROOT);
	WATCH_CLIENT(V_PATH, PATH);

	//цикл действий
	while(process_dir)
	{
		//ввод
		std::cout<< "Введите действе: ";
		std::cin >> choice_dir; 

		//команда подьема на верх
		if(choice_dir == "u")
		{
			if(PATH != ROOT)
			{
				PATH = UP_PATH(PATH);
				DIRGET_CLIENT(V_PATH,PATH);
				WATCH_CLIENT(V_PATH, PATH);
			}
			else
			{
				std::cout << "Достигнут корень" << "\n";
			}	
		}
		else if(choice_dir == "d")//команда осмотра дир
		{	
			DIRGET_CLIENT(V_PATH,PATH);
			WATCH_CLIENT(V_PATH, PATH);
		}
		else if(choice_dir == "q")//выход из SERVER_FILE_SYSTEM 
		{
			process_dir = false;	
		}
		else if(choice_dir == "t")
		{
			GET_OBJ = PATH;
			process_dir = false;
		}
		else if(choice_dir == "help")//справка о командах
		{
			std::cout << "u   - вверх по директории" << "\n"
				  << "q    - завершение работы / выход" << "\n"
				  << "help - помощь / справка" << "\n"
				  << "t - выбор текущей директории для отправки" << "\n"
				  << "d  - просмотр текущей библиотеки" << "\n";
		}
		else//альтернатива - цифры прогулка по древу
		{
			//буфер цифр
			int num;
			//проверка на цифры
			if(StringToInt(choice_dir, num) && num >= 0 && num < V_PATH.size())
			{
				//информирование о выбранном обьекте
				std::cout << "Выбран обьект: " << V_PATH[num].path().filename() << "\n\n";

				//диретория ли это
				if(V_PATH[num].is_directory())
				{
					PATH = V_PATH[num].path().string();
					DIRGET_CLIENT(V_PATH,PATH);
					WATCH_CLIENT(V_PATH, PATH);
				}
				else //файл как обьект передачи
				{
					
					std::cout << "Вы уверенны? y/n " << "\n";
					char fs;
					std::cin >> fs;
					if(fs == 'y')
					{
						GET_OBJ = V_PATH[num].path().string();
						process_dir = false;
					}
					else
					{
						WATCH_CLIENT(V_PATH, PATH);
					}
				}
			}
			else //ошибка ввода
			{
				std::cout << "ОШИБКА_ВВОДА" << "\n";
			}
		}	
	}
}

void ADMIN(std::vector<FTP_CONFIG>& VECTOR)
{
	bool process{true};    //на выбор действия
	bool first_time{true}; //на профиль
	std::string choice_dir;//буффер ввода
	int choice;
	CURLcode res;

	//обьект отпраки
	std::string CLIENT_OBJ{"отсутсвие"};
	FILE_INFO SERVER_OBJ; 
	SERVER_OBJ.PATH = "отсутсвие";

	FTP_CONFIG* T_FTP = nullptr;

	while(process)
	{
		//варианты
		help();
		std::cout << "Введите действеие: ";
		while (!(std::cin >> choice))
		{
    			std::cin.clear(); // сброс флага ошибки
   			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Введите действеие: ";
		}

		switch(choice)
		{
			case(1):
				CHOICE_PROFILE(VECTOR,T_FTP,first_time);
				break;
			case(2):
				NEW_PROFILE(VECTOR);
				break;
			case(3):
				if(!first_time)
				{
					SERVER_FILE_SYSTEM(*T_FTP,SERVER_OBJ.PATH,SERVER_OBJ.type);			
				}
				else
				{
					std::cout<<"Профиль не выбран"<<"\n";
				}
				break;
			case(4):
				if(!first_time)
				{
					CLIENT_FILE_SYSTEM(CLIENT_OBJ);			
				}	
				else
				{
					std::cout<<"Профиль не выбран"<<"\n";
				}
				break;
			case(5):
				res = T_FTP->UPLOAD_PROCESS(CHANGE_PATH(CLIENT_OBJ),SERVER_OBJ);
				OK_CONNECTION(res);

				break;
			case(6):
				res = T_FTP->DOWNLOAD_PROCESS(CHANGE_PATH(CLIENT_OBJ),SERVER_OBJ);
				OK_CONNECTION(res);	
				break;
			case(7):
				if(!first_time)
				{	
					std::cout << "\nСТАТУС об обьекьектах SERVER & CLIENT & PROFILE" << "\n";
					std::cout << "IP: "     << T_FTP->URL << "\n";
					std::cout << "SERVER: " << SERVER_OBJ.PATH << "\n";
					std::cout << "CLIENT: " << CLIENT_OBJ << "\n\n";
					//T_FTP->PROSTO();
				}
				else
				{
					std::cout << "Профиль не выбран" << "\n";
				}
				break;
			case(8):
				process = false;
				break;
			case(9):
				DELETE_PROFILE(VECTOR, T_FTP, first_time);
				break;
			default:
				std::cout << "Ошибка ввода";
				break;
		}
		
		WAIT();
		clearConsole();
	}
}
