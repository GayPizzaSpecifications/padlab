cmake_minimum_required(VERSION 3.1)
set(PROJECT analogue)
set(SOURCES analogue.c)

project(${PROJECT} C)
set(CMAKE_C_STANDARD 11)
find_package(SDL2 REQUIRED)
add_definitions(${SDL2_CFLAGS})
include_directories(${SDL2_INCLUDE_DIRS})
set(LIBRARIES ${SDL2_LIBRARIES} m)

add_executable(${PROJECT} ${SOURCES})
target_link_libraries(${PROJECT} ${LIBRARIES})
