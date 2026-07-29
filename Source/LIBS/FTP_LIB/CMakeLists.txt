set(Sources
	DIR.cpp
	FTP_E.cpp
	FTP_M.cpp
	SUPPORT.cpp
	FILE_SY.cpp
)

add_library(FTP_LIB STATIC ${Sources})

target_include_directories(FTP_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

