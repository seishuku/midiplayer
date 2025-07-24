cmake_minimum_required (VERSION 3.8)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

project("midiplayer" LANGUAGES C)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(SDL_TEST OFF)

add_subdirectory(deps/SDL)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_executable(${CMAKE_PROJECT_NAME} "main.c")

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC SDL2-static SDL2main)
