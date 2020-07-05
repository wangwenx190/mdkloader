cmake_minimum_required(VERSION 3.18)

project(mdkloader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

set(MDK_C_HEADERS
    mdk/c/global.h
    mdk/c/MediaInfo.h
    mdk/c/Player.h
    mdk/c/RenderAPI.h
    mdk/c/VideoFrame.h
)
set(MDK_CXX_HEADERS
    mdk/global.h
    mdk/MediaInfo.h
    mdk/Player.h
    mdk/RenderAPI.h
    mdk/VideoFrame.h
)
set(LOADER_SOURCES
    mdkloader.h
    mdkloader.cpp
)

add_library(libmdkloader STATIC
    ${MDK_C_HEADERS}
    ${MDK_CXX_HEADERS}
    ${LOADER_SOURCES}
)

if(MSVC)
    target_compile_options(libmdkloader PRIVATE /utf-8)
endif()
target_compile_definitions(libmdkloader PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
)
target_compile_definitions(libmdkloader PUBLIC
    BUILD_MDK_STATIC
)
target_link_libraries(libmdkloader PRIVATE Qt::Core)
target_include_directories(libmdkloader PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)
