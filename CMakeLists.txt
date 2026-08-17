# Указываем минимальную версию
cmake_minimum_required(VERSION 4.0)

# Имя проекта
project(SERVER)

#для выода exe
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
#-------------------------------------------------------------------
#для exe
add_executable(KINY 
	Source/MAIN/LAN_PR.cpp)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/LIBS/FILE_LIB)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/LIBS/FTP_LIB)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/LIBS/A_PC_LIB)
#-------------------------------------------------------------------
# _LIB|DLL_ -- линковка библиотеки
#-------------------------------------------------------------------
if(WIN32) 
	target_link_libraries(FTP_LIB
		PRIVATE ${CMAKE_SOURCE_DIR}/Source/LIBS/curl-8.18.0_2-win64-mingw/lib/libcurl.dll.a)
	target_link_libraries(A_PC_LIB
		PRIVATE ${CMAKE_SOURCE_DIR}/Source/LIBS/curl-8.18.0_2-win64-mingw/lib/libcurl.dll.a)	

elseif(UNIX AND NOT APPLE)
	find_package(CURL REQUIRED)
	if(NOT CURL_FOUND)
		message(FATAL_ERROR "no libcurl")
	endif()
		
	target_link_libraries(FTP_LIB
		PRIVATE ${CURL_LIBRARIES})
	target_link_libraries(A_PC_LIB
		PRIVATE ${CURL_LIBRARIES})
endif()

target_link_libraries(KINY
	PRIVATE FILE_LIB)

target_link_libraries(KINY
	PRIVATE A_PC_LIB)

#---------------------------------------------------------------------
#копирование DLL
add_custom_command(TARGET KINY POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Source/LIBS/curl-8.18.0_2-win64-mingw/bin
        $<TARGET_FILE_DIR:KINY>
)

