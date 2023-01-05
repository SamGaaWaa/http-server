cmake_minimum_required(VERSION 3.20)
project(http)

aux_source_directory(./src SOURCES)


add_library(http OBJECT ${SOURCES})
target_include_directories(http PUBLIC ./)

