add_library(A_PC_LIB A_PC_LIB.cpp)

target_include_directories(A_PC_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(A_PC_LIB
	PRIVATE FTP_LIB)

target_link_libraries(A_PC_LIB
	PRIVATE FILE_LIB)
