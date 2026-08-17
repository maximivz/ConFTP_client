#include <fstream>  
#include "FTP_LIB.h" 
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <iomanip>

//конструктор
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

//деконструктор
FTP_CONFIG::~FTP_CONFIG()
{
	if(ACTIVATE)
	{
		if(curl)
		{
			curl_easy_cleanup(curl);
			curl_multi_cleanup(multi);
		}
		curl_global_cleanup();
	}
}

//тестовый запуск
CURLcode FTP_CONFIG::TEST()
{
	if(!curl) 
		return CURLE_FAILED_INIT;	
	
	CURL* curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());	
	curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());

	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);

	curl_easy_reset(curl);
	curl_easy_cleanup(curl);

	return res;
}

//активация профил
CURLcode FTP_CONFIG::ACTIVATION_PROFILE()
{
	curl_global_init(CURL_GLOBAL_ALL);
	multi = curl_multi_init();
	curl = curl_easy_init();
	CURLcode res;

	if(!curl)
	{ 
		return CURLE_FAILED_INIT;	
	}

	curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());	
	curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());
	res = this->TEST();
	curl = curl_easy_init();
		
	if(res == CURLE_OK)
	{
		ACTIVATE = true;
	}
	else
	{
		if(curl)
		{
			curl_easy_cleanup(curl);
			curl_multi_cleanup(multi);
		}
		curl_global_cleanup();
		ACTIVATE = false;		
	}
	
	return res;
}

void FTP_CONFIG::DEACTIVATION_PROFILE()
{
	if(ACTIVATE)
	{
		if(curl)
		{
			curl_easy_cleanup(curl);
			curl_multi_cleanup(multi);
		}
		curl_global_cleanup();
	}
	ACTIVATE = false;	
}

CURLcode FTP_CONFIG::DOWNLOAD_PROCESS(std::string C_OBJ, FILE_INFO S_OBJ)
{
	CURLcode res;
	bool flag{true};

	if(S_OBJ.type == 'd')
	{	
		if(!std::filesystem::exists(C_OBJ + GET_FILENAME(S_OBJ.PATH)))
		{
			if(!std::filesystem::create_directory(C_OBJ + GET_FILENAME(S_OBJ.PATH)))
			{
				flag = false;
			}
		}

		if(flag)
		{
			res = CURLE_OK;
		}
		else
		{
			res = CURLE_URL_MALFORMAT;
		}
		
		int num{0};
		std::vector <FILE_INFO> VD;
		if(this->DIRGET(VD, S_OBJ.PATH) != CURLE_OK)
			return CURLE_URL_MALFORMAT;
		
		if(this->MDOWNLOAD_DIR(VD,C_OBJ,S_OBJ.PATH) == CURLM_OK)
			return CURLE_OK;
		else
			return CURLE_URL_MALFORMAT;

	}
	else
	{	
		res = this->DOWNLOAD_FILE(BUILD_DOWNLOAD_PATH(C_OBJ,S_OBJ.PATH), S_OBJ.PATH);
	}

	return res;	
}

CURLcode FTP_CONFIG::UPLOAD_PROCESS(std::string C_OBJ, FILE_INFO S_OBJ)
{
	CURLcode res;	

	std::filesystem::directory_entry entry (C_OBJ);
	if(entry.is_directory())
	{
		std::string PATH_DIR_SERVER = S_OBJ.PATH;
		std::string PATH_DIR_PC = entry.path().string();

		std::vector<std::filesystem::directory_entry> VP;

		//имя и пароль
		curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());

		//адресс URL
		std::string CURL_URL = (this->URL + PATH_DIR_SERVER);
		curl_easy_setopt(curl, CURLOPT_URL, CURL_URL.c_str());
	
		//команда
		struct curl_slist* command = nullptr;
		std::string dir_name = GET_FILENAME(PATH_DIR_PC);

		//CWD
		std::string cwd = "CWD /" + PATH_DIR_SERVER;
		if(cwd.back() == '/' && !PATH_DIR_SERVER.empty())
			cwd.pop_back();
		command = curl_slist_append(command, cwd.c_str());

		//MKD
		std::string command_mkd = "MKD " + dir_name;
		command = curl_slist_append(command, command_mkd.c_str());

		curl_easy_setopt(curl, CURLOPT_QUOTE, command);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);


		//запуск
		CURLcode res = CURLE_OK;
		res = curl_easy_perform(curl);


		//обработка ошибки
		long ftp_code = 0;
    		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ftp_code);

		std::cout << "FTP RESPONSE: " << ftp_code << "\n";

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

		if(res == CURLE_OK)
		{
			DIRGET_CLIENT(VP, PATH_DIR_PC);
			std::cout << "#1" << "\n";
				
			if(this->MUPLOAD_DIR(VP,C_OBJ, S_OBJ.PATH) == CURLM_OK)
				return CURLE_OK;
			else
				return CURLE_URL_MALFORMAT;

			std::cout << "#2" << "\n";
		}

		return res;
	
	}
	else
	{
		res = this->UPLOAD_FILE(C_OBJ, BUILD_UPLOAD_PATH(C_OBJ,S_OBJ.PATH));
	}

	std::cout << "#3" << "\n";

	return res;
}


//загрузка файлов с сервера
CURLcode FTP_CONFIG::DOWNLOAD_FILE(const std::string PATH_FILE_PC,
				   const std::string PATH_FILE_SERVER)
{	
	//файл
	std::ofstream file(PATH_FILE_PC, std::ios::binary);
	if(!(file.is_open())) 	return CURLE_URL_MALFORMAT;

	//настройка запроса
	curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());
	curl_easy_setopt(curl, CURLOPT_URL,(this->URL + PATH_FILE_SERVER).c_str());	//адресс файла на серваке
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WRITE_CALLB_FILE);		//функция для отправки по частям
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);				//ссылка на файл
									
    	curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L);

	
	//запрос + закрытие файла
	CURLcode res = curl_easy_perform(curl);
	file.close();

	return res;
}


//загрузка на сервер
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

	CURLcode res = curl_easy_perform(curl);

	curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L);
	file.close();
	
	return res;
}

//DOWN_DIR - multi
bool FTP_CONFIG::CREATE_CDIRm(std::string PATH_DIR_PC, std::string PATH_DIR_SERVER)
{	
	PATH_DIR_PC = PATH_DIR_PC + GET_FILENAME(PATH_DIR_SERVER);

	if(!std::filesystem::exists(PATH_DIR_PC))
	{
		if(!std::filesystem::create_directory(PATH_DIR_PC))
		{
			return false;
		}
	}
	return true;
}


//UP_DIR - multi
void FTP_CONFIG::CREATE_SDIRm(std::string PATH_DIR_PC, std::string PATH_DIR_SERVER, UPLOAD_TASK& OBJ)
{
	//имя и пароль
	curl_easy_setopt(OBJ.curl, CURLOPT_USERNAME, USERNAME.c_str());
	curl_easy_setopt(OBJ.curl, CURLOPT_PASSWORD, PASSWORD.c_str());

	//адресс URL
	std::string CURL_URL = (this->URL + PATH_DIR_SERVER);
	curl_easy_setopt(OBJ.curl, CURLOPT_URL, CURL_URL.c_str());
	
	//команда:
	struct curl_slist* command = nullptr;
	std::string dir_name = GET_FILENAME(PATH_DIR_PC);

	//#1 CWD
	std::string cwd = "CWD /" + PATH_DIR_SERVER;
	if(cwd.back() == '/' && !PATH_DIR_SERVER.empty())
		cwd.pop_back();
	std::cout << cwd <<"\n";
	command = curl_slist_append(command, cwd.c_str());

	//#2 MKD
	std::string command_mkd = "MKD " + dir_name;
	std::cout << command_mkd << "\n";
	command = curl_slist_append(command, command_mkd.c_str());

	curl_easy_setopt(OBJ.curl, CURLOPT_QUOTE, command);
	curl_easy_setopt(OBJ.curl, CURLOPT_NOBODY, 1L);
}

//UP_FILE - multi
void FTP_CONFIG::UPLOAD_FILEm(const std::string PATH_FILE_PC,
                              const std::string PATH_FILE_SERVER, UPLOAD_TASK& OBJ)
{
	std::cout << PATH_FILE_PC << "\n";
	std::cout << this->URL + PATH_FILE_SERVER << "\n";

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
