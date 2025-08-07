#include <vulkan/vulkan.h>

#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

VkResult setVertexBuffer(const char* pMessage, void *pData, size_t iDataSize, VkBuffer &vkBuffer, VkDeviceMemory &vkDeviceMemory);
VkResult setIndexBuffer(const char* pMessage, void *pData, size_t iDataSize, VkBuffer &vkBuffer, VkDeviceMemory &vkDeviceMemory);

#endif // VERTEX_BUFFER_H