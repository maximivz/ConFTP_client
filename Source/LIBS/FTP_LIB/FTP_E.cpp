#include <fstream>  
#include "FTP_LIB.h" 
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <iomanip>

//конструктор_
FTP_CONFIG::FTP_CONFIG(const std::string URL_in, 
		   const std::string USERNAME_in,
		   const std::string PASSWORD_in, 
		   const std::string CERT_PATH_in)
{
	URL = URL_in;
	USERNAME = USERNAME_in;
	PASSWORD = PASSWORD_in;
	CERT_PATH = CERT_PATH_in;
}

//деконструктор_
FTP_CONFIG::~FTP_CONFIG()
{
	if(curl)
	{
		curl_easy_cleanup(curl);	
		curl = nullptr;
	}

	if(multi)
	{
		curl_multi_cleanup(multi);
		multi = nullptr;
	}
}

//тестовый запуск_
CURLcode FTP_CONFIG::TEST()
{
	//проверка инициализация
	if(!curl) 
		return CURLE_FAILED_INIT;	

	//настройка
	curl_easy_setopt(curl, CURLOPT_URL, (URL + "/").c_str());	
	curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());

	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

	//выполнение
	CURLcode res = curl_easy_perform(curl);

	//сброс
	curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
	curl_easy_reset(curl);

	//вывод результата
	return res;
}

//активация профил_
CURLcode FTP_CONFIG::ACTIVATION_PROFILE()
{
//ИНИЦИАЛИЗАЦИЯ:
	//#1 - для изи
	//если уже есть 
	if(curl)
	{ 
		curl_easy_cleanup(curl);	
		curl = nullptr;
	}
	curl = curl_easy_init();
	//проверка
	if(!curl)
	{ 
		return CURLE_FAILED_INIT;	
	}
	
	//#2 - для multi
	//если уже есть 
	if(multi)
	{ 
		curl_multi_cleanup(multi);	
		multi = nullptr;
	}
	multi = curl_multi_init();
	//проверка
	if(!multi)
	{ 
		return CURLE_FAILED_INIT;	
	}
	//для мульти
	multi = curl_multi_init();
	
	//для разбора результата
	CURLcode res;

//ТЕСТ
	//результат
	res = this->TEST();
		

	//Сработало
	if(res == CURLE_OK)
	{
		ACTIVATE = true;
	}
	else//НЕТ
	{
		if(curl)
		{
			curl_easy_cleanup(curl);
			curl = nullptr;
		}

		if(multi)
		{
			curl_multi_cleanup(multi);
			curl = nullptr;
		}
		ACTIVATE = false;	
	}
	
	return res;
}

//деактивация_
void FTP_CONFIG::DEACTIVATION_PROFILE()
{
	if(curl)
	{
		curl_easy_cleanup(curl);
		curl = nullptr;
	}

	if(multi)
	{
		curl_multi_cleanup(multi);
		curl = nullptr;
	}	

	ACTIVATE = false;	
}

CURLcode FTP_CONFIG::DOWNLOAD_PROCESS(std::string C_OBJ, FILE_INFO S_OBJ)
{
	CURLcode res;
	bool flag{true};

	//Это директория
	if(S_OBJ.type == 'd')
	{
		//есть такая
		int m;
		std::error_code ecr;
		std::string PATH_NEW_DIR = BUILD_DOWNLOAD_PATH(C_OBJ ,S_OBJ.PATH);
		std::cout << PATH_NEW_DIR << "\n";
		std::cin >> m;

		//std::string PATH_NEW_DIR = BUILD_DOWNLOAD_PATH(C_OBJ ,GET_FILENAME(S_OBJ.PATH));
		if(!std::filesystem::exists(PATH_NEW_DIR))
		{
			//если такой нет то создаем
			if(!std::filesystem::create_directory(PATH_NEW_DIR, ecr))
			{
				flag = false;
			}

			if(ecr)
			{
				std::cerr << "ERROR_FILESYSTEM_CREATING: " << ecr.message() << "\n";
			}
		}

		//получилось
		if(flag)
		{
			res = CURLE_OK;
		}
		else //нет
		{
			return CURLE_URL_MALFORMAT;
		}
		
		std::cin >> m;
		
		//сборщик файлов для РЕКУРСИИ
		std::vector <FILE_INFO> VD;

		//Получилось собрать ?
		if(this->DIRGET(VD, S_OBJ.PATH) != CURLE_OK)
			return CURLE_URL_MALFORMAT; //нет
		
		std::cin >> m;

		//рекурсия
		//закидываем обновленный клиент путь на созданный дир и старый сервер + список новых обьектов
		if(this->MDOWNLOAD_DIR(VD,PATH_NEW_DIR) == CURLM_OK)
			res = CURLE_OK;
		else
			return CURLE_URL_MALFORMAT;

	}
	else//это файл
	{	
		res = this->DOWNLOAD_FILE(BUILD_DOWNLOAD_PATH(C_OBJ,S_OBJ.PATH), S_OBJ.PATH);
	}

	return res;	
}

struct curl_slist* CREATE_COMMAND_UCREATE_DIR( std::string PATH_DIR_SERVER, std::string PATH_DIR_PC)
{
		//команда
		struct curl_slist* command = nullptr;
		std::string dir_name = GET_FILENAME(PATH_DIR_PC);

		//MKD
		std::string command_mkd = "MKD " + PATH_DIR_SERVER + "/" + dir_name;
		//std::cout << command_mkd << "\n";
		command = curl_slist_append(command, command_mkd.c_str());
		return command;
}

CURLcode FTP_CONFIG::UPLOAD_PROCESS(std::string C_OBJ, FILE_INFO S_OBJ)
{
	CURLcode res;	

	std::filesystem::directory_entry entry (C_OBJ);

	//ЭТО директория
	if(entry.is_directory())
	{
		std::string PATH_DIR_SERVER = S_OBJ.PATH;
		std::string PATH_DIR_PC = entry.path().string();

		std::vector<std::filesystem::directory_entry> VP;

		//имя и пароль
		curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());

		//адресс URL
		std::string CURL_URL = (this->URL + PATH_DIR_SERVER + "/");
		curl_easy_setopt(curl, CURLOPT_URL, CURL_URL.c_str());
	
		//команда
		struct curl_slist* command = CREATE_COMMAND_UCREATE_DIR(PATH_DIR_SERVER, PATH_DIR_PC);
		curl_easy_setopt(curl, CURLOPT_QUOTE, command);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);


		//запуск
		CURLcode res = CURLE_OK;
		res = curl_easy_perform(curl);


		//обработка ошибки
		long ftp_code = 0;
    		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ftp_code);

		//очиска 
		curl_slist_free_all(command);
		curl_easy_setopt(curl, CURLOPT_QUOTE, nullptr);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
		curl_easy_reset(curl);

		//обработка
		if(res == CURLE_OK)
		{
    			res = CURLE_OK;
		}
		else if(ftp_code == 257 || ftp_code == 550)
		{
    			res = CURLE_OK; // папка уже есть
		}
		else
		{
    			return res;
		}

		//все ок -- ЭТАП ЗАПУКСКА ВОЛШЕБНОЙ РЕКУРСИИ
		if(res == CURLE_OK)
		{
			//получили список директорий в VP из клиента
			DIRGET_CLIENT(VP, PATH_DIR_PC);
			
			//выполняем рекурсивно и в конце проверка
			if(this->MUPLOAD_DIR(VP,BUILD_UPLOAD_PATH(C_OBJ ,S_OBJ.PATH)) == CURLM_OK)
				return CURLE_OK; //да
			else
				return CURLE_URL_MALFORMAT; //нет

		}
	}
	else //ЭТО ФАЙЛ
	{
		res = this->UPLOAD_FILE(C_OBJ, BUILD_UPLOAD_PATH(C_OBJ,S_OBJ.PATH));
	}

	//Результат
	return res;
}


//загрузка файлов с сервера_
CURLcode FTP_CONFIG::DOWNLOAD_FILE(const std::string PATH_FILE_PC,
				   const std::string PATH_FILE_SERVER)
{	
	//файл
	std::ofstream file(PATH_FILE_PC, std::ios::binary);
	if(!(file.is_open())) 	return CURLE_URL_MALFORMAT;

	//настройка запроса
	curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());
	//адресс файла на серваке
	curl_easy_setopt(curl, CURLOPT_URL,(this->URL + PATH_FILE_SERVER).c_str());
	//функция для отправки по частям
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WRITE_CALLB_FILE);
	//ссылка на файл
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L);

	
	//запрос + закрытие файла + reset
	CURLcode res = curl_easy_perform(curl);
	curl_easy_reset(curl);	
	file.close();

	return res;
}


//загрузка на сервер_
CURLcode FTP_CONFIG::UPLOAD_FILE(const std::string PATH_FILE_PC,
                                 const std::string PATH_FILE_SERVER)
{
	std::ifstream file(PATH_FILE_PC, std::ios::binary);
	if(!file.is_open()) 
	{
		return CURLE_FILE_COULDNT_READ_FILE; 
	}

	//размер файла
	file.seekg(0, std::ios::end);
	curl_off_t file_size = file.tellg();
	file.seekg(0, std::ios::beg);

	//настройка запроса
	curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());	
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_URL,(this->URL + PATH_FILE_SERVER).c_str());

	curl_easy_setopt(curl, CURLOPT_READFUNCTION, READ_CALLB);
	curl_easy_setopt(curl, CURLOPT_READDATA, &file);
	curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, file_size);

	//выполнение
	CURLcode res = curl_easy_perform(curl);

	//закрытие + сброс
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L);
	file.close();
	curl_easy_reset(curl);	
	
	return res;
}

//DOWN_DIR - multi
bool FTP_CONFIG::CREATE_CDIRm(std::string PATH_DIR_PC, std::string PATH_DIR_SERVER)
{	
	int m;
	PATH_DIR_PC = BUILD_DOWNLOAD_PATH(PATH_DIR_PC ,PATH_DIR_SERVER);
	std::cout << PATH_DIR_PC << "\n";
	std::cin >> m;
	bool flag {true};
	std::error_code ecr;
	if(PATH_DIR_PC.empty())
		return false;

	if(!std::filesystem::exists(PATH_DIR_PC))
	{
		if(!std::filesystem::create_directory(PATH_DIR_PC))
		{
			flag = false;
		}
		if(ecr)
		{
			std::cerr << "ERROR_FILESYSTEM_CREATING: " << ecr.message() << "\n";
			flag = false;
		}
	}
	return flag;
}


//UP_DIR - multi
void FTP_CONFIG::CREATE_SDIRm(std::string PATH_DIR_PC, std::string PATH_DIR_SERVER, UPLOAD_TASK& OBJ)
{
	//имя и пароль
	curl_easy_setopt(OBJ.curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(OBJ.curl, CURLOPT_PASSWORD, PASSWORD.c_str());

	//адресс URL
	//std::cout << "CREATE_SDIR = " << this->URL + PATH_DIR_SERVER + "/" << "\n";
	std::string CURL_URL = (this->URL + PATH_DIR_SERVER + "/");
	curl_easy_setopt(OBJ.curl, CURLOPT_URL, CURL_URL.c_str());
	
	//команда:
	OBJ.command = CREATE_COMMAND_UCREATE_DIR(PATH_DIR_SERVER, PATH_DIR_PC);
	
	curl_easy_setopt(OBJ.curl, CURLOPT_QUOTE, OBJ.command);
	curl_easy_setopt(OBJ.curl, CURLOPT_NOBODY, 1L);
}

//UP_FILE - multi
void FTP_CONFIG::UPLOAD_FILEm(const std::string PATH_FILE_PC,
                              const std::string PATH_FILE_SERVER, UPLOAD_TASK& OBJ)
{
	//std::cout << PATH_FILE_PC << "\n";
	//std::cout << this->URL + PATH_FILE_SERVER << "\n";
	std::cout << "CREATE_SDIR = " << PATH_FILE_PC  << "||" << this->URL + PATH_FILE_SERVER << "\n";

	OBJ.file = new std::ifstream();
	OBJ.file_name = GET_FILENAME(PATH_FILE_SERVER);
	OBJ.file->open(PATH_FILE_PC, std::ios::binary);
	if(!OBJ.file->is_open()) 
	{
		OBJ.flag = false; 
	}

	//размер файла
	OBJ.file->seekg(0, std::ios::end);
	curl_off_t file_size = OBJ.file->tellg();
	OBJ.file->seekg(0, std::ios::beg);

	//настройка запроса
	curl_easy_setopt(OBJ.curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(OBJ.curl, CURLOPT_PASSWORD, PASSWORD.c_str());	
	curl_easy_setopt(OBJ.curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(OBJ.curl, CURLOPT_URL,(this->URL + PATH_FILE_SERVER).c_str());

	curl_easy_setopt(OBJ.curl, CURLOPT_READFUNCTION, READ_CALLB);
	curl_easy_setopt(OBJ.curl, CURLOPT_READDATA, OBJ.file);
	curl_easy_setopt(OBJ.curl, CURLOPT_INFILESIZE_LARGE, file_size);
}

//DOWN_FILE - multi
void FTP_CONFIG::DOWNLOAD_FILEm(const std::string PATH_FILE_PC,
				const std::string PATH_FILE_SERVER, DOWNLOAD_TASK& OBJ)
{	
	//файл
	OBJ.file = new std::ofstream();
	OBJ.file_name = GET_FILENAME(PATH_FILE_SERVER);
	OBJ.file->open(PATH_FILE_PC, std::ios::binary);

	if(!(OBJ.file->is_open()))
	{
		OBJ.flag = false;
	}

	//настройка запроса
	curl_easy_setopt(OBJ.curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(OBJ.curl, CURLOPT_PASSWORD, PASSWORD.c_str());
	curl_easy_setopt(OBJ.curl, CURLOPT_URL,(this->URL + PATH_FILE_SERVER).c_str());	//адресс файла на серваке
	curl_easy_setopt(OBJ.curl, CURLOPT_WRITEFUNCTION, WRITE_CALLB_FILE);		//функция для отправки по частям
	curl_easy_setopt(OBJ.curl, CURLOPT_WRITEDATA, OBJ.file);				//ссылка на файл
									
    	curl_easy_setopt(OBJ.curl, CURLOPT_UPLOAD, 0L);
}
