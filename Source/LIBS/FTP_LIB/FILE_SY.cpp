#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include "FTP_LIB.h"
#include <chrono>
#include <iomanip>
#include <sstream>


void DIRGET_CLIENT(std::vector<std::filesystem::directory_entry>& VP,std::string path)
{
	std::filesystem::path p = path;
	VP.clear();

	for(const auto& entry : std::filesystem::directory_iterator(p))
	{
		VP.push_back(entry);
	}
}

std::string UP_PATH(std::string path)
{
    return std::filesystem::path(path).parent_path().string();
}


std::string GET_ROOT()
{
	std::string ROOT;
	std::cout << "Введите корневую папку: " << "\n";
	std::cin  >> ROOT;
	while(!std::filesystem::exists(ROOT) && std::filesystem::is_directory(ROOT))
	{
		std::cout << "Не существует" << "\n";
		std::cout << "Ввод: " << "\n";
	}
	return ROOT;
}

std::string GET_FILE_TIME(const std::filesystem::directory_entry& entry)
{
    auto ftime = entry.last_write_time();

    // перевод filesystem time -> system_clock time_point (через duration)
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - std::filesystem::file_time_type::clock::now()
        + std::chrono::system_clock::now()
    );

    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&tt), "%d.%m.%Y %H:%M:%S");

    return oss.str();
}


