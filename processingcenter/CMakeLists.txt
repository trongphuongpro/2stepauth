cmake_minimum_required(VERSION 3.0)

#-----------------------------------------------------------------------------#
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CXX_COMPILER arm-linux-g++)
set(CMAKE_C_COMPILER arm-linux-gcc)
#-----------------------------------------------------------------------------#

set(TARGET main)

add_subdirectory($ENV{HOME}/Work/eLinux/BBB/libraries/message message)

add_executable(${TARGET} src/main.cpp)

target_link_libraries(${TARGET} message)
target_include_directories(${TARGET} PUBLIC include)

#-----------------------------------------------------------------------------#
target_compile_options(${TARGET} PUBLIC -Wall
										-Werror
										-O2
)
#-----------------------------------------------------------------------------#
install(TARGETS ${TARGET} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)