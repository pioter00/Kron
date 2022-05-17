cmake_minimum_required(VERSION 3.21)
project(Kron)
set(CMAKE_CXX_STANDARD 14)
set(PROJECT_NAME Kron)


add_executable(${PROJECT_NAME} main.cpp TaskList.cpp TaskList.h Task.cpp Task.h ConsoleAPI.cpp ConsoleAPI.h Logger.cpp Logger.h)
target_link_libraries(${PROJECT_NAME} pthread rt)
