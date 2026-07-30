#include <fstream>  
#include "FTP_LIB.h" 
#include <string>
#include <iostream>
#include <curl/curl.h>
//#include <chrono>
//#include <thread>


CURLMcode FTP_CONFIG::MUPLOAD_DIR(std::vector<std::filesystem::directory_entry> VP,std::string PATH_DIR_PC, std::string PATH_DIR_SERVER)
{
	//создание переменных для работы
	UPLOAD_TASK* mass_easy = new UPLOAD_TASK [VP.size()];
	int num{0};
	int from{0};
	int until{0};
	CURLMcode mer = CURLM_OK;
	CURLcode easy_error = CURLE_OK;

	//std::cout << "#1 - up" << "\n";
	while(until < VP.size())
	{
		until += 5;
		from = num;
		while(num < VP.size() && num < until)
		{
			if(VP[num].is_directory())
			{		
				std::string PDPC = CHANGE_PATH(VP[num].path().string()); 
				std::string PDSE = (PATH_DIR_SERVER + GET_FILENAME(PATH_DIR_PC) + "/");
		
				//std::cout << PDPC << "\n";
				//std::cout << PDSE << "\n";
		
				//инициализация
				mass_easy[num].curl = curl_easy_init();

				//создание дир операции
				this->CREATE_SDIRm(PDPC, PDSE, mass_easy[num]); 		
			}
			else
			{
				std::string PFPC = CHANGE_PATH(VP[num].path().string());
				std::string PFSE = CHANGE_PATH(BUILD_UPLOAD_PATH(PFPC, (PATH_DIR_SERVER + GET_FILENAME(PATH_DIR_PC))));

				//std::cout << PFPC << "\n";
				//std::cout << PFSE << "\n";
			
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
        			// есть ли ошибка по завершению
        			if (msg->data.result != CURLE_OK)
        			{
            				easy_error = msg->data.result;
            				std::cout << curl_easy_strerror(msg->data.result) << "\n";
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

                				// освобождение завершившегося
                				curl_multi_remove_handle(multi, fin);
                				curl_easy_cleanup(fin);
                				mass_easy[f_num].curl = nullptr;

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

                			// освобождение завершившегося
                			curl_multi_remove_handle(multi, fin);
                			curl_easy_cleanup(fin);
                			mass_easy[f_num].curl = nullptr;

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
				DIRGET_CLIENT(VP_copy, CHANGE_PATH(VP[num].path().string()));
				std::string PDPC = CHANGE_PATH(VP[num].path().string()); 
				std::string PDSE = (PATH_DIR_SERVER + GET_FILENAME(PATH_DIR_PC) + "/");

				mer = this->MUPLOAD_DIR(VP_copy,PDPC,PDSE);
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

CURLMcode FTP_CONFIG::MDOWNLOAD_DIR(std::vector <FILE_INFO> VP,std::string C_OBJ,std::string S_OBJ)
{
	C_OBJ = C_OBJ + GET_FILENAME(S_OBJ); 
	DOWNLOAD_TASK* mass_easy = new DOWNLOAD_TASK [VP.size()];
	int from{0};
	int until{0};
	int num{0};
	CURLMcode mer = CURLM_OK;
	CURLcode easy_error = CURLE_OK;

	while(until < VP.size())
	{
		from = num;
		until += 5;
		while(num < VP.size() && num < until)
		{
			if(VP[num].type == 'd')
			{
				if(!this->CREATE_CDIRm(CHANGE_PATH(C_OBJ + "/"),VP[num].PATH))
				{
					CLEANM_DOWN(mass_easy,VP.size());
					return CURLM_INTERNAL_ERROR;
				}
			}
			else
			{
			
				//std::cout << BUILD_DOWNLOAD_PATH(CHANGE_PATH(C_OBJ),VP[num].PATH) << "\n";
				//std::cout << VP[num].PATH << "\n";
				mass_easy[num].curl = curl_easy_init(); 
				this->DOWNLOAD_FILEm(BUILD_DOWNLOAD_PATH(CHANGE_PATH(C_OBJ),VP[num].PATH), VP[num].PATH, mass_easy[num]);
			
				if(!mass_easy[num].flag)
				{
					CLEANM_DOWN(mass_easy,VP.size());
					return CURLM_INTERNAL_ERROR;
				}


				curl_multi_add_handle(multi, mass_easy[num].curl);
			}

			num++;	
		}

		//запуск мулти
		int run{0};
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
							std::cout << mass_easy[f_num].file_name << " -- " << curl_easy_strerror(easy_error) << "\n";
                    					mass_easy[f_num].file->close();
                    					delete mass_easy[f_num].file;
                    					mass_easy[f_num].file = nullptr;
                				}

                				// освобождение завершившегося
                				curl_multi_remove_handle(multi, fin);
                				curl_easy_cleanup(fin);
                				mass_easy[f_num].curl = nullptr;

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
                			curl_easy_cleanup(fin);
                			mass_easy[f_num].curl = nullptr;

                			break; // нашли — дальше искать не нужно
            			}
        			f_num++;
			}
		}
        
    	}

	if(mer != CURLM_OK || easy_error != CURLE_OK)
	{
		std::cout << curl_easy_strerror(easy_error) << "\n";
		std::cout << curl_multi_strerror(mer) << "\n";
		CLEANM_DOWN(mass_easy,VP.size());
		return mer;
	}



	num = 0;
	std::vector <FILE_INFO> VP_copy;	

	while(num < VP.size())
	{	
		if(mer == CURLM_OK)
		{
			if(VP[num].type == 'd')
			{

				VP_copy.clear();
				if(this->DIRGET(VP_copy, VP[num].PATH))
				{
					CLEANM_DOWN(mass_easy,VP.size());
					return CURLM_INTERNAL_ERROR;
				}
				mer = this->MDOWNLOAD_DIR(VP_copy,CHANGE_PATH(C_OBJ + "/"),VP[num].PATH);
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
}


void FTP_CONFIG::PROSTO()
{
	UPLOAD_TASK o_c1;
	UPLOAD_TASK o_c2;

	o_c1.curl = curl_easy_init();
	o_c2.curl = curl_easy_init(); 

	std::string PFSE = "1/1.txt";
	std::string PFPC = "F:/1/1.txt";
	
	this->UPLOAD_FILEm(PFPC, PFSE, o_c1);
	this->UPLOAD_FILEm("F:/1/2.txt", "1/2.txt", o_c2);	

	curl_multi_add_handle(multi, o_c1.curl);
	curl_multi_add_handle(multi, o_c2.curl);
	
	int run{0};
	CURLMcode mer = CURLM_OK;	
	mer = curl_multi_perform(multi, &run);
	while(run)
	{
		int numfds = 0;

		curl_multi_wait(multi, nullptr, 0, 1000, &numfds);

		mer = curl_multi_perform(multi, &run);
	}

	curl_multi_remove_handle(multi,o_c1.curl);
	curl_multi_remove_handle(multi,o_c2.curl);

	o_c1.file->close();
	delete o_c1.file;
	o_c1.file = nullptr;

	o_c2.file->close();
	delete o_c2.file;
	o_c2.file = nullptr;

	curl_easy_cleanup(o_c1.curl);
	curl_easy_cleanup(o_c2.curl);
	
	if(mer != CURLM_OK)
	{
		std::cout << "bad";
	}
}

//мусорка - мусоросжигатель - крEматорий
void CLEANM_UP(UPLOAD_TASK*& mass_easy, int size)
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

		if (mass_easy[num].curl)
				curl_easy_cleanup(mass_easy[num].curl);

		num++;
	}

	delete[] mass_easy;
	mass_easy = nullptr;
}

void CLEANM_DOWN(DOWNLOAD_TASK*& mass_easy, int size)
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

		if (mass_easy[num].curl)
		{
			//std::cout << num << "\n";
			curl_easy_cleanup(mass_easy[num].curl);
		}

		num++;
	}

	delete[] mass_easy;
	mass_easy = nullptr;
}

