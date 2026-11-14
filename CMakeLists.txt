# Указываем минимальную версию
cmake_minimum_required(VERSION 4.0)

# Имя проекта
project(SERVER)

#для выода exe
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
#-------------------------------------------------------------------
#для exe
add_executable(ConFTP 
	Source/MAIN/LAN_PR.cpp)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/LIBS/FILE_LIB)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/LIBS/FTP_LIB)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/LIBS/A_PC_LIB)
#-------------------------------------------------------------------
# _LIB|DLL_ -- линковка библиотеки
#-------------------------------------------------------------------
#линковка ститически базового для iostream
#targrret_link_options(ConFTP PRIVATE -static-libgcc -static-libstdc++)

if(WIN32)
	target_link_libraries(ConFTP
		PRIVATE ${CMAKE_SOURCE_DIR}/Source/LIBS/curl-8.18.0_2-win64-mingw/lib/libcurl.dll.a)
	target_link_libraries(FTP_LIB
		PRIVATE ${CMAKE_SOURCE_DIR}/Source/LIBS/curl-8.18.0_2-win64-mingw/lib/libcurl.dll.a)
	target_link_libraries(A_PC_LIB
		PRIVATE ${CMAKE_SOURCE_DIR}/Source/LIBS/curl-8.18.0_2-win64-mingw/lib/libcurl.dll.a)
	#копирование DLL
	add_custom_command(TARGET ConFTP POST_BUILD
    	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/Source/LIBS/curl-8.18.0_2-win64-mingw/bin
	$<TARGET_FILE_DIR:ConFTP>)

elseif(UNIX AND NOT APPLE)
	find_package(CURL REQUIRED)
	if(NOT CURL_FOUND)
		message(FATAL_ERROR "no libcurl")
	endif()
		
	target_link_libraries(ConFTP
		PRIVATE ${CURL_LIBRARIES})
	target_link_libraries(FTP_LIB
		PRIVATE ${CURL_LIBRARIES})
	target_link_libraries(A_PC_LIB
		PRIVATE ${CURL_LIBRARIES})	
endif()

target_include_directories(FILE_LIB PRIVATE ${CMAKE_SOURCE_DIR}/Source/LIBS/JSON)

target_link_libraries(ConFTP
	PRIVATE FILE_LIB)

target_link_libraries(ConFTP
	PRIVATE A_PC_LIB)

#---------------------------------------------------------------------


