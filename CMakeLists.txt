cmake_minimum_required(VERSION 3.5)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/yurac/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(AsciConverter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED 23)
add_subdirectory(lib)
add_subdirectory(bin)