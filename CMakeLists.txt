cmake_minimum_required(VERSION 2.8)
project(nfv_exam)

set (BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set (EXECUTABLE_OUTPUT_PATH "${BASE_PATH}/bin")

MESSAGE (STATUS "${CMAKE_GENERATOR}")

#specify CMAKE_BUILD_TYPE to Debug or Release
SET (CMAKE_C_FLAGS "-Wall -g")

SET (CMAKE_CXX_FLAGS "-std=c++11 -Wall -g")

include_directories(${BASE_PATH}/include)

file(GLOB src "src/*.cpp" "src/*.c")

add_executable(${PROJECT_NAME} ${src})

#SET (SYS_LIB dl rt)

target_link_libraries(${PROJECT_NAME} ${SYS_LIB})
