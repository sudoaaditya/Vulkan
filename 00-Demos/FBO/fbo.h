#pragma once
#include "vk.h"

//! FBO Related Functions Prototypes
VkResult createSwapchainImagesAndImageViews_fbo(void);
VkResult createCommandBuffers_fbo(void);
void addTriangle(float[3][3], float[3][3], float[3][2]);
VkResult createVertexBuffer_fbo(void);
VkResult createIndexBuffer_fbo(void);
VkResult createTexture(const char*);
VkResult createUniformBuffer_fbo(void);
VkResult createShaders_fbo(void);
VkResult createDescriptorSetLayout_fbo(void);
VkResult createPipelineLayout_fbo(void);
VkResult createDescriptorPool_fbo(void);
VkResult createDescriptorSet_fbo(void);
VkResult createRenderPass_fbo(void);
VkResult createPipeline_fbo(void);
VkResult createFramebuffer_fbo(void);
VkResult createSemaphore_fbo(void);
VkResult buildCommandBuffer_fbo(void);