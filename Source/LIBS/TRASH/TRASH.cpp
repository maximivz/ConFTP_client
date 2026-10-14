/*
CURLcode FTP_CONFIG::CREATE_SDIR(std::string PATH_DIR_PC, std::string PATH_DIR_SERVER)
{
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

	//очиска 
	curl_slist_free_all(command);
	curl_easy_setopt(curl, CURLOPT_QUOTE, nullptr);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
	curl_easy_reset(curl);

	//std::cout << "FTP RESPONSE: " << ftp_code << "\n";

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

		int num{0};
		
		while(num < VP.size())
		{
			//CHANGE_PATH(CLIENT_OBJ)
			if(res != CURLE_OK) {break;}
			if(VP[num].is_directory())
			{	
				std::string PDPC = CHANGE_PATH(VP[num].path().string()); 
				std::string PDSE = (PATH_DIR_SERVER + GET_FILENAME(PATH_DIR_PC) + "/");
				
				

				res = this->CREATE_SDIR(PDPC, PDSE); 
			}
			else
			{
				std::string PFPC = CHANGE_PATH(VP[num].path().string());
				std::string PFSE = CHANGE_PATH(BUILD_UPLOAD_PATH(PFPC, (PATH_DIR_SERVER + GET_FILENAME(PATH_DIR_PC))));
				
				

				res = this->UPLOAD_FILE(PFPC, PFSE);	
				if(res == CURLE_OK)
					std::cout << "Загружено: " << GET_FILENAME(PFPC) << "\n";
				
			}
			num++;
		}
	}

	return res;
}
*/

/*
bool FTP_CONFIG::CREATE_CDIR(std::string PATH_DIR_PC, FILE_INFO S_OBJ)
{
	//создание дир
	CURLcode res = CURLE_OK;
	bool flag{true};
	
	PATH_DIR_PC = PATH_DIR_PC + GET_FILENAME(S_OBJ.PATH);


	if(!std::filesystem::exists(PATH_DIR_PC))
	{
		if(!std::filesystem::create_directory(PATH_DIR_PC))
		{
			return false;
		}
	}
	
	int num{0};
	std::vector <FILE_INFO> VD;
	this->DIRGET(VD, S_OBJ.PATH);

	while(num < VD.size())
	{
		if(res == CURLE_OK)
		{
			if(VD[num].type == 'd')
			{
				//std::cout << "клиент - путь для дир создания" << CHANGE_PATH(PATH_DIR_PC + "/") << "\n";
				//std::cout << "создание  - путь для дир создания" << VD[num].PATH << "\n";

				flag = this->CREATE_CDIR(CHANGE_PATH(PATH_DIR_PC + "/"),VD[num]);
				if(flag)
				{
					res = CURLE_OK;
				}
				else
				{
					res = CURLE_WRITE_ERROR;
				}

			}
			else
			{
				//std::cout << "клиент - путь для файл создания" << BUILD_DOWNLOAD_PATH(CHANGE_PATH(PATH_DIR_PC),VD[num].PATH) << "\n";
				//std::cout << "создание - путь для файл создания" <<  VD[num].PATH << "\n";

				res = this->DOWNLOAD_FILE(BUILD_DOWNLOAD_PATH(CHANGE_PATH(PATH_DIR_PC),VD[num].PATH), VD[num].PATH);
				if(res == CURLE_OK)
					std::cout << "Загружено: " << VD[num].name << "\n";
			}
		}
		else
		{
			return false;
		}
		num++;
	}
	return flag;
}
*/