/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#if defined(WIN32) || defined(_WIN32) || defined(_WINDOWS)
#define MDK_WINDOWS
#else
#define MDK_UNIX
#endif

#include "mdkloader.h"
#include "mdk/c/MediaInfo.h"
#include "mdk/c/Player.h"
#include "mdk/c/VideoFrame.h"
#include "mdk/c/global.h"
#include <cassert>
#include <iostream>
#ifdef MDK_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {

#ifndef MDK_HANDLE
#ifdef MDK_WINDOWS
#define MDK_HANDLE HMODULE
#else
#define MDK_HANDLE void *
#endif
#endif

#ifndef MDKLOADER_GETPROCADDRESS
#ifdef MDK_WINDOWS
#define MDKLOADER_GETPROCADDRESS GetProcAddress
#else
#define MDKLOADER_GETPROCADDRESS dlsym
#endif
#endif

#ifndef MDKLOADER_GENERATE_MDKAPI
#define MDKLOADER_GENERATE_MDKAPI(funcName, resultType, ...) \
    using _MDKLOADER_MDKAPI_##funcName = resultType (*)(__VA_ARGS__); \
    _MDKLOADER_MDKAPI_##funcName m_lp##funcName = nullptr;
#endif

#ifndef MDKLOADER_RESOLVE_ERROR
#ifdef _DEBUG
#define MDKLOADER_RESOLVE_ERROR(funcName) assert(m_lp##funcName);
#else
#define MDKLOADER_RESOLVE_ERROR(funcName) \
    if (!m_lp##funcName) { \
        std::cerr << "Failed to resolve symbol" << #funcName << std::endl; \
    }
#endif
#endif

#ifndef MDKLOADER_RESOLVE_MDKAPI
#define MDKLOADER_RESOLVE_MDKAPI(funcName) \
    if (!m_lp##funcName) { \
        m_lp##funcName = reinterpret_cast<_MDKLOADER_MDKAPI_##funcName>( \
            MDKLOADER_GETPROCADDRESS(mdkLib, #funcName)); \
        MDKLOADER_RESOLVE_ERROR(funcName) \
    }
#endif

#ifndef MDKLOADER_EXECUTE_MDKAPI
#define MDKLOADER_EXECUTE_MDKAPI(funcName, ...) \
    if (m_lp##funcName) { \
        m_lp##funcName(__VA_ARGS__); \
    }
#endif

#ifndef MDKLOADER_EXECUTE_MDKAPI_RETURN
#define MDKLOADER_EXECUTE_MDKAPI_RETURN(funcName, defVal, ...) \
    return m_lp##funcName ? m_lp##funcName(__VA_ARGS__) : defVal;
#endif

MDK_HANDLE mdkLib = nullptr;

// global.h
MDKLOADER_GENERATE_MDKAPI(MDK_javaVM, void *, void *)
MDKLOADER_GENERATE_MDKAPI(MDK_setLogLevel, void, MDK_LogLevel)
MDKLOADER_GENERATE_MDKAPI(MDK_logLevel, MDK_LogLevel)
MDKLOADER_GENERATE_MDKAPI(MDK_setLogHandler, void, mdkLogHandler)
MDKLOADER_GENERATE_MDKAPI(MDK_setGlobalOptionString, void, const char *, const char *)
MDKLOADER_GENERATE_MDKAPI(MDK_setGlobalOptionInt32, void, const char *, int)
MDKLOADER_GENERATE_MDKAPI(MDK_setGlobalOptionPtr, void, const char *, void *)
MDKLOADER_GENERATE_MDKAPI(MDK_strdup, char *, const char *)
MDKLOADER_GENERATE_MDKAPI(MDK_version, int)
// MediaInfo.h
MDKLOADER_GENERATE_MDKAPI(MDK_AudioStreamCodecParameters,
                          void,
                          const mdkAudioStreamInfo *,
                          mdkAudioCodecParameters *)
MDKLOADER_GENERATE_MDKAPI(MDK_AudioStreamMetadata,
                          bool,
                          const mdkAudioStreamInfo *,
                          mdkStringMapEntry *)
MDKLOADER_GENERATE_MDKAPI(MDK_VideoStreamCodecParameters,
                          void,
                          const mdkVideoStreamInfo *,
                          mdkVideoCodecParameters *)
MDKLOADER_GENERATE_MDKAPI(MDK_VideoStreamMetadata,
                          bool,
                          const mdkVideoStreamInfo *,
                          mdkStringMapEntry *)
MDKLOADER_GENERATE_MDKAPI(MDK_MediaMetadata, bool, const mdkMediaInfo *, mdkStringMapEntry *)
// Player.h
MDKLOADER_GENERATE_MDKAPI(mdkPlayerAPI_new, mdkPlayerAPI *)
MDKLOADER_GENERATE_MDKAPI(mdkPlayerAPI_delete, void, mdkPlayerAPI **)
MDKLOADER_GENERATE_MDKAPI(MDK_foreignGLContextDestroyed, void)
// VideoFrame.h
MDKLOADER_GENERATE_MDKAPI(mdkVideoFrameAPI_new, mdkVideoFrameAPI *, int, int, MDK_PixelFormat)
MDKLOADER_GENERATE_MDKAPI(mdkVideoFrameAPI_delete, void, mdkVideoFrameAPI **)

} // namespace

bool mdkloader_load(const char *value)
{
    if (value) {
#ifdef MDK_WINDOWS
        const size_t newSize = strlen(value) + 1;
        auto *wValue = new wchar_t[newSize];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wValue, newSize, value, _TRUNCATE);
        mdkLib = LoadLibraryW(wValue);
        delete[] wValue;
#else
        mdkLib = dlopen(value, RTLD_LAZY);
#endif
    }
#ifdef _DEBUG
    assert(mdkLib);
#else
    if (!mdkLib) {
        std::cerr << "Failed to load the MDK library:" << value << std::endl;
        return false;
    }
#endif
    std::cout << "The MDK library has been loaded successfully." << std::endl;
    // global.h
    MDKLOADER_RESOLVE_MDKAPI(MDK_javaVM)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setLogLevel)
    MDKLOADER_RESOLVE_MDKAPI(MDK_logLevel)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setLogHandler)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setGlobalOptionString)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setGlobalOptionInt32)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setGlobalOptionPtr)
    MDKLOADER_RESOLVE_MDKAPI(MDK_strdup)
    MDKLOADER_RESOLVE_MDKAPI(MDK_version)
    // MediaInfo.h
    MDKLOADER_RESOLVE_MDKAPI(MDK_AudioStreamCodecParameters)
    MDKLOADER_RESOLVE_MDKAPI(MDK_AudioStreamMetadata)
    MDKLOADER_RESOLVE_MDKAPI(MDK_VideoStreamCodecParameters)
    MDKLOADER_RESOLVE_MDKAPI(MDK_VideoStreamMetadata)
    MDKLOADER_RESOLVE_MDKAPI(MDK_MediaMetadata)
    // Player.h
    MDKLOADER_RESOLVE_MDKAPI(mdkPlayerAPI_new)
    MDKLOADER_RESOLVE_MDKAPI(mdkPlayerAPI_delete)
    MDKLOADER_RESOLVE_MDKAPI(MDK_foreignGLContextDestroyed)
    // VideoFrame.h
    MDKLOADER_RESOLVE_MDKAPI(mdkVideoFrameAPI_new)
    MDKLOADER_RESOLVE_MDKAPI(mdkVideoFrameAPI_delete)
    const bool ret = mdkloader_isLoaded();
    if (ret) {
        std::cout << "All MDK symbols have been resolved successfully." << std::endl;
    } else {
        std::cerr << "Failed to resolve some MDK symbols." << std::endl;
    }
    return ret;
}

bool mdkloader_isLoaded()
{
    if (!mdkLib) {
        return false;
    }
    const bool globalLoaded = (m_lpMDK_javaVM && m_lpMDK_setLogLevel && m_lpMDK_logLevel
                               && m_lpMDK_setLogHandler && m_lpMDK_setGlobalOptionString
                               && m_lpMDK_setGlobalOptionInt32 && m_lpMDK_setGlobalOptionPtr
                               && m_lpMDK_strdup && m_lpMDK_version);
    const bool mediaInfoLoaded = (m_lpMDK_AudioStreamCodecParameters && m_lpMDK_AudioStreamMetadata
                                  && m_lpMDK_VideoStreamCodecParameters
                                  && m_lpMDK_VideoStreamMetadata && m_lpMDK_MediaMetadata);
    const bool playerLoaded = (m_lpmdkPlayerAPI_new && m_lpmdkPlayerAPI_delete
                               && m_lpMDK_foreignGLContextDestroyed);
    const bool videoFrameLoaded = (m_lpmdkVideoFrameAPI_new && m_lpmdkVideoFrameAPI_delete);
    return (globalLoaded && mediaInfoLoaded && playerLoaded && videoFrameLoaded);
}

int mdkloader_version()
{
    MDKLOADER_EXECUTE_MDKAPI_RETURN(MDK_version, MDK_VERSION)
}

void mdkloader_cleanup()
{
    if (mdkLib) {
#ifdef MDK_WINDOWS
        if (!FreeLibrary(mdkLib)) {
            std::cerr << "Failed to unload the MDK library." << std::endl;
        }
#else
        dlclose(mdkLib);
#endif
    }
}

///////////////////////////////////////////
/// MDK
///////////////////////////////////////////

// global.h

void *MDK_javaVM(void *value)
{
    MDKLOADER_EXECUTE_MDKAPI_RETURN(MDK_javaVM, nullptr, value)
}

void MDK_setLogLevel(MDK_LogLevel value){MDKLOADER_EXECUTE_MDKAPI(MDK_setLogLevel, value)}

MDK_LogLevel MDK_logLevel()
{
    MDKLOADER_EXECUTE_MDKAPI_RETURN(MDK_logLevel, MDK_LogLevel_Debug)
}

void MDK_setLogHandler(mdkLogHandler value)
{
    MDKLOADER_EXECUTE_MDKAPI(MDK_setLogHandler, value)
}

void MDK_setGlobalOptionString(const char *key, const char *value)
{
    MDKLOADER_EXECUTE_MDKAPI(MDK_setGlobalOptionString, key, value)
}

void MDK_setGlobalOptionInt32(const char *key, int value)
{
    MDKLOADER_EXECUTE_MDKAPI(MDK_setGlobalOptionInt32, key, value)
}

void MDK_setGlobalOptionPtr(const char *key, void *value)
{
    MDKLOADER_EXECUTE_MDKAPI(MDK_setGlobalOptionPtr, key, value)
}

char *MDK_strdup(const char *value)
{
    MDKLOADER_EXECUTE_MDKAPI_RETURN(MDK_strdup, nullptr, value)
}

// MediaInfo.h

void MDK_AudioStreamCodecParameters(const mdkAudioStreamInfo *asi, mdkAudioCodecParameters *acp)
{
    MDKLOADER_EXECUTE_MDKAPI(MDK_AudioStreamCodecParameters, asi, acp)
}

bool MDK_AudioStreamMetadata(const mdkAudioStreamInfo *asi, mdkStringMapEntry *sme)
{
    MDKLOADER_EXECUTE_MDKAPI_RETURN(MDK_AudioStreamMetadata, false, asi, sme)
}

void MDK_VideoStreamCodecParameters(const mdkVideoStreamInfo *vsi, mdkVideoCodecParameters *vcp)
{
    MDKLOADER_EXECUTE_MDKAPI(MDK_VideoStreamCodecParameters, vsi, vcp)
}

bool MDK_VideoStreamMetadata(const mdkVideoStreamInfo *vsi, mdkStringMapEntry *sme)
{
    MDKLOADER_EXECUTE_MDKAPI_RETURN(MDK_VideoStreamMetadata, false, vsi, sme)
}

bool MDK_MediaMetadata(const mdkMediaInfo *mi, mdkStringMapEntry *sme){
    MDKLOADER_EXECUTE_MDKAPI_RETURN(MDK_MediaMetadata, false, mi, sme)}

// Player.h

mdkPlayerAPI *mdkPlayerAPI_new()
{
    MDKLOADER_EXECUTE_MDKAPI_RETURN(mdkPlayerAPI_new, nullptr)
}

void mdkPlayerAPI_delete(mdkPlayerAPI **value)
{
    MDKLOADER_EXECUTE_MDKAPI(mdkPlayerAPI_delete, value)
}

void MDK_foreignGLContextDestroyed(){MDKLOADER_EXECUTE_MDKAPI(MDK_foreignGLContextDestroyed)}

// VideoFrame.h

mdkVideoFrameAPI *mdkVideoFrameAPI_new(int w, int h, MDK_PixelFormat f)
{
    MDKLOADER_EXECUTE_MDKAPI_RETURN(mdkVideoFrameAPI_new, nullptr, w, h, f)
}

void mdkVideoFrameAPI_delete(mdkVideoFrameAPI **value)
{
    MDKLOADER_EXECUTE_MDKAPI(mdkVideoFrameAPI_delete, value)
}
