#pragma once
#include <cstddef>
#include "FTP_LIB.h"
//#include "FILE_LIB.hpp"
#include "filesystem"

void WATCH_DIR(const FILE_INFO VECT, std::string URL);

//СПИСОК ПРОФИЛЕЙ
void LISTS_PROFILES(const std::vector<FTP_CONFIG> VECTOR);

//НОВЫЙ ПРОФИЛЬ
void NEW_PROFILE(std::vector<FTP_CONFIG>& VECTOR);

void SERVER_FILE_SYSTEM(FTP_CONFIG& T_FTP,std::string& GET_OBJ,bool& Tobj);

void WATCH_CLIENT(std::vector<std::filesystem::directory_entry>& VP, std::string PATH);

void CLIENT_FILE_SYSTEM(std::string& GET_OBJ);

void CHOICE_PROFILE(std::vector<FTP_CONFIG>& VECTOR,FTP_CONFIG*& T_FTP, bool& first_time);
void DELETE_PROFILE(std::vector<FTP_CONFIG>& VECTOR, FTP_CONFIG* T_FTP, bool& first_time);
void help();

void WAIT();

void clearConsole();

void ADMIN(std::vector<FTP_CONFIG>& VECTOR);


