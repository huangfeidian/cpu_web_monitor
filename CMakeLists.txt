cmake_minimum_required( VERSION 3.7 )
project(cpu_web_monitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)


FILE(GLOB_RECURSE SRC_LISTS ${CMAKE_SOURCE_DIR}/src/*.cpp)

# add log support
find_package(spdlog CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${spdlog_include_dirs})

find_package(nlohmann_json CONFIG REQUIRED)

add_library(cpu_web_monitor ${SRC_LISTS})
target_link_libraries(cpu_web_monitor spdlog::spdlog)

add_executable(dump_test ${CMAKE_SOURCE_DIR}/test/dump_timeline_svg.cpp)
target_link_libraries(dump_test cpu_web_monitor nlohmann_json::nlohmann_json)

