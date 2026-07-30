#pragma once
#include <cstddef>
#include <curl/curl.h>
#include <vector>
#include <string>
#include <filesystem>

//--------------СТРУКТУРА
struct UPLOAD_TASK
{
	std::string file_name = "NONAME";
	CURL* curl;
	std::ifstream* file = nullptr;
	bool flag{true};
};

struct DOWNLOAD_TASK
{
	std::string file_name = "NONAME";
	CURL* curl;
	std::ofstream* file = nullptr;
	bool flag{true};
};

struct FILE_INFO
{
	//FILE/DIR/LINKS - information
	char type{'d'};//тип - директория / файл / ярлык
	std::string prava{"root"};//права 

	int links;//количество ссылок
	std::string user{"ftp-server"};//пользователи
	std::string group{"frp-server"};//група

	long size;//размер в байтах
	
	std::string month{"-"};//месяц 
	std::string day{"-"};//день
	std::string time_or_year{"-"};//год
	std::string name{"ROOT_SERVER"};//имя

	//путь к обьекту
	std::string PATH{""};
};

//--------------КЛАСС
class FTP_CONFIG
{
	public:
		std::string URL;	//Адресс URL
		std::string USERNAME;   //пользователь
		std::string PASSWORD;   //пароль
		std::string CERT_PATH;  //адрес сертификата
		CURL* curl = nullptr;   //обьект для описания запросов
		CURLM* multi = nullptr;
		bool ACTIVATE = false;
	public:
	//конструктор
	FTP_CONFIG(const std::string URL_in, 
		   const std::string USERNAME_in,
		   const std::string PASSWORD_in, 
		   const std::string CERT_PATH_in);
	//деконструктор
	~FTP_CONFIG();
	//тестовый запус
	CURLcode TEST();
	//активация профиля
	CURLcode ACTIVATION_PROFILE();
	//деактивация профиля
	void DEACTIVATION_PROFILE();
	
	//загрузка из сервера - процесс
	CURLcode DOWNLOAD_PROCESS(std::string C_OBJ, FILE_INFO S_OBJ);

	//загрузка из клиента - процесс
	CURLcode UPLOAD_PROCESS(std::string C_OBJ, FILE_INFO S_OBJ);

	//функиция загрузки данных на сервер
	CURLcode DOWNLOAD_FILE(const std::string PATH_SAVE_FILE_PC,
		           	const std::string PATH_FILE_SERVER);
	//функиция загрузки данных с сервера
	CURLcode UPLOAD_FILE(const std::string PATH_FILE_PC,
                	 	const std::string URL_FILE_SERVER);

	bool CREATE_CDIRm(std::string PATH_DIR_PC, std::string PATH_DIR_SERVER);

	void CREATE_SDIRm(std::string PATH_DIR_PC, std::string PATH_DIR_SERVER, UPLOAD_TASK& OBJ);

	void UPLOAD_FILEm(const std::string PATH_FILE_PC,
        		  const std::string PATH_FILE_SERVER, UPLOAD_TASK& OBJ);

	void DOWNLOAD_FILEm(const std::string PATH_FILE_PC,
			    const std::string PATH_FILE_SERVER, DOWNLOAD_TASK& OBJ);

	

	//получение данных о директории (сборка древа директории)
	CURLcode DIRGET(std::vector <FILE_INFO>& VD, std::string ADD_PATH);


	CURLcode CREATE_SDIR(std::string PATH_DIR_PC, std::string PATH_DIR_SERVER);
	bool CREATE_CDIR(std::string PATH_DIR_PC, FILE_INFO S_OBJ);

	

	//multi
	CURLMcode MUPLOAD_DIR(std::vector<std::filesystem::directory_entry> VP,std::string PATH_DIR_PC, std::string PATH_DIR_SERVER);
	
	CURLMcode MDOWNLOAD_DIR(std::vector <FILE_INFO> VP,std::string C_OBJ,std::string S_OBJ);

	void PROSTO();


};

//--------------ВСПОМОГАТ

//для отправки
size_t READ_CALLB(char* ptr,
		  size_t size,
		  size_t part,
		  void* userdata);
size_t WRITE_CALLB_FILE(void* ptr,
		  size_t size,
		  size_t part,
 		  void* userdata);
size_t WRITE_CALLB_DIR(void* ptr,
		  size_t size,
		  size_t part,
 		  void* userdata);

//изменение пути
std::string CHANGE_PATH(std::string);

//преобразование из string в int
bool StringToInt(const std::string& str, int& value);

//очитска имени
void CLEAN_STRING(std::string& str);

//парсить директориб
void PARSER_DIR(FILE_INFO& DIR_OBJ, std::string DIR_LINE);

//функции
std::string CHANGE_PATH(std::string path);

//получение имени
std::string GET_FILENAME(const std::string path);

//построение пути для UPLOAD
std::string BUILD_UPLOAD_PATH(std::string local_file,
                       	      std::string server_dir);

//построение пути для DOWNLOAD
std::string BUILD_DOWNLOAD_PATH(std::string local_dir,
				std::string server_file);

//построение дериктории клиента
void DIRGET_CLIENT(std::vector<std::filesystem::directory_entry>& VP,std::string path);

std::string UP_PATH(std::string path);

std::string GET_ROOT();

std::string GET_FILE_TIME(const std::filesystem::directory_entry& entry);

std::string EncodeSpaces(std::string s);

std::string S_UP_PATH(std::string path);

//очистка TASKS_ARRAY
void CLEANM_UP(UPLOAD_TASK*& mass_easy, int size);

void CLEANM_DOWN(DOWNLOAD_TASK*& mass_easy, int size);

