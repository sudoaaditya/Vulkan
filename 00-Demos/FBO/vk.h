#pragma once

#include<windows.h>
#include<stdio.h>
#include<stdlib.h>

#include "icon.h"

// vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR
#include<vulkan/vulkan.h>

// glm related macros & header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // clip space depth range is [0, 1]
#include "../../glm/glm.hpp"
#include "../../glm/gtc/matrix_transform.hpp"

// vulkan related libraries
#pragma comment(lib, "vulkan-1.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// FBO Related Variables
#define FBO_WIDTH 512
#define FBO_HEIGHT 512

// Vertex Attributes Enum
enum {
    AMK_ATTRIBUTE_POSITION = 0,
    AMK_ATTRIBUTE_NORMAL = 1,
    AMK_ATTRIBUTE_TEXCOORD = 2
};

// Vertex Buffer
typedef struct {
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} VertexData;

typedef struct {
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} UniformData;

extern FILE *fptr;

extern int winWidth;
extern int winHeight;

// Vulkan Device
extern VkDevice vkDevice;

// Device Queue
extern VkQueue vkQueue;

extern VkSwapchainKHR vkSwapchainKHR;

// Command Pool
extern VkCommandPool vkCommandPool;

extern VkFormat vkFormat_color;
extern VkFormat vkFormat_depth;

extern VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

// ! FBO RELATED EXTERNS
// Build Command Buffers
extern VkClearColorValue vkClearColorValue_fbo;
extern VkClearDepthStencilValue vkClearDepthStencilValue_fbo;

// Render Variables
extern BOOL bInitialized_fbo;

extern int fboWidth;
extern int fboHeight;

// Command Buffer
extern VkCommandBuffer vkCommandBuffer_fbo;

// Fences & Semaphore
extern VkSemaphore vkSemaphore_fbo;

// Fbo Image related Variables
extern VkImageView vkImageView_fbo;
extern VkSampler vkSampler_fbo;

// For Rotation
extern BOOL bAnimate;
extern BOOL bLight;
extern BOOL bTexture;
