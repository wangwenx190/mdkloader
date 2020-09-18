cmake_minimum_required(VERSION 3.18)

project(MDKLoader LANGUAGES CXX)

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

add_library(${PROJECT_NAME}
    ${MDK_C_HEADERS}
    ${MDK_CXX_HEADERS}
    ${LOADER_SOURCES}
)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        MDKLOADER_STATIC
    )
endif()
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        target_compile_options(${PROJECT_NAME} PRIVATE /guard:cf)
        target_link_options(${PROJECT_NAME} PRIVATE /GUARD:CF)
    endif()
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE
    MDKLOADER_BUILD_LIBRARY
)
target_compile_definitions(${PROJECT_NAME} PUBLIC
    BUILD_MDK_STATIC
)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
endif()
if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)
