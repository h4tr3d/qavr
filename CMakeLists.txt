project(qavr)
cmake_minimum_required(VERSION 2.6.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

if(CMAKE_CROSSCOMPILING)
  set(QT_HEADERS_DIR "/usr/i486-mingw32/include")
  set(QT_LIBRARY_DIR "/usr/i486-mingw32/lib")
endif()

find_package(Qt4 REQUIRED)

add_subdirectory(src)
add_subdirectory(data)
