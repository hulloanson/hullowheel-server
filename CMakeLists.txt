cmake_minimum_required(VERSION 2.6)

project(HulloWheel C)

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories("/usr/include/")

link_libraries("explain" "pthread" "m" "z")

aux_source_directory("${CMAKE_SOURCE_DIR}/src" SOURCES)

add_executable("HulloWheel" ${SOURCES})

