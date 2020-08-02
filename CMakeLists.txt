cmake_minimum_required(VERSION 3.18)

project(mdkloader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX d)
else()
    set(CMAKE_DEBUG_POSTFIX _debug)
endif()

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
    mdkloader_global.h
    mdkloader.h
    mdkloader.cpp
)

if(WIN32 AND BUILD_SHARED_LIBS)
    enable_language(RC)
    list(APPEND LOADER_SOURCES mdkloader.rc)
endif()

add_library(libmdkloader
    ${MDK_C_HEADERS}
    ${MDK_CXX_HEADERS}
    ${LOADER_SOURCES}
)

set_target_properties(libmdkloader PROPERTIES OUTPUT_NAME MDKLoader)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(libmdkloader PUBLIC
        MDKLOADER_STATIC
    )
endif()
if(MSVC)
    target_compile_options(libmdkloader PRIVATE /utf-8)
endif()
target_compile_definitions(libmdkloader PRIVATE
    MDKLOADER_BUILD_LIBRARY
)
target_compile_definitions(libmdkloader PUBLIC
    BUILD_MDK_STATIC
)
target_include_directories(libmdkloader PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)
