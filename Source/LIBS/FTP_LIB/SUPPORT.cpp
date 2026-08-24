#include "FTP_LIB.h"
#include <iostream>
#include <string>
#include <fstream>   
#include <curl/curl.h>
#include <algorithm> 

//очистка от \r и \n
void CLEAN_STRING(std::string& str)
{
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

//изменение пути
std::string CHANGE_PATH(std::string path)
{
    //if(path == "*")
    //	return "";

    std::replace(
        path.begin(),
        path.end(),
        '\\',
        '/'
    );

    return path;
}

//получение имени файла
std::string GET_FILENAME(std::string path)
{
	if (!path.empty() && path.back() == '/')
	{
    		path.pop_back();
	}

	size_t pos = path.rfind('/');

	if (pos == std::string::npos)
		return path;

	return path.substr(pos + 1);
}


//проверка результата вывода
bool OK_CONNECTION(CURLcode MESSAGE)
{
	if(MESSAGE == CURLE_OK)
	{
		return true;
	}
	else
	{
		std::cout << curl_easy_strerror(MESSAGE) << "\n";		
		return false;
	}
}

//(UPLOAD) - колбэк функция для пошаговой отпраки файла на сервер
size_t READ_CALLB(char* ptr,     
		  size_t size,   
		  size_t part,	 
		  void* userdata)
{
	std::ifstream* file = static_cast<std::ifstream*>(userdata); 
	if(!(file->is_open())) 
		return CURL_READFUNC_ABORT;

	size_t max_b = size * part;

	//конец файла
	if(file->eof())
		return 0;

	file->read(ptr,max_b);
	size_t bytes_read = file->gcount();

	//проверка ошибок
	if(file->bad())
		return CURL_READFUNC_ABORT;

	return bytes_read;
}

//(DOWNLOAD) - FILE - колбэк функция для пошаговой отпраки файла
size_t WRITE_CALLB_FILE(void* ptr,
		     size_t size,
		     size_t part,
		     void* userdata) 
{
	std::ofstream* file = static_cast<std::ofstream*>(userdata);
	if(!(file->is_open())) 
		return CURL_READFUNC_ABORT;
	size_t bytes_write = size * part;
	file->write(static_cast<char*>(ptr), bytes_write);

	//проверка ошибок
	if(file->bad())
        	return CURL_READFUNC_ABORT;

	return bytes_write;
}

//(DOWNLOAD) - DIR - колбэк функция для пошаговой отпраки данных о директории сервера
size_t WRITE_CALLB_DIR(void* ptr, // данные с сервера
		     size_t size,
		     size_t part,
		     void* userdata)
{
	std::string* buffer = static_cast<std::string*>(userdata); //приводим к типу string
	buffer->append((char*)ptr /*const char* */,size*part /*размер в байтах*/); //в в конец добавляем 
	return size*part;
}

//постройка DOWN PATH
std::string BUILD_DOWNLOAD_PATH(std::string local_dir,
				std::string server_file)
{
	std::string out_local;
	std::string filename = GET_FILENAME(server_file);

	if (local_dir != "" && local_dir.back() == '/')
		out_local = local_dir + filename;
	else
		out_local = local_dir + "/" + filename;

    	return out_local; 
}

//постройка UP PATH
std::string BUILD_UPLOAD_PATH(std::string local_file,
			      std::string server_dir) 
{
	std::string out_server;
	std::string filename = GET_FILENAME(local_file);
	//std::cout << "#$" << local_file << "&&"<< server_dir <<"\n";

	if (server_dir != "" && server_dir.back() == '/')
		out_server = server_dir + filename;
	else
		out_server = server_dir + '/' + filename;

	//std::cout<< out_server << "\n";

	return out_server;
}

//перевод из стринг в инт
bool StringToInt(const std::string& str, int& value)
{
    	try
	{
       		size_t pos;
        	value = std::stoi(str, &pos);

		return pos == str.size();
    	}
    	catch(...)
    	{
        	return false;
    	}
}

//Замена пробелов на %20
std::string ChangeSpaces(std::string s)
{
    std::string out;
    for(char c : s)
    {
        if(c == ' ') //да
		out += "%20";
        else //нет
		out += c;
    }
    return out;
}

//Сброс пути обратно к верхней дир
std::string S_UP_PATH(std::string path)
{
 	if (!path.empty() && path.back() == '/')
		path.pop_back();

    	size_t pos = path.rfind('/');

    	if (pos != std::string::npos)
        	return path.substr(0, pos + 1);
	else
		return "";


    	return path;
}
