set(Sources
	FILE_LIB.cpp
)

add_library(FILE_LIB STATIC ${Sources})

target_include_directories(FILE_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(FILE_LIB PUBLIC FTP_LIB)
