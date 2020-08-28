/*
 * Copyright (c) 2019-2020 WangBin <wbsecg1 at gmail.com>
 */
#pragma once
#include "global.h"

enum MDK_RenderAPI {
    MDK_RenderAPI_Invalid,
    MDK_RenderAPI_OpenGL = 1,
    MDK_RenderAPI_Vulkan = 2,
    MDK_RenderAPI_Metal = 3,
    MDK_RenderAPI_D3D11 = 4,
};

/*!
  \brief mdkRenderAPI
  use concrete types in user code, for example mdkD3D11RenderAPI
 */
typedef struct mdkRenderAPI mdkRenderAPI;

struct mdkGLRenderAPI {
    MDK_RenderAPI type;
/*** Render Context Resources. Foreign context (provided by user) only ***/
    int fbo; // if >=0, will draw in given fbo. no need to bind in user code
    int unused;
    void* (*getProcAddress)(const char* name, void* opaque); /* NOT IMPLENETED */
    void* (*getCurrentNativeContext)(void* opaque); /* NOT IMPLENETED */
    void* opaque; /* NOT IMPLENETED */

/***
  Render Context Creation Options.
  as input, they are desired values to create an internal context(ignored if context is provided by user). as output, they are result values(if context is not provided by user)
***/
    bool debug; /* default false. NOT IMPLENETED */
    int8_t egl; /* default -1. -1: auto. 0: no, 1: yes */
/* if any one of opengl and opengles is 0, then another is treated as 1 */
    int8_t opengl; /* default -1. -1: auto. 0: no, 1: yes */
    int8_t opengles; /* default -1. -1: auto. 0: no, 1: yes */
    uint8_t profile; /* default 3. 0: no profile, 1: core profile, 2: compatibility profile */
    float version; /* default 0, ignored if < 2.0. requested version major.minor. result version may < requested version if not supported */
    int8_t reserved[32];
};

struct mdkMetalRenderAPI {
    MDK_RenderAPI type;
/*** Render Context Resources. Foreign context (provided by user) only ***/
// id<?> => void*: to be compatible with c++
    void* device = nullptr; // MUST set if metal is provided by user
    void* cmdQueue = nullptr; // optional. if not null, device can be null. currentQueue callback to share the same command buffer?
/* one of texture and currentRenderTarget MUST be set if metal is provided by user */
    void* texture = nullptr; // optional. id<MTLTexture>. if not null, device can be null. usually for offscreen rendering. render target for MTLRenderPassDescriptor if encoder is not provided by user. set once for offscreen rendering
    void* opaque = nullptr; // optional. callback opaque
    void* (*currentRenderTarget)(void* opaque) = nullptr; // optional. usually for on screen rendering. return id<MTLTexture>.
    // no encoder because we need own render pass
    void* reserved[2];

/***
  Render Context Creation Options.
  as input, they are desired values to create an internal context(ignored if context is provided by user). as output, they are result values(if context is not provided by user)
***/
    // device options: macOS only
    int device_index/*  = -1*/; // -1 will use system default device. callback with index+name?
};

/*!
  NOTE: include d3d11.h first to use D3D11RenderAPI
 */
#if defined(D3D11_SDK_VERSION)
struct mdkD3D11RenderAPI {
    MDK_RenderAPI type;
/*** Render Context Resources. Foreign context (provided by user) only ***/
/*
  context and rtv can be set by user if user can provide. then rendering becomes foreign context mode.
  if rtv is not null, no need to set context
  \sa Player.setRenderAPI()
 */
    ID3D11DeviceContext* context;
    // rtv or texture. usually user can provide a texture from gui easly, no d3d code to create a view
    ID3D11DeviceChild* rtv; // optional. the render target(view). ID3D11RenderTargetView or ID3D11Texture2D. can be null if context is not null. if not null, no need to set context
    void* reserved[2];

/***
  Render Context Creation Options.
  as input, they are desired values to create an internal context(ignored if context is provided by user). as output, they are result values(if context is not provided by user)
***/
    bool debug;
    int buffers; /* UWP must >= 2. */
    int adapter; /* adapter index */
    float feature_level; /* 0 is the highest */
};
#endif


// always declare
struct mdkVulkanRenderAPI {
    MDK_RenderAPI type;

#if (VK_VERSION_1_0+0)
    VkInstance instance/* = VK_NULL_HANDLE*/; // OPTIONAL. shared instance. for internal created context but not foreign context, to load instance extensions
    VkPhysicalDevice phy_device/* = VK_NULL_HANDLE*/; // OPTIONAL to create internal context. MUST not null for foreign context. Must set if logical device is provided to create internal context.
    VkDevice device/* = VK_NULL_HANDLE*/; // Optional to create internal context as shared device. Required for foreign context.
    VkQueue graphics_queue/*/* = VK_NULL_HANDLE*/; // OPTIONAL. If null, will use gfx_queue_index. NOT required if vk is create internally
/*!
  \brief rtv
  Used by offscreen rendering.
 */
    VkImageView *rtv/* = nullptr*/; // TODO: VkImage?
    VkRenderPass render_pass/* = VK_NULL_HANDLE*/; // optional. Usually not for offscreen rendering
    void* opaque/* = nullptr*/;
/*!
  \brief renderTargetSize
  Get render target image size
  \return image count, e.g. swapchain image count.
 */
    int (*renderTargetSize)(void* opaque, int* w, int* h); // return count
/*!
  \brief beginFrame
  Optional. Can be null(or not) for offscreen rendering.
  MUST be paired with endFrame()
  \param fb can be null, then will create internally. if not null, should be used with render_pass
  \param imgSem from present queue. can be null if fulfill any of
  1. present queue == gfx queue
  2. getCommandBuffer() is provided and submit in user code
  \return image index.
*/
    int (*beginFrame)(void* opaque, VkImageView* view/* = nullptr*/, VkFramebuffer* fb/*= nullptr*/, VkSemaphore* imgSem/* = nullptr*/)/* = nullptr*/;
    // int getNextImageView(); // not fbo, fbo is bound to render pass(can be dummy tmp). image view can also be used by compute pipeline. return index
/*!
  \brief currentCommandBuffer()
  if null, create pool internally(RTT)
 */
    VkCommandBuffer (*currentCommandBuffer)(void* opaque)/* = nullptr*/;
/*!
  \brief endFrame
  Optional. If null, frame is guaranteed to be rendered to image before executing the next command buffer in user code.
  If not null, user can wait for drawSem before using the image.
  MUST be paired with beginFrame()
  \param drawSem from gfx queue. can be null if fulfill any of
  1. present queue == gfx queue
  2. getCommandBuffer() is provided and submit in user code
  3. RTT offscreen rendering, i.e. rtv is set and beginFrame is null(user should wait for draw finish too)
 */
    void (*endFrame)(void* opaque, VkSemaphore* drawSem/* = nullptr*/)/*= nullptr*/; // can be null if offscreen. wait drawSem before present
#endif // (VK_VERSION_1_0+0)
    void* reserved[2];

/*
  Set by user and used internally even if device is provided by user
 */
    int graphics_family/*  = -1*/; // MUST if graphics and transfer queue family are different
    int compute_family/*  = -1*/; // optional. it's graphics_family if not set
    int transfer_family/*  = -1*/; // optional. it's graphics_family if not set
    int present_family/*  = -1*/; // optional. Must set if logical device is provided to create internal context
/***
  Render Context Creation Options.
  as input, they are desired values to create an internal context(ignored if context is provided by user). as output, they are result values(if context is not provided by user)
***/
    bool debug/* = false*/;
    uint8_t buffers/*  = 2*/; // 2 for double-buffering
    int device_index/*  = -1*/;
    uint32_t max_version/*  = 0*/; // requires vulkan 1.1
    int gfx_queue_index/*  = 0*/; // OPTIONAL
    int transfer_queue_index/*  = -1*/; // OPTIONAL. if not set, will use gfx queue
    int compute_queue_index/*  = -1*/; // OPTIONAL. if not set, will use gfx queue

    int depth/* = 8*/;
    //const char*
    uint8_t reserved_opt[32]; // color space etc.
};
