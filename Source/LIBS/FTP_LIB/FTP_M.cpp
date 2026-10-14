#include <fstream>  
#include "FTP_LIB.h" 
#include <string>
#include <iostream>
#include <curl/curl.h>
//#include <chrono>
//#include <thread>


CURLMcode FTP_CONFIG::MUPLOAD_DIR(std::vector<std::filesystem::directory_entry> VP, std::string PATH_DIR_SERVER)
{
	//создание переменных для работы
	UPLOAD_TASK* mass_easy = new UPLOAD_TASK [VP.size()];
	int num  {0};
	int from {0};
	int until{0};
	CURLMcode mer = CURLM_OK;
	CURLcode easy_error = CURLE_OK;

	//Цикл по 5
	while(until < VP.size())
	{
		until += 5;
		from = num;
		
		//Цикл заполнения в мулти
		while(num < VP.size() && num < until)
		{
			//Директория
			if(VP[num].is_directory())
			{		
				std::string PDPC = CHANGE_PATH(VP[num].path().string()); 
	
				//инициализация
				mass_easy[num].curl = curl_easy_init();

				//создание дир операции
				//Клиент - обработка директори -- обьект списка 
				//Сервер - тотже путь указывающий на нахождение где будут обьекта списка включая тот что сейчас поставят
				this->CREATE_SDIRm(PDPC, PATH_DIR_SERVER, mass_easy[num]);	
			}
			else
			{
				//Клиент - файл
				//Сервер - перестраивает свой путь чтобы вплести имя файла от клиента
				std::string PFPC = CHANGE_PATH(VP[num].path().string());
				std::string PFSE = BUILD_UPLOAD_PATH(PFPC, PATH_DIR_SERVER);

				//инициализация
				mass_easy[num].curl = curl_easy_init();

				//создание файловой операции
				this->UPLOAD_FILEm(PFPC, PFSE, mass_easy[num]);

				//проверка на файл(открытие)
				if(!mass_easy[num].flag)
				{
					//очистка при неудаче открытия
					CLEANM_UP(mass_easy,VP.size());
					return CURLM_INTERNAL_ERROR;
				}
				//VP.erase(VP.begin() + num);				
			}

			//добовление в кластер мулти
			curl_multi_add_handle(multi, mass_easy[num].curl);
			num++;		
		}

		//std::cout << "#2 - up" << "\n";

		int run = 0;

		//запуск мулти
		mer = curl_multi_perform(multi, &run);
		if (mer != CURLM_OK)
		{
    			std::cout << "curl_multi_perform error: " << curl_multi_strerror(mer) << "\n";
		}

		while (run && mer == CURLM_OK)
		{
    			int numfds = 0;
    			mer = curl_multi_wait(multi, nullptr, 0, 1000, &numfds);
    			if (mer != CURLM_OK) break;
    			mer = curl_multi_perform(multi, &run);
		}

		// обработка завершившихся хендлов
		CURLMsg* msg;
		int msgs_left = 0;

		while ((msg = curl_multi_info_read(multi, &msgs_left)) != nullptr && msgs_left > 0)
		{
    			// завершён один из объектов
    			if (msg->msg == CURLMSG_DONE)
    			{
				long code = 0;
				CURLcode res = msg->data.result;
        			// есть ли ошибка по завершению
        			if(res != CURLE_OK)
        			{
					if(res == CURLE_QUOTE_ERROR && code == 550)
					{
            					easy_error = msg->data.result;
            					std::cout << curl_easy_strerror(msg->data.result) << "\n";
					}
        			}

        			// какой завершился
        			CURL* fin = msg->easy_handle;
        			int f_num = from;

        			// поиск
        			while (f_num < VP.size() && f_num < until)
				{
            				// нашли
           				if (mass_easy[f_num].curl == fin)
            				{
                				// имеется файл -> закрытие его
                				if (mass_easy[f_num].file != nullptr)
                				{
                    					mass_easy[f_num].file->close();
                    					delete mass_easy[f_num].file;
                    					mass_easy[f_num].file = nullptr;
                				}

						if (mass_easy[f_num].command != nullptr)
                				{	
							//отвязка
							curl_easy_setopt(mass_easy[f_num].curl, CURLOPT_QUOTE, nullptr);
							//закрыли команду - очистка содержимого
                    					curl_slist_free_all(mass_easy[f_num].command);
							//очистили указатель
                    					mass_easy[f_num].command = nullptr;
                				}

                				// освобождение завершившегося
                				curl_multi_remove_handle(multi, fin);
                				//curl_easy_cleanup(fin);
                				//mass_easy[f_num].curl = nullptr;

                				break; // нашли — дальше искать не нужно
            				}
        				f_num++;
				}
			}
        
    		}
		
	}
	
	// обработка завершившихся хендлов
	CURLMsg* msg;
	int msgs_left = 0;

	while ((msg = curl_multi_info_read(multi, &msgs_left)) != nullptr && msgs_left > 0)
	{
    		// завершён один из объектов
    		if (msg->msg == CURLMSG_DONE)
    		{
			// есть ли ошибка по завершению
        		if (msg->data.result != CURLE_OK)
        		{
            			easy_error = msg->data.result;
        		}

        		// какой завершился
        		CURL* fin = msg->easy_handle;
        		int f_num = 0;

        		// поиск
        		while (f_num < VP.size())
			{
            			// нашли
           			if (mass_easy[f_num].curl == fin)
            			{
                			// имеется файл -> закрытие его
                			if (mass_easy[f_num].file != nullptr)
                			{
						std::cout << mass_easy[f_num].file_name << " -- " << curl_easy_strerror(easy_error) << "\n";
                    				mass_easy[f_num].file->close();
                    				delete mass_easy[f_num].file;
                    				mass_easy[f_num].file = nullptr;
                			}

					if (mass_easy[f_num].command != nullptr)
                			{	
						//отвязка
						curl_easy_setopt(mass_easy[f_num].curl, CURLOPT_QUOTE, nullptr);
						//закрыли команду - очистка содержимого
                    				curl_slist_free_all(mass_easy[f_num].command);
						//очистили указатель
                    				mass_easy[f_num].command = nullptr;
                			}

                			// освобождение завершившегося
                			curl_multi_remove_handle(multi, fin);
                			//curl_easy_cleanup(fin);
                			//mass_easy[f_num].curl = nullptr;

                			break; // нашли — дальше искать не нужно
            			}
        			f_num++;
			}
		}
        
    	}


	
	//имеються ошибки-> Выход
	if(mer != CURLM_OK || easy_error != CURLE_OK)
	{
		std::cout << curl_easy_strerror(easy_error) << "\n";
		std::cout << curl_multi_strerror(mer) << "\n";
		CLEANM_UP(mass_easy,VP.size());
		return mer;
	}
	
	//очистим все равно же использовано
	CLEANM_UP(mass_easy,VP.size());
	
	//std::cout << "#4 - up" << "\n";	

	//рекурсия и продолжение передачи по дир
	num = 0;
	std::vector<std::filesystem::directory_entry> VP_copy;	
	while(num < VP.size())
	{
		if(mer == CURLM_OK)
		{
			if(VP[num].is_directory())
			{	
				VP_copy.clear();

				std::string PDPC = CHANGE_PATH(VP[num].path().string());
				//std::cout << "MUPLOAD-for_new(PDPC)= "<< PDPC << "\n";
				DIRGET_CLIENT(VP_copy, CHANGE_PATH(PDPC));
				std::string PDSE = BUILD_UPLOAD_PATH(PDPC, PATH_DIR_SERVER);
				//std::cout << "MUPLOAD-for_new(PDSE)= "<< PDSE << "\n";
				mer = this->MUPLOAD_DIR(VP_copy,PDSE);
			}
		}
		else
		{
			CLEANM_UP(mass_easy,VP.size());
			return mer;
		}
		num++;
	}

	//std::cout << "#5 - up" << "\n";	
	
	CLEANM_UP(mass_easy,VP.size());
	return mer;
}

CURLMcode FTP_CONFIG::MDOWNLOAD_DIR(std::vector <FILE_INFO> VP,std::string C_OBJ)
{ 
	DOWNLOAD_TASK* mass_easy = new DOWNLOAD_TASK [VP.size()];
	int from{0};
	int until{0};
	int num{0};
	CURLMcode mer = CURLM_OK;
	CURLcode easy_error = CURLE_OK;

	//общий цикл на выполнение
	while(until < VP.size())
	{
		from = num;
		until += 5;
		int m;
		//цикл на 5 обьектов за раз
		while(num < VP.size() && num < until)
		{
			if(VP[num].type == 'd')
			{
				//Клиент - в нашу дир по текущему пути клиента создаем дир из списка
				//Сервер - Обьект из списка, являющ директорией
				std::cout << CHANGE_PATH(C_OBJ) << "\n";
				std::cout << VP[num].PATH << "\n";
				std::cin >> m;
				if(!this->CREATE_CDIRm(CHANGE_PATH(C_OBJ),VP[num].PATH))
				{
					//При ошибке очищаем
					CLEANM_DOWN(mass_easy,VP.size());
					return CURLM_INTERNAL_ERROR;
				}
			}
			else
			{
				//создаем обьект	
				mass_easy[num].curl = curl_easy_init();

				//Клиент - построение старый путь клиента + имя файла
				//Сервер - обьект файл из списка
				//элемент - download_task для коректного закрытия файла и соединения
				std::cout << BUILD_DOWNLOAD_PATH(CHANGE_PATH(C_OBJ),VP[num].PATH) << "\n";
				std::cout << VP[num].PATH << "\n";
				std::cin >> m;
				this->DOWNLOAD_FILEm(BUILD_DOWNLOAD_PATH(CHANGE_PATH(C_OBJ),VP[num].PATH), VP[num].PATH, mass_easy[num]);
			
				//проверка на открытие файла 
				//Вопрос на заметку -- можно ли продолжать проигнорив его но вывев о сбое в нем?
				if(!mass_easy[num].flag)
				{
					CLEANM_DOWN(mass_easy,VP.size());
					return CURLM_INTERNAL_ERROR;
				}

				//добовляем в общий кластер запросов
				curl_multi_add_handle(multi, mass_easy[num].curl);
				
				//избавляемся от файла чтобы не беспокоиться в конце в поиске дир
				//VP.erase(VP.begin() + num);
			}

			num++;	
		}

		//Запуск мулти -- СТАРТ
		int run{0};
		mer = curl_multi_perform(multi, &run);
		if (mer != CURLM_OK)
		{
    			std::cout << "curl_multi_perform error: " << curl_multi_strerror(mer) << "\n";
		}
		
		//ВЫПОЛНЕНИЕ
		while (run && mer == CURLM_OK)
		{
    			int numfds = 0;
    			mer = curl_multi_wait(multi, nullptr, 0, 1000, &numfds);
    			if (mer != CURLM_OK) break;
    				mer = curl_multi_perform(multi, &run);
		}

		//ОБРАБОТКА завершившихся хендлов
		CURLMsg* msg;
		int msgs_left = 0;

		while ((msg = curl_multi_info_read(multi, &msgs_left)) != nullptr && msgs_left > 0)
		{
    			// завершён один из объектов
    			if (msg->msg == CURLMSG_DONE)
    			{
        			// есть ли ошибка по завершению
        			if (msg->data.result != CURLE_OK)
        			{
            				easy_error = msg->data.result;
        			}

        			// какой завершился
        			CURL* fin = msg->easy_handle;
       				int f_num = from;

        			// поиск
        			while (f_num < VP.size() && f_num < until)
        			{
            				// нашли
           				if (mass_easy[f_num].curl == fin)
            				{
                				// имеется файл -> закрытие его
                				if (mass_easy[f_num].file != nullptr)
                				{
							//такой то завершился
							std::cout << mass_easy[f_num].file_name << " -- " << curl_easy_strerror(easy_error) << "\n";
							//закрыли файл
                    					mass_easy[f_num].file->close();
							//удалили открытие файла
                    					delete mass_easy[f_num].file;
							//очистили указатель
                    					mass_easy[f_num].file = nullptr;
                				}
							
                				// освобождение завершившегося из мулти
                				curl_multi_remove_handle(multi, fin);
						//очистили обьект
                				//curl_easy_cleanup(fin);
						//удалалили указатель
                				//mass_easy[f_num].curl = nullptr;

                				break; // нашли — дальше искать не нужно
            				}
					//счет
        				f_num++;
				}
        
    			}
		}
	}

	//ПОВТОРНЫЙ РАЗБОР - ФИНАЛЬНЫЙ 
	CURLMsg* msg;
	int msgs_left = 0;
	while ((msg = curl_multi_info_read(multi, &msgs_left)) != nullptr && msgs_left > 0)
	{
    		// завершён один из объектов
    		if (msg->msg == CURLMSG_DONE)
    		{
        		// есть ли ошибка по завершению
        		if (msg->data.result != CURLE_OK)
        		{
            			easy_error = msg->data.result;
            			std::cout << curl_easy_strerror(msg->data.result) << "\n";
        		}

        		// какой завершился
        		CURL* fin = msg->easy_handle;
        		int f_num = 0;

        		// поиск
        		while (f_num < VP.size())
			{
            			// нашли
           			if (mass_easy[f_num].curl == fin)
            			{
                			// имеется файл -> закрытие его
                			if (mass_easy[f_num].file != nullptr)
                			{
                    				mass_easy[f_num].file->close();
                    				delete mass_easy[f_num].file;
                    				mass_easy[f_num].file = nullptr;
                			}

                			// освобождение завершившегося
                			curl_multi_remove_handle(multi, fin);
                			//curl_easy_cleanup(fin);
                			//mass_easy[f_num].curl = nullptr;

                			break; // нашли — дальше искать не нужно
            			}
        			f_num++;
			}
		}
        
    	}

	//ЕСЛИ ЕСТЬ ОШИБКА КАКАЯ ЛИ ТО ВЫРУБАЕМ ВСЕ 
	if(mer != CURLM_OK || easy_error != CURLE_OK)
	{
		std::cout << curl_easy_strerror(easy_error) << "\n";
		std::cout << curl_multi_strerror(mer) << "\n";
		CLEANM_DOWN(mass_easy,VP.size());
		return mer;
	}
	
	//очитска все равно оно больше не нужно
	CLEANM_DOWN(mass_easy,VP.size());
	
	
	//Начинаем выявлять новые директории из нашего списка
	num = 0;
	std::vector <FILE_INFO> VP_copy; //Создание нового списка	

	//цикл перебора обьектов в списке
	while(num < VP.size())
	{	
		//проверка на проебы след реврерсивных MDOWNLOAD_DIR
		if(mer == CURLM_OK)
		{
			if(VP[num].type == 'd')
			{
				//Очистка прошлого списка
				VP_copy.clear();
				//Создание нового
				if(this->DIRGET(VP_copy, VP[num].PATH) != CURLE_OK)
				{
					CLEANM_DOWN(mass_easy,VP.size());
					return CURLM_INTERNAL_ERROR;
				}
			
				//Клиент - указывает путем на дир в которой будет в будущем список
				//Сервер - указывает откуда брать данные списка
				//Список - список обьектов списка
				mer = this->MDOWNLOAD_DIR(VP_copy,BUILD_DOWNLOAD_PATH(CHANGE_PATH(C_OBJ), VP[num].PATH));
			}
		}
		else
		{
			CLEANM_DOWN(mass_easy,VP.size());
			return mer;
		}
		num++;
	}

	CLEANM_DOWN(mass_easy,VP.size());
	return mer;

	//Итог вопрос нахер нужен S_OBJ если вся работа и функции лежат на список
}



//мусорка - мусоросжигатель - крEматорий
void CLEANM_UP(UPLOAD_TASK*& mass_easy, int size)
{
	if(mass_easy != nullptr)
	{
		int num{0};
		while(num < size)
		{
			//очистка файла если есть
			if(mass_easy[num].file != nullptr)
			{
				if(mass_easy[num].file->is_open())
					mass_easy[num].file->close();
				delete mass_easy[num].file;
				mass_easy[num].file = nullptr;

			}

			//очистка команды для создании дир если есть
			if (mass_easy[num].command != nullptr)
                	{	
				//отвязка
				curl_easy_setopt(mass_easy[num].curl, CURLOPT_QUOTE, nullptr);
				//закрыли команду - очистка содержимого
                    		curl_slist_free_all(mass_easy[num].command);
				//очистили указатель
                    		mass_easy[num].command = nullptr;
                	}

			if (mass_easy[num].curl != nullptr)
			{
				curl_easy_cleanup(mass_easy[num].curl);
				mass_easy[num].curl = nullptr;
			}
			num++;
		}

		delete[] mass_easy;
		mass_easy = nullptr;
	}
}

void CLEANM_DOWN(DOWNLOAD_TASK*& mass_easy, int size)
{
	if(mass_easy != nullptr)
	{
		int num{0};
		while(num < size)
		{
			if(mass_easy[num].file != nullptr)
			{
				if(mass_easy[num].file->is_open())
					mass_easy[num].file->close();
				delete mass_easy[num].file;
				mass_easy[num].file = nullptr;

			}

			if (mass_easy[num].curl != nullptr)
			{
				curl_easy_cleanup(mass_easy[num].curl);
				mass_easy[num].curl = nullptr;
			}

			num++;
		}

		delete[] mass_easy;
		mass_easy = nullptr;
	}
}

