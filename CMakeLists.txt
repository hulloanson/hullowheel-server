cmake_minimum_required(VERSION 2.6)
project(VWheel)

include_directories("/usr/include/")

link_libraries("explain")

aux_source_directory("./src" SOURCES)
add_executable("./bin/VWheel" ${SOURCES})

