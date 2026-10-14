#include <iostream> 
#include <string>
#include <curl/curl.h>

int main()
{
	//Тестовая на проверку запуска .exe
	std::string smth;
	std::cout << "Enter smth: ";
	std::cin >> smth;
	
	//CURL
	CURL* curl; //наш обьект
	CURLcode res;//для результата	    

	curl = curl_easy_init();//инициализация
	
	//не запустилось
	if(!curl)
	{
		return -1;
	}
	
	//УСТАНОВКА АДРЕССА - URL
	curl_easy_setopt(curl,CURLOPT_URL, "https://www.youtube.com/watch?v=p569dAMwQDU");
	
	//Будет ли в выводе текст http заголовка или нет - 1 или 0
	curl_easy_setopt(curl,CURLOPT_HEADER,1);

	//выполняем
	res = curl_easy_perform(curl);

	//чистим
	curl_easy_cleanup(curl);

	return 0;
}
