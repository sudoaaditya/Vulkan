#pragma once

#ifndef VK_H
#define VK_H

#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<string>

// vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR
#include<vulkan/vulkan.h>

// glm related macros & header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // clip space depth range is [0, 1]
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// assimp related header files
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "resource.h"
#include "ModelLoading.h"

// vulkan related libraries
#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "assimp-vc142-mtd.lib")

extern FILE* fptr; // global file pointer for logging

#endif // VK_H