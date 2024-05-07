cmake_minimum_required(VERSION 3.5)

project(VarChannel LANGUAGES CXX C)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_library(VarChannel SHARED
    varchannel.cpp
    varchannel.h
    varcommon.h
    varcommon.c
)

target_link_libraries(VarChannel PRIVATE Qt5::Widgets)

set_target_properties(VarChannel PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

if (WIN32)
    install(TARGETS VarChannel DESTINATION ${CMAKE_BINARY_DIR}/lib)
endif()
