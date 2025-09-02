cmake_minimum_required(VERSION 3.8)

project(HeaderHelpers LANGUAGES CXX)

add_library(HeaderHelpers INTERFACE)

target_include_directories(HeaderHelpers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)