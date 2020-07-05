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

#include "mdkloader.h"

#include "mdk/c/MediaInfo.h"
#include "mdk/c/Player.h"
#include "mdk/c/VideoFrame.h"
#include "mdk/c/global.h"
#include <QDebug>
#include <QLibrary>

namespace {

#ifndef MDKLOADER_GENERATE_MDKAPI
#define MDKLOADER_GENERATE_MDKAPI(funcName, resultType, ...) \
    using _MDKLOADER_MDKAPI_##funcName = resultType (*)(__VA_ARGS__); \
    _MDKLOADER_MDKAPI_##funcName m_lp##funcName = nullptr;
#endif

#ifndef MDKLOADER_RESOLVE_MDKAPI
#define MDKLOADER_RESOLVE_MDKAPI(funcName) \
    if (!m_lp##funcName) { \
        m_lp##funcName = reinterpret_cast<_MDKLOADER_MDKAPI_##funcName>(mdkLib.resolve(#funcName)); \
        Q_ASSERT_X(m_lp##funcName, __FUNCTION__, qUtf8Printable(mdkLib.errorString())); \
    }
#endif

// global.h
MDKLOADER_GENERATE_MDKAPI(MDK_javaVM, void *, void *)
MDKLOADER_GENERATE_MDKAPI(MDK_setLogLevel, void, MDK_LogLevel)
MDKLOADER_GENERATE_MDKAPI(MDK_logLevel, MDK_LogLevel)
MDKLOADER_GENERATE_MDKAPI(MDK_setLogHandler, void, mdkLogHandler)
MDKLOADER_GENERATE_MDKAPI(MDK_setGlobalOptionString, void, const char *, const char *)
MDKLOADER_GENERATE_MDKAPI(MDK_setGlobalOptionInt32, void, const char *, int)
MDKLOADER_GENERATE_MDKAPI(MDK_setGlobalOptionPtr, void, const char *, void *)
MDKLOADER_GENERATE_MDKAPI(MDK_strdup, char *, const char *)
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

QLibrary mdkLib;

} // namespace

void mdkloader_setMdkLibName(const char *value)
{
    if (value) {
        mdkLib.setFileName(QString::fromUtf8(value));
    } else {
        qDebug().noquote() << "Failed to set MDK library name: empty file name.";
    }
}

const char *mdkloader_mdkLibName()
{
    return qUtf8Printable(mdkLib.fileName());
}

bool mdkloader_initMdk()
{
    // global.h
    MDKLOADER_RESOLVE_MDKAPI(MDK_javaVM)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setLogLevel)
    MDKLOADER_RESOLVE_MDKAPI(MDK_logLevel)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setLogHandler)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setGlobalOptionString)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setGlobalOptionInt32)
    MDKLOADER_RESOLVE_MDKAPI(MDK_setGlobalOptionPtr)
    MDKLOADER_RESOLVE_MDKAPI(MDK_strdup)
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
    const bool ret = mdkloader_isMdkLoaded();
    if (ret) {
        qDebug().noquote() << "MDK library has been loaded successfully.";
    } else {
        qWarning().noquote() << "Failed to load MDK library.";
    }
    return ret;
}

bool mdkloader_isMdkLoaded()
{
    const bool globalLoaded = (m_lpMDK_javaVM && m_lpMDK_setLogLevel && m_lpMDK_logLevel
                               && m_lpMDK_setLogHandler && m_lpMDK_setGlobalOptionString
                               && m_lpMDK_setGlobalOptionInt32 && m_lpMDK_setGlobalOptionPtr
                               && m_lpMDK_strdup);
    const bool mediaInfoLoaded = (m_lpMDK_AudioStreamCodecParameters && m_lpMDK_AudioStreamMetadata
                                  && m_lpMDK_VideoStreamCodecParameters
                                  && m_lpMDK_VideoStreamMetadata && m_lpMDK_MediaMetadata);
    const bool playerLoaded = (m_lpmdkPlayerAPI_new && m_lpmdkPlayerAPI_delete
                               && m_lpMDK_foreignGLContextDestroyed);
    const bool videoFrameLoaded = (m_lpmdkVideoFrameAPI_new && m_lpmdkVideoFrameAPI_delete);
    return (globalLoaded && mediaInfoLoaded && playerLoaded && videoFrameLoaded);
}

///////////////////////////////////////////
/// MDK
///////////////////////////////////////////

// global.h

void *MDK_javaVM(void *value)
{
    return m_lpMDK_javaVM(value);
}

void MDK_setLogLevel(MDK_LogLevel value)
{
    m_lpMDK_setLogLevel(value);
}

MDK_LogLevel MDK_logLevel()
{
    return m_lpMDK_logLevel();
}

void MDK_setLogHandler(mdkLogHandler value)
{
    m_lpMDK_setLogHandler(value);
}

void MDK_setGlobalOptionString(const char *key, const char *value)
{
    m_lpMDK_setGlobalOptionString(key, value);
}

void MDK_setGlobalOptionInt32(const char *key, int value)
{
    m_lpMDK_setGlobalOptionInt32(key, value);
}

void MDK_setGlobalOptionPtr(const char *key, void *value)
{
    m_lpMDK_setGlobalOptionPtr(key, value);
}

char *MDK_strdup(const char *value)
{
    return m_lpMDK_strdup(value);
}

// MediaInfo.h

void MDK_AudioStreamCodecParameters(const mdkAudioStreamInfo *asi, mdkAudioCodecParameters *acp)
{
    m_lpMDK_AudioStreamCodecParameters(asi, acp);
}

bool MDK_AudioStreamMetadata(const mdkAudioStreamInfo *asi, mdkStringMapEntry *sme)
{
    return m_lpMDK_AudioStreamMetadata(asi, sme);
}

void MDK_VideoStreamCodecParameters(const mdkVideoStreamInfo *vsi, mdkVideoCodecParameters *vcp)
{
    m_lpMDK_VideoStreamCodecParameters(vsi, vcp);
}

bool MDK_VideoStreamMetadata(const mdkVideoStreamInfo *vsi, mdkStringMapEntry *sme)
{
    return m_lpMDK_VideoStreamMetadata(vsi, sme);
}

bool MDK_MediaMetadata(const mdkMediaInfo *mi, mdkStringMapEntry *sme)
{
    return m_lpMDK_MediaMetadata(mi, sme);
}

// Player.h

mdkPlayerAPI *mdkPlayerAPI_new()
{
    return m_lpmdkPlayerAPI_new();
}

void mdkPlayerAPI_delete(mdkPlayerAPI **value)
{
    m_lpmdkPlayerAPI_delete(value);
}

void MDK_foreignGLContextDestroyed()
{
    m_lpMDK_foreignGLContextDestroyed();
}

// VideoFrame.h

mdkVideoFrameAPI *mdkVideoFrameAPI_new(int w, int h, MDK_PixelFormat f)
{
    return m_lpmdkVideoFrameAPI_new(w, h, f);
}

void mdkVideoFrameAPI_delete(mdkVideoFrameAPI **value)
{
    m_lpmdkVideoFrameAPI_delete(value);
}
