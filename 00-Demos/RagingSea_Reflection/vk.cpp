#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
// for srand, rand and time
#include <cstdlib> 
#include <cmath>
#include <ctime> 

#include<vector>
using namespace std;

#include "vk.h"

// vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR
#include<vulkan/vulkan.h>

// glm related macros & header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // clip space depth range is [0, 1]
#include "../../glm/glm.hpp"
#include "../../glm/gtc/matrix_transform.hpp"

#include "clockUtils/Clock.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "./imgui/imgui.h"
#include "./imgui/backends/imgui_impl_win32.h"
#include "./imgui/backends/imgui_impl_vulkan.h"

// vulkan related libraries
#pragma comment(lib, "vulkan-1.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global variables
BOOL gbFullScreen = FALSE;
BOOL gbWindowMinimized = FALSE;
DWORD dwStyle = 0;
WINDOWPLACEMENT wpPrev;
HWND ghwnd = NULL;
BOOL gbActiveWindow = FALSE;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
FILE *fptr = NULL;
const CHAR *gpszAppName = "ARTR: Vulkan";

// Vertex Attributes Enum
enum {
    AMK_ATTRIBUTE_POSITION = 0,
};

// instance extension related variables
uint32_t enabledInstanceExtensionCount_sea = 0; 
// VK_KHR_SURFACE_EXTENSION_NAME & VK_KHR_WIN32_SURFACE_EXTENSION_NAME & VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char *enabledInstanceExtensionNames_array_sea[3]; 
// vulkan instance
VkInstance vkInstance_sea = VK_NULL_HANDLE;

//vulkan presentation surface object
VkSurfaceKHR vkSurfaceKHR_sea = VK_NULL_HANDLE;

// vulkan physical device related variables
VkPhysicalDevice vkPhysicalDevice_selected_sea = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected_sea = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties_sea;

//
uint32_t physicalDeviceCount_sea = 0;
VkPhysicalDevice *vkPhysicalDevice_array_sea = NULL;

// Device Extension related variables
uint32_t enabledDeviceExtensionCount_sea = 0;
const char *enabledDeviceExtensionNames_array_sea[1]; // VK_KHR_SWAPCHAIN_EXTENSION_NAME

// Vulkan Device
VkDevice vkDevice_sea = VK_NULL_HANDLE;

// Device Queue
VkQueue vkQueue_sea = VK_NULL_HANDLE;

// Surface Format & Surcae ColorSpace
VkFormat vkFormat_color_sea = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR_sea = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// Presentation Mode
VkPresentModeKHR vkPresentModeKHR_sea = VK_PRESENT_MODE_FIFO_KHR;

// Swapchain
int winWidth_sea = WIN_WIDTH, winHeight_sea = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR_sea = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain_sea;

// Swapchain Images & Image Views [ For color Images ]
uint32_t swapchainImageCount_sea = UINT32_MAX;
VkImage *swapchainImage_array_sea = NULL;
VkImageView *swapchainImageView_array_sea = NULL;

// For Depth Image
VkFormat vkFormat_depth_sea = VK_FORMAT_UNDEFINED;
VkImage vkImage_depth_sea = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_depth_sea = VK_NULL_HANDLE;
VkImageView vkImageView_depth_sea = VK_NULL_HANDLE;

// Command Pool
VkCommandPool vkCommandPool_sea = VK_NULL_HANDLE;

// Command Buffer
VkCommandBuffer *vkCommandBuffer_array_sea;

// Render Pass
VkRenderPass vkRenderPass_sea = VK_NULL_HANDLE;

// Frame Buffer
VkFramebuffer *vkFramebuffer_array_sea = NULL;

// Fences & Semaphore
VkSemaphore vkSemaphore_backbuffer_sea = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_rendercomplete_sea = VK_NULL_HANDLE;
VkFence *vkFence_array_sea = NULL;

// Build Command Buffers
VkClearColorValue vkClearColorValue_sea;
VkClearDepthStencilValue vkClearDepthStencilValue_sea;

// Render Variables
BOOL bInitialized_sea = FALSE;
uint32_t currentImageIndex_sea = UINT32_MAX;

// Validation Layer
BOOL bValidation_sea = TRUE;
uint32_t enabledValidationLayerCount_sea = 0;
const char *enabledValidationLayerNames_array_sea[1]; //VK_LAYER_KHRONOS_validation
VkDebugReportCallbackEXT vkDebugReportCallbackEXT_sea;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr_sea = NULL;

// Vertex Buffer
typedef struct {
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} VertexData;

// Position
VertexData vertexData_position_sea;

// Uniform Related Declarations
struct MyUniformData {
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    float cameraPosition[4];
    float waveDirections[4][4];
    float waveSettings[4][4];
    float detailParams[4];
    float depthColor[4];
    float surfaceColor[4];
    float skyBottomColor[4];
    float skyTopColor[4];
    float sunDirection[4];  // xyz: direction towards the sun
    float sunColor[4];      // rgb: sun color, a: sun glow
    float sunParams[4];     // x: glitter strength, y: sky ambient, z: subsurface strength
    float skyParams[4];     // x: horizon haze, y: fog density, w: sky exposure
    float shadingParams[4];
    float lightingParams[4];
    float sphereParams[4];  // x: 1/radius (plane units), y: max angle, z: sphere blend
    float bronzeDarkColor[4];   // rgb: troughs, a: roughness
    float bronzeBrightColor[4]; // rgb: crests, a: bronze blend
};

typedef struct {
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} UniformData;

UniformData uniformData_sea;

// Ocean Mask Texture (globe UVs, white = ocean)
VkImage vkImage_oceanMask = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_oceanMask = VK_NULL_HANDLE;
VkImageView vkImageView_oceanMask = VK_NULL_HANDLE;
VkSampler vkSampler_oceanMask = VK_NULL_HANDLE;

vector<glm::vec3> vertexData_array_sea;
float halfSize_sea = 5.0f; // bound of rect go from -5 to 5
const float gSeaModelScale = 6.0f; // plane scale in the model matrix
int segmentCount_sea = 512; // no of segments to divide the plane into

// Shader Variables
VkShaderModule vkShaderModule_vertex_sea = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_sea = VK_NULL_HANDLE;

// Descriptor Set Layout
VkDescriptorSetLayout vkDescriptorSetLayout_sea = VK_NULL_HANDLE;

// Pipeline Layout
VkPipelineLayout vkPipelineLayout_sea = VK_NULL_HANDLE;

// Descriptor Pool
VkDescriptorPool vkDescriptorPool_sea = VK_NULL_HANDLE;

// Descriptor Set
VkDescriptorSet vkDescriptorSet_sea = VK_NULL_HANDLE;

// Pipeline
VkViewport vkViewport_sea;
VkRect2D vkRect2D_scissor_sea;
VkPipeline vkPipeline_sea = VK_NULL_HANDLE;

// For Rotation
Clock myClock_sea;

// Camera movement
float gCameraOffsetX = 0.0f;
float gCameraOffsetY = 0.0f;
float gCameraOffsetZ = 0.0f;
const float gCameraMoveSpeed = 0.3f;

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

VkDescriptorPool vkDescriptorPool_imgui_sea = VK_NULL_HANDLE;

struct SeaUiState {
    float timeScale;
    float windDirectionX;
    float windDirectionY;
    float primaryWavelength;
    float primaryAmplitude;
    float waveSpeed;
    float choppiness;
    float detailHeight;
    float detailFrequency;
    float detailSpeed;
    float detailLayers;
    float depthColor[4];
    float surfaceColor[4];
    float skyBottomColor[4];
    float skyTopColor[4];
    float sunDirection[3];
    float sunColor[4];
    float sunIntensity;
    float sunGlow;
    float glitterStrength;
    float skyAmbient;
    float sssStrength;
    float horizonHaze;
    float fogDensity;
    float skyExposure;
    float colorOffset;
    float colorMultiplier;
    float fresnelPower;
    float reflectionStrength;
    float specularPower;
    float foamHeight;
    float foamIntensity;
    float sphereBlend;      // 0: flat, 1: sphere
    float sphereRadius;     // world units
    float bronzeBlend;      // 0: water, 1: bronze
    float bronzeDarkColor[3];
    float bronzeBrightColor[3];
    float bronzeRoughness;
};

SeaUiState gSeaUiState = {
    0.919f,                         // time scale
    1.0f,                           // wind direction x
    0.35f,                          // wind direction y
    1.786f,                         // primary wavelength
    0.035f,                         // primary amplitude
    2.5f,                           // wave speed
    0.188f,                         // choppiness
    0.015f,                         // detail height
    6.384f,                         // detail frequency
    4.392f,                         // detail speed
    4.0f,                           // detail layers
    {0.00f, 0.055f, 0.11f, 0.0f},   // depth color (deep daytime blue)
    {0.02f, 0.28f, 0.36f, 0.0f},    // surface color (sunlit teal)
    {0.40f, 0.57f, 0.78f, 0.0f},    // sky horizon
    {0.14f, 0.34f, 0.68f, 0.0f},    // sky zenith
    {-0.15f, 0.22f, -1.0f},         // sun direction (low, ahead of the camera)
    {1.00f, 0.92f, 0.78f, 0.0f},    // sun color (warm)
    3.5f,                           // sun intensity
    0.25f,                          // sun glow
    0.6f,                           // glitter strength
    1.0f,                           // sky ambient
    0.18f,                          // subsurface strength
    0.35f,                          // horizon haze
    0.5f,                           // fog density
    0.85f,                          // sky exposure
    0.10f,
    1.05f,
    5.6f,
    0.75f,                          // reflection strength (fresnel scale)
    160.0f,
    0.32f,
    0.22f,
    0.0f,                           // sphere blend (start flat)
    9.55f,                          // sphere radius (largest that still closes)
    0.0f,                           // bronze blend (start as water)
    // Bronze measured from the Atlas model textures:
    {0.30f, 0.23f, 0.15f},          // bronze dark
    {0.60f, 0.49f, 0.37f},          // bronze bright
    0.53f,                          // bronze roughness
};

float gSphereBlendTarget = 0.0f;
const float gSphereBlendSpeed = 0.25f; // per second

float gBronzeBlendTarget = 0.0f;
const float gBronzeBlendSpeed = 0.2f; // per second

bool gShowSeaControls = true;
bool gShowImGuiDemoWindow = false;
bool gCommandBuffersDirty = true;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {
    // Func
    VkResult initialize(void);
    VkResult display(void);
    void update(void);
    void uninitialize(void);
    
    // Vars
    WNDCLASSEX wndclass;
    MSG msg;
    HWND hwnd;
    TCHAR szAppName [255];
    BOOL bDone = FALSE;
    VkResult vkResult = VK_SUCCESS;

    fptr = fopen("_VulkanWindowLog.txt", "w");
    if(fptr == NULL) {
        MessageBox(NULL, TEXT("Cannot Create Log!!.."), TEXT("ErrMsg"), MB_OK);
        exit(0);
    }
    else {
        fprintf(fptr, "Log Created Successful!!\n\n");
    }

    wsprintf(szAppName, TEXT("%s"), gpszAppName);

    // Centered window
    int xPos = GetSystemMetrics(SM_CXSCREEN);
    int yPos = GetSystemMetrics(SM_CYSCREEN);
    int xMid = xPos / 2;
    int yMid = yPos / 2;

    xPos = xMid - (WIN_WIDTH / 2);
    yPos = yMid - (WIN_HEIGHT / 2);

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ICON_MORPHED));
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(ICON_MORPHED));
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
            szAppName,
            TEXT("AMK_Vulkan : Raging Sea"),
            WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
            xPos,
            yPos,
            WIN_WIDTH,
            WIN_HEIGHT,
            NULL,
            NULL,
            hInstance,
            NULL
        );

    ghwnd = hwnd;

    vkResult = initialize();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "WinMain(): initialize() Failed!.\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    } else {
        fprintf(fptr, "WinMain(): initialize() Successful!.\n");
    }

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    //game loop
    while(!bDone) {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT) {
                bDone = TRUE;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            if(gbWindowMinimized == FALSE) {
                if(gbActiveWindow == TRUE) {
                    update();
                }
                vkResult = display();
                if(vkResult != VK_FALSE && vkResult != VK_SUCCESS 
                    && vkResult != VK_SUBOPTIMAL_KHR && vkResult != VK_ERROR_OUT_OF_DATE_KHR) {
                    fprintf(fptr, "WinMain(): display() Failed!.\n");
                    bDone = TRUE;
                }
            }
        }
    }

    uninitialize();
    
    return((int)msg.wParam);
}


LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    //func
    void ToggleFullScreen(void);
    VkResult resize(int, int);

    //var
    BOOL bIsMax = FALSE;

    if((iMsg >= WM_MOUSEFIRST && iMsg <= WM_MOUSELAST)
        || iMsg == WM_KEYDOWN || iMsg == WM_KEYUP
        || iMsg == WM_SYSKEYDOWN || iMsg == WM_SYSKEYUP
        || iMsg == WM_CHAR || iMsg == WM_SETFOCUS || iMsg == WM_KILLFOCUS) {
        gCommandBuffersDirty = true;
    }

    if (ImGui_ImplWin32_WndProcHandler(hwnd, iMsg, wParam, lParam)) {
        return TRUE;
    }

    switch(iMsg) {

        case WM_CREATE:
            memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
            wpPrev.length = sizeof(WINDOWPLACEMENT);
            break;

        case WM_SETFOCUS:
            gbActiveWindow = TRUE;
            break;

        case WM_KILLFOCUS:
            gbActiveWindow = FALSE;
            break;

        case WM_SIZE:
            if(wParam == SIZE_MINIMIZED) {
                gbWindowMinimized = TRUE;
            } else {
                resize(LOWORD(lParam), HIWORD(lParam));
                gbWindowMinimized = FALSE;
            }
            break;

        case WM_KEYDOWN:
            switch(wParam) {

                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;

                case VK_LEFT:
                    gCameraOffsetX -= gCameraMoveSpeed;
                    break;

                case VK_RIGHT:
                    gCameraOffsetX += gCameraMoveSpeed;
                    break;

                case VK_UP:
                    gCameraOffsetZ -= gCameraMoveSpeed;
                    break;

                case VK_DOWN:
                    gCameraOffsetZ += gCameraMoveSpeed;
                    break;

                default:
                    break;
            }
            break;

        case WM_CHAR:
            switch(LOWORD(wParam)) {
                case 's':
                case 'S':
                    if(!bIsMax) {
                        ShowWindow(hwnd, SW_MAXIMIZE);
                        bIsMax = TRUE;
                    }
                    else {
                        ShowWindow(hwnd, SW_SHOWNORMAL);
                        bIsMax = FALSE;
                    }
                break;

                case 'f':
                case 'F':
                    ToggleFullScreen();
                    break;

                case 'b':
                case 'B':
                    // toggle between flat sea and sphere
                    gSphereBlendTarget = (gSphereBlendTarget > 0.5f) ? 0.0f : 1.0f;
                    break;

                case 'c':
                case 'C':
                    // toggle between water and bronze
                    gBronzeBlendTarget = (gBronzeBlendTarget > 0.5f) ? 0.0f : 1.0f;
                    break;
                
                default:
                    break;
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void){

	//var
	MONITORINFO mi = {sizeof(MONITORINFO)};

	if(!gbFullScreen){

		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if(dwStyle & WS_OVERLAPPEDWINDOW) {

			if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)){

				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOZORDER);
			}
		}
		// ShowCursor(FALSE);
		gbFullScreen = TRUE;
	}
	else {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		// ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
}


VkResult initialize(void) {

    // function declarations
    VkResult createVulkanInstance(void);
    VkResult getSupportedSurface(void);
    VkResult getPhysicalDevice(void);
    VkResult printVKInfo(void);
    VkResult createVulkanDevice(void);
    void getDeviceQueue(void);
    VkResult createSwapchain(VkBool32);
    VkResult createSwapchainImagesAndImageViews(void);
    VkResult createCommandPool(void);
    VkResult createCommandBuffers(void);
    VkResult createVertexBuffer(void);
    VkResult createUniformBuffer(void);
    VkResult createTexture(const char*);
    VkResult createShaders(void);
    VkResult createDescriptorSetLayout(void);
    VkResult createPipelineLayout(void);
    VkResult createDescriptorPool(void);
    VkResult createDescriptorSet(void);
    VkResult createRenderPass(void);
    VkResult createPipeline(void);
    VkResult createFramebuffers(void);
    VkResult createSemaphores(void);
    VkResult createFences(void);
    VkResult initializeImGui(void);
    VkResult buildCommandBuffers(void);


    // varibales
    VkResult vkResult = VK_SUCCESS;

    // code
    vkResult = createVulkanInstance();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createVulkanInstance() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createVulkanInstance() Successful!.\n\n");
    }

    // create vulkan presentation surface
    vkResult = getSupportedSurface();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): getSupportedSurface() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): getSupportedSurface() Successful!.\n\n");
    }

    // Get Physical Device, enumerate and select it's queue family index
    vkResult = getPhysicalDevice();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): getPhysicalDevice() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): getPhysicalDevice() Successful!.\n\n");
    }

    // Print Vulkan Info
    vkResult = printVKInfo();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): printVKInfo() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): printVKInfo() Successful!.\n\n");
    }

    vkResult = createVulkanDevice();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createVulkanDevice() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createVulkanDevice() Successful!.\n\n");
    }

    // Device Queue
    getDeviceQueue();

    // Swapchain
    vkResult = createSwapchain(VK_FALSE);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createSwapchain() Failed!.\n");
        return (VK_ERROR_INITIALIZATION_FAILED);
    } else {
        fprintf(fptr, "initialize(): createSwapchain() Successful!.\n\n");
    }

    // Swapchain Images & Image Views
    vkResult = createSwapchainImagesAndImageViews();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createSwapchainImagesAndImageViews() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createSwapchainImagesAndImageViews() Successful!.\n\n");
    }

    // Command Pool
    vkResult = createCommandPool();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createCommandPool() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createCommandPool() Successful!.\n\n");
    }

    // Command Buffer
    vkResult = createCommandBuffers();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createCommandBuffers() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createCommandBuffers() Successful!.\n\n");
    }

    // Create Vertex Buffer
    vkResult = createVertexBuffer();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createVertexBuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createVertexBuffer() Successful!.\n\n");
    }

    // Create Uniform Buffer
    vkResult = createUniformBuffer();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createUniformBuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createUniformBuffer() Successful!.\n\n");
    }

    // Create Ocean Mask Texture
    vkResult = createTexture("textures/ocean_mask.png");
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createTexture() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createTexture() Successful!.\n\n");
    }


    // Create Shaders
    vkResult = createShaders();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createShaders() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createShaders() Successful!.\n\n");
    }

    // Create Descriptor Set Layout
    vkResult = createDescriptorSetLayout();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createDescriptorSetLayout() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createDescriptorSetLayout() Successful!.\n\n");
    }

    // Create Pipeline Layout
    vkResult = createPipelineLayout();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createPipelineLayout() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createPipelineLayout() Successful!.\n\n");
    }

    // Create Descriptor Pool
    vkResult = createDescriptorPool();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createDescriptorPool() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createDescriptorPool() Successful!.\n\n");
    }

    // Create Descriptor Set
    vkResult = createDescriptorSet();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createDescriptorSet() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createDescriptorSet() Successful!.\n\n");
    }

    // Render Pass
    vkResult = createRenderPass();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createRenderPass() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createRenderPass() Successful!.\n\n");
    }

    // Pipeline
    vkResult = createPipeline();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createPipeline() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createPipeline() Successful!.\n\n");
    }

    // Framebuffers
    vkResult = createFramebuffers();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createFramebuffers() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createFramebuffers() Successful!.\n\n");
    }

    // Create Semaphores
    vkResult = createSemaphores();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createSemaphores() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createSemaphores() Successful!.\n\n");
    }

    // Create Fences
    vkResult = createFences();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createFences() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createFences() Successful!.\n\n");
    }

    // Initialize Dear ImGui for Win32 + Vulkan
    vkResult = initializeImGui();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): initializeImGui() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): initializeImGui() Successful!.\n\n");
    }

    // initialize clear color values
    memset((void*)&vkClearColorValue_sea, 0, sizeof(VkClearColorValue));
    vkClearColorValue_sea.float32[0] = 0.0f;
    vkClearColorValue_sea.float32[1] = 0.0f;
    vkClearColorValue_sea.float32[2] = 0.0f;
    vkClearColorValue_sea.float32[3] = 1.0f; // analogous to glClearColor

    // initialize clear depth stencil values
    memset((void*)&vkClearDepthStencilValue_sea, 0, sizeof(VkClearDepthStencilValue));
    vkClearDepthStencilValue_sea.depth = 1.0f; // analogous to glClearDepth [ Float Value]
    vkClearDepthStencilValue_sea.stencil = 0; // analogous to glClearStencil [ Integer Value ]


    // Build Command Buffers
    vkResult = buildCommandBuffers();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): buildCommandBuffers() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): buildCommandBuffers() Successful!.\n\n");
    }

    gCommandBuffersDirty = false;

    myClock_sea.start();

    // Initialization is completed!
    bInitialized_sea = TRUE;
    fprintf(fptr, "initialize(): Initialization Successful!.\n");

    return (vkResult);
}

VkResult resize(int width, int height) {
    // Function declarations
    VkResult createSwapchain(VkBool32);
    VkResult createSwapchainImagesAndImageViews(void);
    VkResult createCommandBuffers(void);
    VkResult createPipelineLayout(void);
    VkResult createPipeline(void);
    VkResult createFramebuffers(void);
    VkResult createRenderPass(void);
    VkResult buildCommandBuffers(void);


    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Code
    if(height <= 0)
        height = 1;

    // If control comes here before initialization is done, then return false
    if(bInitialized_sea == FALSE) {
        fprintf(fptr, "resize(): initialization is not completed or failed\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // As recreation of swapchain is required, we are going to repeat many steps of initialization again
    // hence set bInitialized_sea to FALSE
    bInitialized_sea = FALSE; // this will prevent display() function to execute before resize() is done

    // Set Global Width & Height
    winWidth_sea = width;
    winHeight_sea = height;

    // Wait til vkDevice_sea is idle
    if(vkDevice_sea) {
        vkDeviceWaitIdle(vkDevice_sea); // this basically waits on til all the operations are done using the device and then this function call returns
    }

    // Check if vkSwapchainKHR_sea is NULL, if it is NULL then we cannot proceed
    if(vkSwapchainKHR_sea == VK_NULL_HANDLE) {
        fprintf(fptr, "resize(): vkSwapchainKHR is NULL cannot proceed!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // Destroy Frame Buffers
    if(vkFramebuffer_array_sea) {
        for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
            vkDestroyFramebuffer(vkDevice_sea, vkFramebuffer_array_sea[i], NULL);
            vkFramebuffer_array_sea[i] = VK_NULL_HANDLE;
        }
    }

    if(vkFramebuffer_array_sea) {
        free(vkFramebuffer_array_sea);
        vkFramebuffer_array_sea = NULL;
    }

    // Destroy  Command Buffers
    if(vkCommandBuffer_array_sea) {
        for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
            vkFreeCommandBuffers(vkDevice_sea, vkCommandPool_sea, 1, &vkCommandBuffer_array_sea[i]);
            vkCommandBuffer_array_sea[i] = VK_NULL_HANDLE;
        }
    }

    if(vkCommandBuffer_array_sea) {
        free(vkCommandBuffer_array_sea);
        vkCommandBuffer_array_sea = NULL;
    }

    // Destroy Pipeline
    if(vkPipeline_sea) {
        vkDestroyPipeline(vkDevice_sea, vkPipeline_sea, NULL);
        vkPipeline_sea = VK_NULL_HANDLE;
    }

    // Destroy Pipeline Layout
    if(vkPipelineLayout_sea) {
        vkDestroyPipelineLayout(vkDevice_sea, vkPipelineLayout_sea, NULL);
        vkPipelineLayout_sea = VK_NULL_HANDLE;
    }

    // Destroy Render Pass
    if(vkRenderPass_sea) {
        vkDestroyRenderPass(vkDevice_sea, vkRenderPass_sea, NULL);
        vkRenderPass_sea = VK_NULL_HANDLE;
    }

    // destroy depth stencil image view
    if(vkImageView_depth_sea) {
        vkDestroyImageView(vkDevice_sea, vkImageView_depth_sea, NULL);
        vkImageView_depth_sea = VK_NULL_HANDLE;
    }

    // destroy depth stencil image
    if(vkImage_depth_sea) {
        vkDestroyImage(vkDevice_sea, vkImage_depth_sea, NULL);
        vkImage_depth_sea = VK_NULL_HANDLE;
    }

    // destroy depth stencil memory
    if(vkDeviceMemory_depth_sea) {
        vkFreeMemory(vkDevice_sea, vkDeviceMemory_depth_sea, NULL);
        vkDeviceMemory_depth_sea = VK_NULL_HANDLE;
    }

    if(swapchainImageView_array_sea) {
        for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
            if(swapchainImageView_array_sea[i]) {
                vkDestroyImageView(vkDevice_sea, swapchainImageView_array_sea[i], NULL);
                swapchainImageView_array_sea[i] = VK_NULL_HANDLE;
            }
        }
    }

    if(swapchainImageView_array_sea) {
        free(swapchainImageView_array_sea);
        swapchainImageView_array_sea = NULL;
    }

    // Destroy vulkan Images
    // VALIDATION USE CASE 4: uncomment the given block to see the error
    /* if(swapchainImage_array_sea) {
        for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
            if(swapchainImage_array_sea[i]) {
                vkDestroyImage(vkDevice_sea, swapchainImage_array_sea[i], NULL);
                swapchainImage_array_sea[i] = VK_NULL_HANDLE;
            }
        }
    } */

    if(swapchainImage_array_sea) {
        free(swapchainImage_array_sea);
        swapchainImage_array_sea = NULL;
    }

    // Destroy Swapchain
    if(vkSwapchainKHR_sea) {
        vkDestroySwapchainKHR(vkDevice_sea, vkSwapchainKHR_sea, NULL);
        vkSwapchainKHR_sea = VK_NULL_HANDLE;
    }

    // RECREATE FOR RESIZE
    // Create Swapchain
    vkResult = createSwapchain(VK_TRUE);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize(): createSwapchain() Failed!.\n");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }

    // Create Swapchain Images & Image Views
    vkResult = createSwapchainImagesAndImageViews();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize(): createSwapchainImagesAndImageViews() Failed!.\n");
        return (vkResult);
    }

    // Create Render Pass
    vkResult = createRenderPass();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize(): createRenderPass() Failed!.\n");
        return (vkResult);
    }

    if(ImGui::GetCurrentContext() != NULL) {
        uint32_t minImageCount = (swapchainImageCount_sea < 2) ? 2 : swapchainImageCount_sea;
        ImGui_ImplVulkan_SetMinImageCount(minImageCount);

        ImGui_ImplVulkan_PipelineInfo pipelineInfoMain = {};
        pipelineInfoMain.RenderPass = vkRenderPass_sea;
        pipelineInfoMain.Subpass = 0;
        pipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        ImGui_ImplVulkan_CreateMainPipeline(&pipelineInfoMain);
    }

    // Create Pipeline Layout
    vkResult = createPipelineLayout();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize(): createPipelineLayout() Failed!.\n");
        return (vkResult);
    }

    // Create Pipeline
    vkResult = createPipeline();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize(): createPipeline() Failed!.\n");
        return (vkResult);
    }

    // Create Framebuffers
    vkResult = createFramebuffers();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize(): createFramebuffers() Failed!.\n");
        return (vkResult);
    }
    
    // Create Command Buffer
    vkResult = createCommandBuffers();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize(): createCommandBuffers() Failed!.\n");
        return (vkResult);
    }

    // Build Command Buffers
    vkResult = buildCommandBuffers();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize(): buildCommandBuffers() Failed!.\n");
        return (vkResult);
    }

    // add extra new line for better readability
    fprintf(fptr, "\n\n");

    bInitialized_sea = TRUE;
    gCommandBuffersDirty = false;

    return (vkResult);
}

VkResult display(void) {

    // Function declarations
    VkResult resize(int, int);
    VkResult updateUniformBuffer(void);
    VkResult buildCommandBuffers(void);
    bool buildImGuiUI(void);

    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Code
    //if control comes here before initialization is done, then return false
    if(bInitialized_sea == FALSE) {
        vkResult = (VkResult)VK_FALSE;
        fprintf(fptr, "display(): bInitialized is FALSE!.\n");
        return (vkResult);
    }

    // Acquire index of next swapchain image
    vkResult = vkAcquireNextImageKHR(
        vkDevice_sea, 
        vkSwapchainKHR_sea,
        UINT64_MAX, // timeout in nanoseconds
        vkSemaphore_backbuffer_sea,
        VK_NULL_HANDLE,
        &currentImageIndex_sea
    );

    if(vkResult != VK_SUCCESS) {
        if(vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR) {
            fprintf(fptr, "display(): vkAcquireNextImageKHR() Failed! Swapchain is out of date.\n");
            // Resize the swapchain
            vkResult = resize(winWidth_sea, winHeight_sea);
            if(vkResult != VK_SUCCESS) {
                fprintf(fptr, "display(): resize() Failed!.\n");
                return (vkResult);
            }
        } else {
            fprintf(fptr, "display(): vkAcquireNextImageKHR() Failed!.\n");
            return (vkResult);
        }
    }

    // Use Fence to allow host to wait for complition of execution of prev command buffer
    vkResult = vkWaitForFences(vkDevice_sea, 1, &vkFence_array_sea[currentImageIndex_sea], VK_TRUE, UINT64_MAX);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "display(): vkWaitForFences() Failed!.\n");
        return (vkResult);
    }

    // Now ready the facnces for execution of next command buffer
    vkResult = vkResetFences(vkDevice_sea, 1, &vkFence_array_sea[currentImageIndex_sea]);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "display(): vkResetFences() Failed!.\n");
        return (vkResult);
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    bool uiChanged = buildImGuiUI();
    ImGui::Render();

    if(uiChanged) {
        gCommandBuffersDirty = true;
    }

    if(gCommandBuffersDirty) {
        vkResult = buildCommandBuffers();
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "display(): buildCommandBuffers() Failed!.\n");
            return (vkResult);
        }

        gCommandBuffersDirty = false;
    }

    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "display(): updateUniformBuffer() Failed!.\n");
        return (vkResult);
    }

    // One of the memeber of vkSubmitInfo structure requires array of pipeline stages, we haveonly one have of 
    // complition of color attachment, so we need to create array of size 1
    const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    // declare, memset & initialize vkSubmitInfo structure
    VkSubmitInfo vkSubmitInfo;
    memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));

    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext = NULL;
    vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &vkSemaphore_backbuffer_sea;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array_sea[currentImageIndex_sea];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &vkSemaphore_rendercomplete_sea;

    // Now submit command buffer to queue for execution
    vkResult = vkQueueSubmit(vkQueue_sea, 1, &vkSubmitInfo, vkFence_array_sea[currentImageIndex_sea]);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "display(): vkQueueSubmit() Failed!.\n");
        return (vkResult);
    }

    // We are going to present rendered image after declaring & initializing vkPresentInfoKHR structure
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));

    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_rendercomplete_sea;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR_sea;
    vkPresentInfoKHR.pImageIndices = &currentImageIndex_sea;
    vkPresentInfoKHR.pResults = NULL; // this is optional, so we are not using it

    // Present the queue!
    vkResult = vkQueuePresentKHR(vkQueue_sea, &vkPresentInfoKHR);
    if(vkResult != VK_SUCCESS) {
        if(vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR) {
            fprintf(fptr, "display(): vkQueuePresentKHR() Failed! Swapchain is out of date.\n");
            // Resize the swapchain
            vkResult = resize(winWidth_sea, winHeight_sea);
            if(vkResult != VK_SUCCESS) {
                fprintf(fptr, "display(): resize() Failed!.\n");
                return (vkResult);
            }
        }
        else {
            fprintf(fptr, "display(): vkQueuePresentKHR() Failed!.\n");
            return (vkResult);
        }
    }

    vkDeviceWaitIdle(vkDevice_sea); // VALIDATION USE CASE 1: Comment this line to see the error

    return (vkResult);
}

void uninitialize(void){
    
    // Function declarations
    void uninitializeImGui(void);

    // to do toggle window full screen!

    if(ghwnd) {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // wait til vkDevice_sea is idle
    if(vkDevice_sea) {
        vkDeviceWaitIdle(vkDevice_sea); // this basically waits on til all the operations are done using the device and then this function call returns
        fprintf(fptr, "\nuninitialize(): vkDeviceWaitIdle is done!\n");
    }

    uninitializeImGui();

    // Destroy Fence
    // VALIDATION USE CASE 3: Comment this line to see the error
    if(vkFence_array_sea) {
        for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
            vkDestroyFence(vkDevice_sea, vkFence_array_sea[i], NULL);
            fprintf(fptr, "uninitialize(): vkDestroyFence() Succeed for {%d}!.\n", i);
            vkFence_array_sea[i] = VK_NULL_HANDLE;
        }
    }

    if(vkFence_array_sea) {
        free(vkFence_array_sea);
        fprintf(fptr, "uninitialize(): freed vkFence_array!.\n");
        vkFence_array_sea = NULL;
    }

    // Destroy Semaphore
    if(vkSemaphore_rendercomplete_sea) {
        vkDestroySemaphore(vkDevice_sea, vkSemaphore_rendercomplete_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroySemaphore() for Render Complete Succeed!\n");
        vkSemaphore_rendercomplete_sea = VK_NULL_HANDLE;
    }

    if(vkSemaphore_backbuffer_sea) {
        vkDestroySemaphore(vkDevice_sea, vkSemaphore_backbuffer_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroySemaphore() for Back Buffer Succeed!\n");
        vkSemaphore_backbuffer_sea = VK_NULL_HANDLE;
    }

    // Destroy Frame Buffers
    if(vkFramebuffer_array_sea) {
        for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
            vkDestroyFramebuffer(vkDevice_sea, vkFramebuffer_array_sea[i], NULL);
            fprintf(fptr, "uninitialize(): vkDestroyFramebuffer() Succeed for {%d}!.\n", i);
            vkFramebuffer_array_sea[i] = VK_NULL_HANDLE;
        }
    }

    if(vkFramebuffer_array_sea) {
        free(vkFramebuffer_array_sea);
        fprintf(fptr, "uninitialize(): freed vkFramebuffer_array!.\n");
        vkFramebuffer_array_sea = NULL;
    }

    // Destroy Pipeline
    if(vkPipeline_sea) {
        vkDestroyPipeline(vkDevice_sea, vkPipeline_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyPipeline() Succeed!\n");
        vkPipeline_sea = VK_NULL_HANDLE;
    }

    // Destroy Render Pass
    if(vkRenderPass_sea) {
        vkDestroyRenderPass(vkDevice_sea, vkRenderPass_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyRenderPass() Succeed!\n");
        vkRenderPass_sea = VK_NULL_HANDLE;
    }

    // Destroy Descriptor Pool
    // When descriptor pool is destroyed, all the descriptor sets created from it are destroyed internally
    // so we  don't need to destroy descriptor set explicitly 
    if(vkDescriptorPool_sea) {
        vkDestroyDescriptorPool(vkDevice_sea, vkDescriptorPool_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDescriptorPool & vkDescriptorSet Destroy Succeed!\n");
        vkDescriptorPool_sea = VK_NULL_HANDLE;
    }

    // Destroy Pipeline Layout
    if(vkPipelineLayout_sea) {
        vkDestroyPipelineLayout(vkDevice_sea, vkPipelineLayout_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyPipelineLayout() Succeed!\n");
        vkPipelineLayout_sea = VK_NULL_HANDLE;
    }

    // Destroy Descriptor Set Layout
    if(vkDescriptorSetLayout_sea) {
        vkDestroyDescriptorSetLayout(vkDevice_sea, vkDescriptorSetLayout_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyDescriptorSetLayout() Succeed!\n");
        vkDescriptorSetLayout_sea = VK_NULL_HANDLE;
    }

    // Destroy Shader
    if(vkShaderModule_fragment_sea) {
        vkDestroyShaderModule(vkDevice_sea, vkShaderModule_fragment_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyShaderModule() Succeed for Fragment Shader!\n");
        vkShaderModule_fragment_sea = VK_NULL_HANDLE;
    }

    if(vkShaderModule_vertex_sea) {
        vkDestroyShaderModule(vkDevice_sea, vkShaderModule_vertex_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyShaderModule() Succeed for Vertex Shader!\n");
        vkShaderModule_vertex_sea = VK_NULL_HANDLE;
    }

    // Destroy Ocean Mask Sampler
    if(vkSampler_oceanMask) {
        vkDestroySampler(vkDevice_sea, vkSampler_oceanMask, NULL);
        fprintf(fptr, "uninitialize(): vkDestroySampler() Succeed for Ocean Mask Sampler!\n");
        vkSampler_oceanMask = VK_NULL_HANDLE;
    }

    // Destroy Ocean Mask Image View
    if(vkImageView_oceanMask) {
        vkDestroyImageView(vkDevice_sea, vkImageView_oceanMask, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyImageView() Succeed for Ocean Mask Image View!\n");
        vkImageView_oceanMask = VK_NULL_HANDLE;
    }

    // Destroy Ocean Mask Image Memory
    if(vkDeviceMemory_oceanMask) {
        vkFreeMemory(vkDevice_sea, vkDeviceMemory_oceanMask, NULL);
        fprintf(fptr, "uninitialize(): vkFreeMemory() Succeed for Ocean Mask Image Memory!\n");
        vkDeviceMemory_oceanMask = VK_NULL_HANDLE;
    }

    // Destroy Ocean Mask Image
    if(vkImage_oceanMask) {
        vkDestroyImage(vkDevice_sea, vkImage_oceanMask, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyImage() Succeed for Ocean Mask Image!\n");
        vkImage_oceanMask = VK_NULL_HANDLE;
    }

    // Destroy Uniform Buffer
    if(uniformData_sea.vkDeviceMemory) {
        vkFreeMemory(vkDevice_sea, uniformData_sea.vkDeviceMemory, NULL);
        fprintf(fptr, "uninitialize(): vkFreeMemory() Succeed for Uniform Buffer!\n");
        uniformData_sea.vkDeviceMemory = VK_NULL_HANDLE;
    }

    if(uniformData_sea.vkBuffer) {
        vkDestroyBuffer(vkDevice_sea, uniformData_sea.vkBuffer, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyBuffer() Succeed for Uniform Buffer!\n");
        uniformData_sea.vkBuffer = VK_NULL_HANDLE;
    }

    // Destroy Vertex Buffer Position
    if(vertexData_position_sea.vkDeviceMemory) {
        vkFreeMemory(vkDevice_sea, vertexData_position_sea.vkDeviceMemory, NULL);
        fprintf(fptr, "uninitialize(): vkFreeMemory() Succeed for Vertex Buffer for Position!\n");
        vertexData_position_sea.vkDeviceMemory = VK_NULL_HANDLE;
    }

    if(vertexData_position_sea.vkBuffer) {
        vkDestroyBuffer(vkDevice_sea, vertexData_position_sea.vkBuffer, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyBuffer() Succeed for Vertex Buffer for Position!\n");
        vertexData_position_sea.vkBuffer = VK_NULL_HANDLE;
    }

    // Destroy  Command Buffers
    if(vkCommandBuffer_array_sea) {
        for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
            vkFreeCommandBuffers(vkDevice_sea, vkCommandPool_sea, 1, &vkCommandBuffer_array_sea[i]);
            fprintf(fptr, "uninitialize(): vkFreeCommandBuffers() Succeed for {%d}\n", i);
            vkCommandBuffer_array_sea[i] = VK_NULL_HANDLE;
        }
    }

    if(vkCommandBuffer_array_sea) {
        free(vkCommandBuffer_array_sea);
        fprintf(fptr, "uninitialize(): freed vkCommandBuffer_array!.\n");
        vkCommandBuffer_array_sea = NULL;
    }

    // Destroy the command pool
    if(vkCommandPool_sea) {
        vkDestroyCommandPool(vkDevice_sea, vkCommandPool_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyCommandPool Successful!.\n");
        vkCommandPool_sea = VK_NULL_HANDLE;
    }

    // destroy depth stencil image view
    if(vkImageView_depth_sea) {
        vkDestroyImageView(vkDevice_sea, vkImageView_depth_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyImageView() Succeed for Depth Stencil Image View!\n");
        vkImageView_depth_sea = VK_NULL_HANDLE;
    }

    // destroy depth stencil image
    if(vkImage_depth_sea) {
        vkDestroyImage(vkDevice_sea, vkImage_depth_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyImage() Succeed for Depth Stencil Image!\n");
        vkImage_depth_sea = VK_NULL_HANDLE;
    }

    // destroy depth stencil memory
    if(vkDeviceMemory_depth_sea) {
        vkFreeMemory(vkDevice_sea, vkDeviceMemory_depth_sea, NULL);
        fprintf(fptr, "uninitialize(): vkFreeMemory() Succeed for Depth Stencil Memory!\n");
        vkDeviceMemory_depth_sea = VK_NULL_HANDLE;
    }

    // Destroy Vulkan Swapchain Image Views
    if(swapchainImageView_array_sea) {
        for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
            if(swapchainImageView_array_sea[i]) {
                vkDestroyImageView(vkDevice_sea, swapchainImageView_array_sea[i], NULL);
                fprintf(fptr, "uninitialize(): vkDestroyImageView() Succeed for {%d}\n", i);
                swapchainImageView_array_sea[i] = VK_NULL_HANDLE;
            }
        }
    }

    if(swapchainImageView_array_sea) {
        free(swapchainImageView_array_sea);
        fprintf(fptr, "uninitialize(): freed swapchainImageView_array!.\n");
        swapchainImageView_array_sea = NULL;
    }

    // Destroy vulkan Images
    // VALIDATION USE CASE 4: uncomment the given block to see the error
    /* if(swapchainImage_array_sea) {
        for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
            if(swapchainImage_array_sea[i]) {
                vkDestroyImage(vkDevice_sea, swapchainImage_array_sea[i], NULL);
                fprintf(fptr, "uninitialize(): vkDestroyImage() Succeed for {%d}\n", i);
                fflush(fptr);
                swapchainImage_array_sea[i] = VK_NULL_HANDLE;
            }
        }
    } */

    if(swapchainImage_array_sea) {
        free(swapchainImage_array_sea);
        fprintf(fptr, "uninitialize(): freed swapchainImage_array!.\n");
        swapchainImage_array_sea = NULL;
    }


    // Destroy Vulkan Swapchain
    if(vkSwapchainKHR_sea) {
        vkDestroySwapchainKHR(vkDevice_sea, vkSwapchainKHR_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroySwapchainKHR() Succeed!\n");
        vkSwapchainKHR_sea = VK_NULL_HANDLE;
    }
    
    // No need to destroy device queue

    // Destroy Vulkan Device
    if(vkDevice_sea) {
        vkDestroyDevice(vkDevice_sea, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyDevice() Succeed!\n");
        vkDevice_sea = VK_NULL_HANDLE;
    }
    
    //No need to destroy selected physical device!

    // destroy surface
    if(vkSurfaceKHR_sea) {
        vkDestroySurfaceKHR(vkInstance_sea, vkSurfaceKHR_sea, NULL);
        vkSurfaceKHR_sea = VK_NULL_HANDLE;
		fprintf(fptr,"uninitialize(): vkDestroySurfaceKHR() Succeed\n");
    }

    if(vkDebugReportCallbackEXT_sea && vkDestroyDebugReportCallbackEXT_fnptr_sea) {
        vkDestroyDebugReportCallbackEXT_fnptr_sea(vkInstance_sea, vkDebugReportCallbackEXT_sea, NULL);
        vkDebugReportCallbackEXT_sea = VK_NULL_HANDLE;
        vkDestroyDebugReportCallbackEXT_fnptr_sea = NULL;
        fprintf(fptr,"uninitialize(): vkDestroyDebugReportCallbackEXT_fnptr() Succeed\n");
    }

    // destroy vkInstance_sea
    if(vkInstance_sea) {
        vkDestroyInstance(vkInstance_sea, NULL);
        vkInstance_sea = VK_NULL_HANDLE;
		fprintf(fptr,"uninitialize(): vkDestroyInstance() Succeed\n");
    }

	if(fptr){
		fprintf(fptr,"uninitialize(): File Closed Successfully..\n");
        fclose(fptr);
		fptr = NULL;
	}
}

void update(void) {
    // frame time, so transitions are frame-rate independent
    static double lastTime = myClock_sea.getElapsedTime();
    double currentTime = myClock_sea.getElapsedTime();
    float deltaTime = (float)glm::clamp(currentTime - lastTime, 0.0, 0.1);
    lastTime = currentTime;

    // ease sphere blend towards its target
    if(gSeaUiState.sphereBlend < gSphereBlendTarget) {
        gSeaUiState.sphereBlend = glm::min(gSeaUiState.sphereBlend + gSphereBlendSpeed * deltaTime, gSphereBlendTarget);
    } else if(gSeaUiState.sphereBlend > gSphereBlendTarget) {
        gSeaUiState.sphereBlend = glm::max(gSeaUiState.sphereBlend - gSphereBlendSpeed * deltaTime, gSphereBlendTarget);
    }

    // ease bronze blend towards its target
    if(gSeaUiState.bronzeBlend < gBronzeBlendTarget) {
        gSeaUiState.bronzeBlend = glm::min(gSeaUiState.bronzeBlend + gBronzeBlendSpeed * deltaTime, gBronzeBlendTarget);
    } else if(gSeaUiState.bronzeBlend > gBronzeBlendTarget) {
        gSeaUiState.bronzeBlend = glm::max(gSeaUiState.bronzeBlend - gBronzeBlendSpeed * deltaTime, gBronzeBlendTarget);
    }
}

//! //////////////////////////////////////// Definations of vulkan Related Functions ///////////////////////////////////////////////

VkResult createVulkanInstance (void) {
    // function declarations
    VkResult fillInstanceExtensionNames(void);
    VkResult fillValidationLayerNames(void);
    VkResult createValidationCallbackFunction(void);

    // varibales
    VkResult vkResult = VK_SUCCESS;

    // code
    vkResult = fillInstanceExtensionNames();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVulkanInstance(): fillInstanceExtensionNames() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVulkanInstance(): fillInstanceExtensionNames() Successful!.\n\n");
    }

    if(bValidation_sea == TRUE) {
        //fill validation layers names
        vkResult = fillValidationLayerNames();
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "createVulkanInstance(): fillValidationLayerNames() Failed!.\n");
            return (vkResult);
        } else {
            fprintf(fptr, "createVulkanInstance(): fillValidationLayerNames() Successful!.\n");
        }
    }

    // step 2:
    VkApplicationInfo vkApplicationInfo;
    memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.pNext = NULL;
    vkApplicationInfo.pApplicationName = gpszAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpszAppName;
    vkApplicationInfo. engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_3;  // change it VK_API_VERSION_1_4 once you update vulkan

    // Step 3: initialize struct VkInstanceCreateInfo
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount_sea;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array_sea;

    // if validation layer is enabled/valid then fill data else keep it null
    if(bValidation_sea == TRUE) {
        vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount_sea;
        vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_array_sea;
    } else {
        vkInstanceCreateInfo.enabledLayerCount = 0;
        vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
    }

    // Step 4: Create instance using vkCreateInstance
    vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance_sea);
    if(vkResult == VK_ERROR_INCOMPATIBLE_DRIVER) {
        fprintf(fptr, "createVulkanInstance(): vkCreateInstance() Failed Due to Incompatible Driver (%d)!.\n", vkResult);
        return (vkResult);
    } else if(vkResult == VK_ERROR_EXTENSION_NOT_PRESENT) {
        fprintf(fptr, "createVulkanInstance(): vkCreateInstance() Failed Due to Extention Not Present (%d)!.\n", vkResult);
        return (vkResult);
    } else if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVulkanInstance(): vkCreateInstance() Failed Due to Unknown Reason (%d)!.\n", vkResult);
        return (vkResult);
    } else {
        fprintf(fptr, "createVulkanInstance(): vkCreateInstance() Successful!.\n\n");
    }

    // Step 5: Create Validation Layer Callback Function [ Do this for validation callbaaks ]
    if(bValidation_sea == TRUE) {
        vkResult = createValidationCallbackFunction();
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "createVulkanInstance(): createValidationCallbackFunction() Failed!.\n");
            return (vkResult);
        } else {
            fprintf(fptr, "createVulkanInstance(): createValidationCallbackFunction() Successful!.\n\n");
        }
    }

    return (vkResult);

}

VkResult fillInstanceExtensionNames (void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Step 1: Find how many instance extension are supported by this vulkan driver & keep it in local variable
    uint32_t instanceExtensionCount = 0;

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "fillInstanceExtensionNames(): vkEnumerateInstanceExtensionProperties() First Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillInstanceExtensionNames(): vkEnumerateInstanceExtensionProperties() First Call Successful!.\n");
    }

    // step 2: Allocate & fill struct vk Extenstions array correspoinding to above acount
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "fillInstanceExtensionNames(): vkEnumerateInstanceExtensionProperties() Second Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillInstanceExtensionNames(): vkEnumerateInstanceExtensionProperties() Second Call Successful!.\n");
    }

    // Step 3: fill all supoorted extensions names in array of char pointers
    char **instanceExtensionNames_array = NULL;
    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
    for(uint32_t i = 0; i < instanceExtensionCount; i++) {
        instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(
            instanceExtensionNames_array[i], 
            vkExtensionProperties_array[i].extensionName, 
            strlen(vkExtensionProperties_array[i].extensionName) + 1
        );
        fprintf(fptr, "fillInstanceExtensionNames(): Vulkan Extension Name = %s \n", instanceExtensionNames_array[i]);
    }

    // step 4:
    free(vkExtensionProperties_array);

    // step 5
    VkBool32 surfaceExtensionFound = VK_FALSE;
    VkBool32 win32vulkanSurfaceExtensionFound = VK_FALSE;
    VkBool32 debugReportExtensionFound = VK_FALSE;
    for(uint32_t i = 0; i < instanceExtensionCount; i++) {
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0) {
            surfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array_sea[enabledInstanceExtensionCount_sea++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) ==  0) {
            win32vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array_sea[enabledInstanceExtensionCount_sea++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }
        if(strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) ==  0) {
            debugReportExtensionFound = VK_TRUE;
            if(bValidation_sea == TRUE) {
                enabledInstanceExtensionNames_array_sea[enabledInstanceExtensionCount_sea++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            } else {
                // array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
            }
        }
    }

    // step 6
    for(uint32_t i = 0; i < instanceExtensionCount; i++) {
        free(instanceExtensionNames_array[i]);
    }
    free(instanceExtensionNames_array);

    // step 7:
    if(surfaceExtensionFound == VK_FALSE) {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(fptr, "fillInstanceExtensionNames(): VK_KHR_SURFACE_EXTENSION_NAME Not Found!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillInstanceExtensionNames(): VK_KHR_SURFACE_EXTENSION_NAME Found!.\n");
    }

    if(win32vulkanSurfaceExtensionFound == VK_FALSE) {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(fptr, "fillInstanceExtensionNames(): VK_KHR_WIN32_SURFACE_EXTENSION_NAME Not Found!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillInstanceExtensionNames(): VK_KHR_WIN32_SURFACE_EXTENSION_NAME Found!.\n");
    }

    if(debugReportExtensionFound == VK_FALSE) {
        if(bValidation_sea == TRUE) {
            vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
            fprintf(fptr, "fillInstanceExtensionNames(): Validation is ON but VK_EXT_DEBUG_REPORT_EXTENSION_NAME Not Supported!.\n");
            return (vkResult);
        } else {
            fprintf(fptr, "fillInstanceExtensionNames(): Validation is OFF and VK_EXT_DEBUG_REPORT_EXTENSION_NAME Not Supported!.\n");
        }
    } else {
        if(bValidation_sea == TRUE) {
            fprintf(fptr, "fillInstanceExtensionNames(): Validation is ON but VK_EXT_DEBUG_REPORT_EXTENSION_NAME is Supported!.\n");
        } else {
            fprintf(fptr, "fillInstanceExtensionNames(): Validation is OFF and VK_EXT_DEBUG_REPORT_EXTENSION_NAME is Supported!.\n");
        }
    }

    // step 8: print all the supported extensions
    for(uint32_t i = 0; i < enabledInstanceExtensionCount_sea; i++) {
        fprintf(fptr, "fillInstanceExtensionNames(): Enabled Vulkan Instance Extension Name = %s \n", enabledInstanceExtensionNames_array_sea[i]);
    }

    return vkResult;
}

VkResult fillValidationLayerNames(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // step 1: Find how many validation layers are supported by this vulkan driver & keep it in local variable
    uint32_t validationLayerCount = 0;

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "fillValidationLayerNames(): vkEnumerateInstanceLayerProperties() First Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillValidationLayerNames(): vkEnumerateInstanceLayerProperties() First Call Successful!.\n");
    }

    // step 2: Allocate & fill struct vk Validation Layers array correspoinding to above acount
    VkLayerProperties *vkLayerProperties_array = NULL;
    vkLayerProperties_array = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * validationLayerCount);
    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "fillValidationLayerNames(): vkEnumerateInstanceLayerProperties() Second Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillValidationLayerNames(): vkEnumerateInstanceLayerProperties() Second Call Successful!.\n");
    }

    // step 3: fill all supoorted layers names in array of char pointers
    char **validationLayerNames_array = NULL;
    validationLayerNames_array = (char**)malloc(sizeof(char*) * validationLayerCount);
    for(uint32_t i = 0; i < validationLayerCount; i++) {
        validationLayerNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1);
        memcpy(
            validationLayerNames_array[i], 
            vkLayerProperties_array[i].layerName, 
            strlen(vkLayerProperties_array[i].layerName) + 1
        );
        fprintf(fptr, "fillValidationLayerNames(): Vulkan Validation Layer Name = %s \n", validationLayerNames_array[i]);
    }

    // step 4: free vkLayerProperties_array
    free(vkLayerProperties_array);

    // step 5: check if validation layer is supported or not
    VkBool32 validationLayerFound = VK_FALSE;
    for(uint32_t i = 0; i < validationLayerCount; i++) {
        if(strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0) {
            validationLayerFound = VK_TRUE;
            enabledValidationLayerNames_array_sea[enabledValidationLayerCount_sea++] = "VK_LAYER_KHRONOS_validation";
        }
    }

    // step 6
    for(uint32_t i = 0; i < validationLayerCount; i++) {
        free(validationLayerNames_array[i]);
    }
    free(validationLayerNames_array);

    // step 7
    if(validationLayerFound == VK_FALSE) {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(fptr, "fillValidationLayerNames(): VK_LAYER_KHRONOS_validation Not Supported!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillValidationLayerNames(): VK_LAYER_KHRONOS_validation Supported!.\n");
    }

    // step 8: print all the supported layers
    for(uint32_t i = 0; i < enabledValidationLayerCount_sea; i++) {
        fprintf(fptr, "fillValidationLayerNames(): Enabled Vulkan Validation Layer Name = %s \n", enabledValidationLayerNames_array_sea[i]);
    }

    return (vkResult);
}

VkResult createValidationCallbackFunction(void) {
    // function declaratons
    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
        VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT,
        uint64_t, size_t, int32_t, const char*, const char*,
        void*
    );

    // variables
    VkResult vkResult = VK_SUCCESS;
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

    // code
    // get the required function pointers
    vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance_sea, "vkCreateDebugReportCallbackEXT");
    if(vkCreateDebugReportCallbackEXT_fnptr == NULL) {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(fptr, "createValidationCallbackFunction(): vkGetInstanceProcAddr() for vkCreateDebugReportCallbackEXT Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createValidationCallbackFunction(): vkGetInstanceProcAddr() for vkCreateDebugReportCallbackEXT Successful!.\n");
    }

    vkDestroyDebugReportCallbackEXT_fnptr_sea = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance_sea, "vkDestroyDebugReportCallbackEXT");
    if(vkCreateDebugReportCallbackEXT_fnptr == NULL) {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(fptr, "createValidationCallbackFunction(): vkGetInstanceProcAddr() for vkDestroyDebugReportCallbackEXT Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createValidationCallbackFunction(): vkGetInstanceProcAddr() for vkDestroyDebugReportCallbackEXT Successful!.\n");
    }

    // fill struct VkDebugReportCallbackCreateInfoEXT to get vulkan debug report callback object
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
    memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));

    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;

    vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance_sea, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createValidationCallbackFunction(): vkCreateDebugReportCallbackEXT_fnptr() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createValidationCallbackFunction(): vkCreateDebugReportCallbackEXT_fnptr() Successful!.\n");
    }

    return (vkResult);
}

// get supported surface
VkResult getSupportedSurface(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    //code
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
    memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    // vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetModuleHandle(NULL);
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

    vkResult = vkCreateWin32SurfaceKHR(
        vkInstance_sea, 
        &vkWin32SurfaceCreateInfoKHR,
        NULL,
        &vkSurfaceKHR_sea
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getSupportedSurface(): vkCreateWin32SurfaceKHR() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "getSupportedSurface(): vkCreateWin32SurfaceKHR() Successful!.\n");
    }


    return vkResult;
}

VkResult getPhysicalDevice() {
    // variables
    VkResult vkResult = VK_SUCCESS;

    //code
    vkResult = vkEnumeratePhysicalDevices(vkInstance_sea, &physicalDeviceCount_sea, NULL);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() First Call Failed!.\n");
        return (vkResult);
    } else if(physicalDeviceCount_sea == 0) {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() Resulted in Zero Physical Devices!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() First Call Successful!.\n");
    }

    vkPhysicalDevice_array_sea = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount_sea);

    vkResult = vkEnumeratePhysicalDevices(vkInstance_sea, &physicalDeviceCount_sea, vkPhysicalDevice_array_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() Second Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() Second Call Successful!.\n");
    }

    VkBool32 bFound = VK_FALSE;

    for(uint32_t i = 0; i < physicalDeviceCount_sea; i++) {
        uint32_t queueCount = UINT32_MAX;

        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array_sea[i], &queueCount, NULL);
        VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array_sea[i], &queueCount, vkQueueFamilyProperties_array);

        VkBool32 *isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * queueCount);

        for(uint32_t j = 0; j < queueCount; j++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(
                vkPhysicalDevice_array_sea[i],
                j,
                vkSurfaceKHR_sea,
                &isQueueSurfaceSupported_array[j]
            );
        }

        for(uint32_t j = 0; j < queueCount; j++) {
            if(vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT
                && isQueueSurfaceSupported_array[j] == VK_TRUE) {
                vkPhysicalDevice_selected_sea = vkPhysicalDevice_array_sea[i];
                graphicsQueueFamilyIndex_selected_sea = j;
                bFound = VK_TRUE;
                break;
            }
        }

        if(isQueueSurfaceSupported_array) {
            free(isQueueSurfaceSupported_array);
            isQueueSurfaceSupported_array = NULL;
            fprintf(fptr, "getPhysicalDevice(): freed isQueueSurfaceSupported_array!.\n");
        }
        
        if(vkQueueFamilyProperties_array) {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
            fprintf(fptr, "getPhysicalDevice(): freed vkQueueFamilyProperties_array!.\n");
        }

        if(bFound == VK_TRUE) {
            break;
        }
    }


    if(bFound == VK_TRUE) {
        fprintf(fptr, "getPhysicalDevice(): Successful to get required graphics enabled physical device!.\n");
    } else {
        if(vkPhysicalDevice_array_sea) {
            free(vkPhysicalDevice_array_sea);
            vkPhysicalDevice_array_sea = NULL;
            fprintf(fptr, "getPhysicalDevice(): freed vkPhysicalDevice_array!.\n");
        }
        fprintf(fptr, "getPhysicalDevice(): Failed to get required graphics enabled physical device!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    memset((void*)&vkPhysicalDeviceMemoryProperties_sea, 0, sizeof(VkPhysicalDeviceMemoryProperties));

    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected_sea, &vkPhysicalDeviceMemoryProperties_sea);

    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected_sea, &vkPhysicalDeviceFeatures);

    if(vkPhysicalDeviceFeatures.tessellationShader == VK_TRUE) {
        fprintf(fptr, "getPhysicalDevice(): Selected Physical Device Supports Tessellation Shader!.\n");
    } else {
        fprintf(fptr, "getPhysicalDevice(): Selected Physical Device Does Not Supports Tessellation Shader!.\n");
    }

    if(vkPhysicalDeviceFeatures.geometryShader == VK_TRUE) {
        fprintf(fptr, "getPhysicalDevice(): Selected Physical Device Supports Geometry Shader!.\n");
    } else {
        fprintf(fptr, "getPhysicalDevice(): Selected Physical Device Does Not Supports Geometry Shader!.\n");
    }

    return (vkResult);
}

VkResult printVKInfo (void) {
    // varibales
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(fptr, "printVKInfo(): Printing Vulkan Info: \n\n");

    for(uint32_t i = 0; i < physicalDeviceCount_sea; i++) {

        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));

        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array_sea[i], &vkPhysicalDeviceProperties);

        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);

        fprintf(fptr, "Physical Device [%d] Properties: \n", i);
        // API Version
        fprintf(fptr, "API Version: %d.%d.%d\n", majorVersion, minorVersion, patchVersion);
        //Device Name
        fprintf(fptr, "Device Name: %s\n", vkPhysicalDeviceProperties.deviceName);
        
        switch(vkPhysicalDeviceProperties.deviceType) {
    
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                fprintf(fptr, "Device Type: Integrated GPU (iGPU)\n");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                fprintf(fptr, "Device Type: Discrete GPU (dGPU)\n");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                fprintf(fptr, "Device Type: Virtual GPU (vGPU)\n");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                fprintf(fptr, "Device Type: GPU\n");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                fprintf(fptr, "Device Type: Nor iGPU, dGPU, vGPU, or CPU, Something Other\n");
                break;

            default:
                fprintf(fptr, "Device Type: Unknown\n");
                break;
        }

        // Vendor ID
        fprintf(fptr, "Vendor ID: 0x%04x\n", vkPhysicalDeviceProperties.vendorID);

        // Device ID
        fprintf(fptr, "Device ID: 0x%04x\n", vkPhysicalDeviceProperties.deviceID);

        fprintf(fptr, "\n");
    }

    if(vkPhysicalDevice_array_sea) {
        free(vkPhysicalDevice_array_sea);
        vkPhysicalDevice_array_sea = NULL;
        fprintf(fptr, "printVKInfo(): freed vkPhysicalDevice_array!.\n");
    }

    return (vkResult);
}

VkResult fillDeviceExtensionNames (void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Step 1: Find how many devices extension are supported by this vulkan driver & keep it in local variable
    uint32_t devicesExtensionCount = 0;

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected_sea, NULL, &devicesExtensionCount, NULL);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "fillDeviceExtensionNames(): vkEnumerateDeviceExtensionProperties() First Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillDeviceExtensionNames(): vkEnumerateDeviceExtensionProperties() First Call Successful!.\n");
    }

    // step 2: Allocate & fill struct vk Extenstions array correspoinding to above count
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * devicesExtensionCount);
    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected_sea, NULL, &devicesExtensionCount, vkExtensionProperties_array);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "fillDeviceExtensionNames(): vkEnumerateDeviceExtensionProperties() Second Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillDeviceExtensionNames(): vkEnumerateDeviceExtensionProperties() Second Call Successful!.\n");
    }

    // Step 3: 
    char **deviceExtensionNames_array = NULL;
    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * devicesExtensionCount);
    fprintf(fptr, "fillDeviceExtensionNames(): Vulkan Device Extension Count = %d \n", devicesExtensionCount);
    for(uint32_t i = 0; i < devicesExtensionCount; i++) {
        deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(
            deviceExtensionNames_array[i], 
            vkExtensionProperties_array[i].extensionName, 
            strlen(vkExtensionProperties_array[i].extensionName) + 1
        );
        fprintf(fptr, "fillDeviceExtensionNames(): Vulkan Device Extension Name = %s \n", deviceExtensionNames_array[i]);
    }

    fprintf(fptr, "\n");


    // step 4:
    free(vkExtensionProperties_array);

    // step 5
    VkBool32 vulkanSwapchainExtensionFound = VK_FALSE;
    for(uint32_t i = 0; i < devicesExtensionCount; i++) {
        if(strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
            vulkanSwapchainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array_sea[enabledDeviceExtensionCount_sea++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
    }

    // step 6
    for(uint32_t i = 0; i < devicesExtensionCount; i++) {
        free(deviceExtensionNames_array[i]);
    }
    free(deviceExtensionNames_array);

    // step 7:
    if(vulkanSwapchainExtensionFound == VK_FALSE) {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(fptr, "fillDeviceExtensionNames(): VK_KHR_SWAPCHAIN_EXTENSION_NAME Not Found!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillDeviceExtensionNames(): VK_KHR_SWAPCHAIN_EXTENSION_NAME Found!.\n");
    }

    // step 8:
    for(uint32_t i = 0; i < enabledDeviceExtensionCount_sea; i++) {
        fprintf(fptr, "fillDeviceExtensionNames(): Enabled Vulkan Device Extension Name = %s \n", enabledDeviceExtensionNames_array_sea[i]);
    }

    return vkResult;
}

VkResult createVulkanDevice () {
    
    // function definations
    VkResult fillDeviceExtensionNames(void);

    //variables
    VkResult vkResult = VK_SUCCESS;

    vkResult = fillDeviceExtensionNames();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVulkanDevice(): fillDeviceExtensionNames() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVulkanDevice(): fillDeviceExtensionNames() Successful!.\n");
    }

    // !NEWLY ADDED CODE : intialize VkDeviceQueueCreateInfo
    float queuePriorities[] = { 1.0f };
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));

    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = 0;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected_sea;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    // initialize VkDeviceCreateInfo structure
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount_sea;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array_sea;
    vkDeviceCreateInfo.enabledLayerCount = 0; // these are deprecated in current version
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL; // these are deprecated in current version
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    // !NEWLY ADDED CODE : set VkDeviceQueueCreateInfo
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

    vkResult = vkCreateDevice(
        vkPhysicalDevice_selected_sea,
        &vkDeviceCreateInfo,
        NULL, &vkDevice_sea
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVulkanDevice(): vkCreateDevice() Failed!. (%d)\n", vkResult);
        return (vkResult);
    } else {
        fprintf(fptr, "createVulkanDevice(): vkCreateDevice() Successful!.\n");
    }

    return(vkResult);
}

void getDeviceQueue (void) {

    vkGetDeviceQueue(
        vkDevice_sea,
        graphicsQueueFamilyIndex_selected_sea,
        0, &vkQueue_sea
    );

    if(vkQueue_sea == VK_NULL_HANDLE) {
        fprintf(fptr, "getDeviceQueue(): vkGetDeviceQueue() Failed!.\n");
    } else {
        fprintf(fptr, "getDeviceQueue(): vkGetDeviceQueue() Successful!.\n\n");
    }

}

VkResult getPhysicalDeviceSurfaceFormatAndColorSpace (void) {
    
    //variables
    VkResult vkResult = VK_SUCCESS;
    uint32_t formatCount = 0;

    // code

    // get the count of supported color formats
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
        vkPhysicalDevice_selected_sea, 
        vkSurfaceKHR_sea, &formatCount,
        NULL
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDeviceSurfaceFormatAndColorSpace(): vkGetPhysicalDeviceSurfaceFormatsKHR() frist Call Failed!.\n");
    } else if(formatCount == 0) {
        fprintf(fptr, "getPhysicalDeviceSurfaceFormatAndColorSpace(): vkGetPhysicalDeviceSurfaceFormatsKHR() Failed: 0 supported formats found!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    } else {
        fprintf(fptr, "getPhysicalDeviceSurfaceFormatAndColorSpace(): vkGetPhysicalDeviceSurfaceFormatsKHR() first Call Successful!. [Found %d Formats]\n", formatCount);
    }

    VkSurfaceFormatKHR *vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    
    // fill the allocated array with supported formats
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
        vkPhysicalDevice_selected_sea, 
        vkSurfaceKHR_sea, &formatCount,
        vkSurfaceFormatKHR_array
    );
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDeviceSurfaceFormatAndColorSpace(): vkGetPhysicalDeviceSurfaceFormatsKHR() Second Call Failed!.\n");
    } else {
        fprintf(fptr, "getPhysicalDeviceSurfaceFormatAndColorSpace(): vkGetPhysicalDeviceSurfaceFormatsKHR() Second Call Successful!.\n");
    }

    // Decide the surface color format first!
    if(formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED) {
        vkFormat_color_sea = VK_FORMAT_B8G8R8G8_422_UNORM;
    } else {
        vkFormat_color_sea = vkSurfaceFormatKHR_array[0].format;
    }

    // Decide the Color Space
    vkColorSpaceKHR_sea = vkSurfaceFormatKHR_array[0].colorSpace;

    if(vkSurfaceFormatKHR_array) {
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        fprintf(fptr, "getPhysicalDeviceSurfaceFormatAndColorSpace(): vkSurfaceFormatKHR_array freed.\n");
    }

    return (vkResult);
}

VkResult getPhysicalDeviceSurfacePresentMode(void) {

    // Variables
    VkResult vkResult = VK_SUCCESS;
    uint32_t modeCount = 0;

    //code
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(
        vkPhysicalDevice_selected_sea, 
        vkSurfaceKHR_sea, &modeCount,
        NULL
    );
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): vkGetPhysicalDeviceSurfacePresentModesKHR() frist Call Failed!.\n");
    } else if(modeCount == 0) {
        fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): vkGetPhysicalDeviceSurfacePresentModesKHR() Failed: 0 supported modes found!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    } else {
        fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): vkGetPhysicalDeviceSurfacePresentModesKHR() first Call Successful!. [Found %d Present Modes]\n", modeCount);
    }

    VkPresentModeKHR *vkPresentModeKHR_array = (VkPresentModeKHR*)malloc(modeCount * sizeof(VkPresentModeKHR));

    // fill the allocated array with supported present modes
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(
        vkPhysicalDevice_selected_sea, 
        vkSurfaceKHR_sea, &modeCount,
        vkPresentModeKHR_array
    );
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): vkGetPhysicalDeviceSurfacePresentModesKHR() Second Call Failed!.\n");
    } else {
        fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): vkGetPhysicalDeviceSurfacePresentModesKHR() Second Call Successful!.\n");
    }

    for(uint32_t i = 0 ;  i < modeCount; i++) {
        if(vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            vkPresentModeKHR_sea = vkPresentModeKHR_array[i];
            fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): VK_PRESENT_MODE_MAILBOX_KHR Present Mode found!.\n");
            break;
        }
    }

    if(vkPresentModeKHR_sea != VK_PRESENT_MODE_MAILBOX_KHR) {
        // since we don't have mailbox as supported format let's settle for FIFO then!
        vkPresentModeKHR_sea = VK_PRESENT_MODE_FIFO_KHR;
        fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): Present Mode set to VK_PRESENT_MODE_FIFO_KHR!.\n");
    }

    if(vkPresentModeKHR_array) {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): vkPresentModeKHR_array freed.\n");
    }

    return (vkResult);
}

VkResult createSwapchain (VkBool32 vSync) {

    // Functions
    VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult getPhysicalDeviceSurfacePresentMode(void);

    // Variables
    VkResult vkResult = VK_SUCCESS;
    
    // Code
    // Surface Color & Color Space
    vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchain(): getPhysicalDeviceSurfaceFormatAndColorSpace() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchain(): getPhysicalDeviceSurfaceFormatAndColorSpace() Successful!.\n");
    }

    // Get Physical Device Surface Capabilities
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected_sea, vkSurfaceKHR_sea, &vkSurfaceCapabilitiesKHR);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchain(): vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchain(): vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Successful!.\n");
    }

    // Decide Image Count of Swapchain using minImageCount & maxImageCount from vkSurfaceCapabilitiesKHR
    uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
    uint32_t desiredNumberOfSwapchainImages = 0;

    if(vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages) {
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
    } else {
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
    }

    fprintf(
        fptr, 
        "createSwapchain(): desiredNumberOfSwapchainImages is : %d, [Min: %d, Max: %d]\n", 
        desiredNumberOfSwapchainImages,  vkSurfaceCapabilitiesKHR.minImageCount,  
        vkSurfaceCapabilitiesKHR.maxImageCount
    );

    // Decide Size of Swapchain Image using currentExtent Size & window Size
    memset((void*)&vkExtent2D_swapchain_sea, 0, sizeof(VkExtent2D));

    if(vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX) {
        vkExtent2D_swapchain_sea.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_swapchain_sea.height = vkSurfaceCapabilitiesKHR.currentExtent.height;

        fprintf(
            fptr, 
            "createSwapchain(): Swapchain Image Width : %d X Height : %d\n", 
            vkExtent2D_swapchain_sea.width, vkExtent2D_swapchain_sea.height
        );
    } else {
        // if surface size is already defined then swapchain image size must match with it!
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));

        vkExtent2D.width = (uint32_t)winWidth_sea;
        vkExtent2D.height = (uint32_t)winHeight_sea;

        vkExtent2D_swapchain_sea.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
        vkExtent2D_swapchain_sea.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));

        fprintf(
            fptr, 
            "createSwapchain(): Swapchain Image (Derived from best of minImageExtent, maxImageExtent & Window Size) Width  : %d X Height : %d\n", 
            vkExtent2D_swapchain_sea.width, vkExtent2D_swapchain_sea.height
        );
    }

    // Set Swapchain Image Usage Flag
    VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    // Whether to Consider Pre-Transform/Flipping or Not
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;

    if(vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
    }

    // Physical Device Presentation Mode
    vkResult = getPhysicalDeviceSurfacePresentMode();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchain(): getPhysicalDeviceSurfacePresentMode() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchain(): getPhysicalDeviceSurfacePresentMode() Successful!.\n");
    }

    // Initalize VkSwapchainCreateInfoKHR
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));

    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR_sea;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color_sea;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR_sea;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain_sea.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain_sea.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR_sea;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    vkResult = vkCreateSwapchainKHR(vkDevice_sea, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchain(): vkCreateSwapchainKHR() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchain(): vkCreateSwapchainKHR() Successful!.\n");
    }

    return (vkResult);
}

VkResult createSwapchainImagesAndImageViews(void) {
    // Function Definations
    VkResult getSupportedDepthFormat(void);

    // variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Step 1: Get Swapchain Image Count
    vkResult = vkGetSwapchainImagesKHR(vkDevice_sea, vkSwapchainKHR_sea, &swapchainImageCount_sea, NULL);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() First Call Failed!.\n");
        return (vkResult);
    } else if( swapchainImageCount_sea == 0) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() Failed: 0 Swapchain Images found!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() Successful!. : Swapchain Image Count : [%d]\n", swapchainImageCount_sea);
    }

    // Step 2: Allocate Swapchain Image Array
    swapchainImage_array_sea = (VkImage*)malloc(sizeof(VkImage) * swapchainImageCount_sea);

    // Step 3: Fill Swapchain Image Array
    vkResult = vkGetSwapchainImagesKHR(vkDevice_sea, vkSwapchainKHR_sea, &swapchainImageCount_sea, swapchainImage_array_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() Second Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() Second Call Successful!.\n");
    }

    // Setp 4: Allocate Swapchain Image Views Array
    swapchainImageView_array_sea = (VkImageView*)malloc(sizeof(VkImageView) * swapchainImageCount_sea);

    // Step 5: vkCreateImageView for each Swapchain Image
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_color_sea;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    // Step 6: Fill Imafe view Array  using above struct
    for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
        vkImageViewCreateInfo.image = swapchainImage_array_sea[i];
        vkResult = vkCreateImageView(vkDevice_sea, &vkImageViewCreateInfo, NULL, &swapchainImageView_array_sea[i]);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "createSwapchainImagesAndImageViews(): vkCreateImageView() Failed at {%d}!.\n", i);
            return (vkResult);
        } else {
            fprintf(fptr, "createSwapchainImagesAndImageViews(): vkCreateImageView() Successful for {%d}!.\n", i);
        }
    }

    // For Depth Image

    vkResult = getSupportedDepthFormat();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): getSupportedDepthFormat() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): getSupportedDepthFormat() Successful!.\n");
    }

    // For depth image initialize VkImageCreateInfo
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));

    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = vkFormat_depth_sea;
    vkImageCreateInfo.extent.width = winWidth_sea;
    vkImageCreateInfo.extent.height = winHeight_sea;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vkResult = vkCreateImage(vkDevice_sea, &vkImageCreateInfo, NULL, &vkImage_depth_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkCreateImage() Failed for Depth Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkCreateImage() Successful for Depth Image!.\n");
    }

    // Memory Requirements for Depth Image
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetImageMemoryRequirements(vkDevice_sea, vkImage_depth_sea, &vkMemoryRequirements);

    // Step 6
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // this will be set in next step

    // Step A 
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties_sea.memoryTypeCount; i++) {
        // Step B
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1) {
            // Step C
            if(vkPhysicalDeviceMemoryProperties_sea.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                // Step D
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        // Step E
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //Setp 9
    vkResult = vkAllocateMemory(vkDevice_sea, &vkMemoryAllocateInfo, NULL, &vkDeviceMemory_depth_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkAllocateMemory() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkAllocateMemory() Successful!.\n");
    }

    // Step 10
    vkResult = vkBindImageMemory(vkDevice_sea, vkImage_depth_sea, vkDeviceMemory_depth_sea, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkBindDev() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkBindDev() Successful!.\n");
    }

    // Create Image View for Depth Image
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_depth_sea;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = vkImage_depth_sea;

    vkResult = vkCreateImageView(vkDevice_sea, &vkImageViewCreateInfo, NULL, &vkImageView_depth_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkCreateImageView() Failed for Depth Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkCreateImageView() Successful for Depth Image!.\n");
    }

    return (vkResult);
}

VkResult getSupportedDepthFormat(void) {
    //Variables
    VkResult vkResult = VK_SUCCESS;

    // Code 
    VkFormat vkFormat_depth_array[] = {
        VK_FORMAT_D32_SFLOAT_S8_UINT, // 32-bit signed float depth + 8-bit unsigned int stencil
        VK_FORMAT_D32_SFLOAT, // 32-bit signed float depth
        VK_FORMAT_D24_UNORM_S8_UINT, // 24-bit unsigned normalized depth + 8-bit unsigned int stencil
        VK_FORMAT_D16_UNORM_S8_UINT, // 16-bit unsigned normalized depth + 8-bit unsigned int stencil
        VK_FORMAT_D16_UNORM // 16-bit unsigned normalized depth
    };

    for(uint32_t i = 0; i < sizeof(vkFormat_depth_array) / sizeof(vkFormat_depth_array[0]); i++) {
        VkFormatProperties vkFormatProperties;
        memset((void*)&vkFormatProperties, 0, sizeof(VkFormatProperties));

        vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice_selected_sea, vkFormat_depth_array[i], &vkFormatProperties);

        if(vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            vkFormat_depth_sea = vkFormat_depth_array[i];
            fprintf(fptr, "getSupportedDepthFormat(): Supported Depth Format Found: %d\n", vkFormat_depth_sea);
            vkResult = VK_SUCCESS;
            break;
        }
    }

    return (vkResult);
}

VkResult createCommandPool(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
    memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(vkCommandPoolCreateInfo));

    vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.pNext = NULL;
    vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected_sea;

    vkResult = vkCreateCommandPool(vkDevice_sea, &vkCommandPoolCreateInfo, NULL, &vkCommandPool_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createCommandPool(): vkCreateCommandPool() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createCommandPool(): vkCreateCommandPool() Successful!.\n");
    }

    return (vkResult);
}

VkResult createCommandBuffers(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Step 1: Init and Allocate VkCommandBufferAllocateInfo
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
    memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool_sea;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    // Step 2: Allocate Command Buffer Array to the size of swapchainImageCount_sea
    vkCommandBuffer_array_sea = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapchainImageCount_sea);

    // Step 3: Allocat eeach command buffer in loop with allocateInfo struct
    for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
        vkResult = vkAllocateCommandBuffers(vkDevice_sea, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array_sea[i]);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "createCommandBuffers(): vkCreatvkAllocateCommandBufferseImageView() Failed at {%d}!.\n", i);
            return (vkResult);
        } else {
            fprintf(fptr, "createCommandBuffers(): vkAllocateCommandBuffers() Successful for {%d}!.\n", i);
        }
    }

    return (vkResult);
}


VkResult createVertexBuffer(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Step 1
    vertexData_array_sea.clear();

    int seg = segmentCount_sea > 0 ? segmentCount_sea : 1;
    float step = (2.0f * halfSize_sea) / (float)seg;

    // Generate non-indexed triangle list: each cell -> two triangles (6 vertices)
    for (int i = 0; i < seg; ++i) {
        float y0 = -halfSize_sea + i * step;
        float y1 = -halfSize_sea + (i + 1) * step;
        for (int j = 0; j < seg; ++j) {
            float x0 = -halfSize_sea + j * step;
            float x1 = -halfSize_sea + (j + 1) * step;

            // Triangle 1: (x0,y0), (x1,y0), (x1,y1)
            vertexData_array_sea.push_back(glm::vec3(x0, y0, 0.0f));
            vertexData_array_sea.push_back(glm::vec3(x1, y0, 0.0f));
            vertexData_array_sea.push_back(glm::vec3(x1, y1, 0.0f));

            // Triangle 2: (x0,y0), (x1,y1), (x0,y1)
            vertexData_array_sea.push_back(glm::vec3(x0, y0, 0.0f));
            vertexData_array_sea.push_back(glm::vec3(x1, y1, 0.0f));
            vertexData_array_sea.push_back(glm::vec3(x0, y1, 0.0f));
        }
    }

    // VertexData for Triangle Position
    // Step 2
    memset((void*)&vertexData_position_sea, 0, sizeof(VertexData));

    // Step 3
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // No flags, Valid Flags are used in scattered buffer
    vkBufferCreateInfo.size = vertexData_array_sea.size() * sizeof(glm::vec3);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    // Setp 4
    vkResult = vkCreateBuffer(vkDevice_sea, &vkBufferCreateInfo, NULL, &vertexData_position_sea.vkBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer(): vkCreateBuffer() Failed for Position!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer(): vkCreateBuffer() Successful for Position!.\n");
    }

    // Step 5
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice_sea, vertexData_position_sea.vkBuffer, &vkMemoryRequirements);

    // Step 6
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // this will be set in next step

    // Step A 
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties_sea.memoryTypeCount; i++) {
        // Step B
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1) {
            // Step C
            if(vkPhysicalDeviceMemoryProperties_sea.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                // Step D
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        // Step E
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //Setp 9
    vkResult = vkAllocateMemory(vkDevice_sea, &vkMemoryAllocateInfo, NULL, &vertexData_position_sea.vkDeviceMemory);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer(): vkAllocateMemory() Failed for Position!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer(): vkAllocateMemory() Successful for Position!.\n");
    }

    // Step 10
    vkResult = vkBindBufferMemory(vkDevice_sea, vertexData_position_sea.vkBuffer, vertexData_position_sea.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer(): vkBindBufferMemory() Failed for Position!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer(): vkBindBufferMemory() Successful for Position!.\n");
    }

    // Step 11
    void *data = NULL;

    vkResult = vkMapMemory(
        vkDevice_sea,
        vertexData_position_sea.vkDeviceMemory,
        0,
        vkMemoryAllocateInfo.allocationSize,
        0,
        &data
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer(): vkMapMemory() Failed for Position!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer(): vkMapMemory() Successful for Position!.\n");
    }

    // Step 12
    memcpy(data, vertexData_array_sea.data(), vertexData_array_sea.size() * sizeof(glm::vec3));

    // Step 13
    vkUnmapMemory(vkDevice_sea, vertexData_position_sea.vkDeviceMemory);

    return(vkResult);
}


VkResult createUniformBuffer (void) {
    // functions
    VkResult updateUniformBuffer(void);

    // variables
    VkResult vkResult = VK_SUCCESS;

    memset((void*)&uniformData_sea, 0, sizeof(UniformData));

    // Step 3
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // No flags, Valid Flags are used in scattered buffer
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    // Setp 4
    vkResult = vkCreateBuffer(vkDevice_sea, &vkBufferCreateInfo, NULL, &uniformData_sea.vkBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createUniformBuffer(): vkCreateBuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createUniformBuffer(): vkCreateBuffer() Successful!.\n");
    }

    // Step 5
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice_sea, uniformData_sea.vkBuffer, &vkMemoryRequirements);

    // Step 6
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // this will be set in next step

    // Step A 
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties_sea.memoryTypeCount; i++) {
        // Step B
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1) {
            // Step C
            if(vkPhysicalDeviceMemoryProperties_sea.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                // Step D
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        // Step E
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //Setp 9
    vkResult = vkAllocateMemory(vkDevice_sea, &vkMemoryAllocateInfo, NULL, &uniformData_sea.vkDeviceMemory);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createUniformBuffer(): vkAllocateMemory() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createUniformBuffer(): vkAllocateMemory() Successful!.\n");
    }

    // Step 10
    vkResult = vkBindBufferMemory(vkDevice_sea, uniformData_sea.vkBuffer, uniformData_sea.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createUniformBuffer(): vkBindBufferMemory() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createUniformBuffer(): vkBindBufferMemory() Successful!.\n");
    }

    // call updateUniformBuffer() to fill the uniform buffer with data
    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createUniformBuffer(): updateUniformBuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createUniformBuffer(): updateUniformBuffer() Successful!.\n");
    }

    return (vkResult);
}

VkResult updateUniformBuffer(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;
    const float angleOffsets[4] = {0.0f, 0.65f, -0.55f, 1.57f};
    const float wavelengthMultipliers[4] = {1.0f, 0.58f, 0.35f, 1.85f};
    const float amplitudeMultipliers[4] = {1.0f, 0.55f, 0.25f, 0.4f};
    const float speedMultipliers[4] = {1.0f, 1.24f, 1.52f, 0.74f};
    const float phaseOffsets[4] = {0.0f, 1.3f, 2.4f, 3.7f};

    struct MyUniformData myUniformData;
    memset((void*)&myUniformData, 0, sizeof(struct MyUniformData));

    myUniformData.modelMatrix = glm::mat4(1.0f);
    glm::mat4 translateMat = glm::mat4(1.0f);
    glm::mat4 rotateMat = glm::mat4(1.0f);
    glm::mat4 scaleMat = glm::mat4(1.0f);
    glm::vec3 cameraPosition = glm::vec3(0.0f + gCameraOffsetX, 2.35f + gCameraOffsetY, 8.5f + gCameraOffsetZ);
    glm::vec3 cameraTarget = glm::vec3(0.0f + gCameraOffsetX, 0.35f + gCameraOffsetY, -10.0f + gCameraOffsetZ);
    glm::vec2 baseWind = glm::vec2(gSeaUiState.windDirectionX, gSeaUiState.windDirectionY);

    if(glm::length(baseWind) < 0.001f) {
        baseWind = glm::vec2(1.0f, 0.0f);
    }
    baseWind = glm::normalize(baseWind);
    
    translateMat *= glm::translate(
        glm::mat4(1.0f),
        glm::vec3(0.0f, -0.55f, 0.0f)
    );

    rotateMat *= glm::rotate(
        glm::mat4(1.0f),
        glm::radians(-90.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    scaleMat *= glm::scale(
        glm::mat4(1.0f),
        glm::vec3(gSeaModelScale, gSeaModelScale, gSeaModelScale)
    );

    myUniformData.modelMatrix = translateMat * rotateMat * scaleMat;

    myUniformData.viewMatrix = glm::lookAt(
        cameraPosition,
        cameraTarget,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    myUniformData.projectionMatrix = glm::mat4(1.0f);

    glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);

    perspectiveProjectionMatrix = glm::perspective(
        glm::radians(50.0f),
        (float)winWidth_sea / (float)winHeight_sea,
        0.1f,
        180.0f
    );

    perspectiveProjectionMatrix[1][1] *= -1.0f; // Invert Y axis for Vulkan

    myUniformData.projectionMatrix = perspectiveProjectionMatrix;

    myUniformData.cameraPosition[0] = cameraPosition.x;
    myUniformData.cameraPosition[1] = cameraPosition.y;
    myUniformData.cameraPosition[2] = cameraPosition.z;
    myUniformData.cameraPosition[3] = (float) myClock_sea.getElapsedTime() * gSeaUiState.timeScale;

    for(int i = 0; i < 4; i++) {
        float cosAngle = cosf(angleOffsets[i]);
        float sinAngle = sinf(angleOffsets[i]);
        glm::vec2 waveDirection = glm::vec2(
            baseWind.x * cosAngle - baseWind.y * sinAngle,
            baseWind.x * sinAngle + baseWind.y * cosAngle
        );

        waveDirection = glm::normalize(waveDirection);

        myUniformData.waveDirections[i][0] = waveDirection.x;
        myUniformData.waveDirections[i][1] = waveDirection.y;
        myUniformData.waveDirections[i][2] = glm::clamp(gSeaUiState.choppiness * (1.0f - (float)i * 0.16f), 0.0f, 0.98f);
        myUniformData.waveDirections[i][3] = glm::max(0.6f, gSeaUiState.primaryWavelength * wavelengthMultipliers[i]);

        myUniformData.waveSettings[i][0] = gSeaUiState.primaryAmplitude * amplitudeMultipliers[i];
        myUniformData.waveSettings[i][1] = gSeaUiState.waveSpeed * speedMultipliers[i];
        myUniformData.waveSettings[i][2] = phaseOffsets[i];
        myUniformData.waveSettings[i][3] = 0.0f;
    }

    myUniformData.detailParams[0] = gSeaUiState.detailHeight;
    myUniformData.detailParams[1] = gSeaUiState.detailFrequency;
    myUniformData.detailParams[2] = gSeaUiState.detailSpeed;
    myUniformData.detailParams[3] = gSeaUiState.detailLayers;

    myUniformData.depthColor[0] = gSeaUiState.depthColor[0];
    myUniformData.depthColor[1] = gSeaUiState.depthColor[1];
    myUniformData.depthColor[2] = gSeaUiState.depthColor[2];
    myUniformData.depthColor[3] = 0.0f;

    myUniformData.surfaceColor[0] = gSeaUiState.surfaceColor[0];
    myUniformData.surfaceColor[1] = gSeaUiState.surfaceColor[1];
    myUniformData.surfaceColor[2] = gSeaUiState.surfaceColor[2];
    myUniformData.surfaceColor[3] = 0.0f;

    myUniformData.skyBottomColor[0] = gSeaUiState.skyBottomColor[0];
    myUniformData.skyBottomColor[1] = gSeaUiState.skyBottomColor[1];
    myUniformData.skyBottomColor[2] = gSeaUiState.skyBottomColor[2];
    myUniformData.skyBottomColor[3] = 0.0f;

    myUniformData.skyTopColor[0] = gSeaUiState.skyTopColor[0];
    myUniformData.skyTopColor[1] = gSeaUiState.skyTopColor[1];
    myUniformData.skyTopColor[2] = gSeaUiState.skyTopColor[2];
    myUniformData.skyTopColor[3] = 0.0f;

    // Must match the sun direction used by the sky / god-ray pass
    glm::vec3 sunDirection = glm::vec3(
        gSeaUiState.sunDirection[0],
        gSeaUiState.sunDirection[1],
        gSeaUiState.sunDirection[2]
    );
    if(glm::length(sunDirection) < 0.001f) {
        sunDirection = glm::vec3(-0.15f, 0.22f, -1.0f);
    }
    sunDirection = glm::normalize(sunDirection);

    myUniformData.sunDirection[0] = sunDirection.x;
    myUniformData.sunDirection[1] = sunDirection.y;
    myUniformData.sunDirection[2] = sunDirection.z;
    myUniformData.sunDirection[3] = 0.0f;

    myUniformData.sunColor[0] = gSeaUiState.sunColor[0];
    myUniformData.sunColor[1] = gSeaUiState.sunColor[1];
    myUniformData.sunColor[2] = gSeaUiState.sunColor[2];
    myUniformData.sunColor[3] = glm::clamp(gSeaUiState.sunGlow, 0.0f, 1.0f);

    myUniformData.sunParams[0] = gSeaUiState.glitterStrength;
    myUniformData.sunParams[1] = gSeaUiState.skyAmbient;
    myUniformData.sunParams[2] = gSeaUiState.sssStrength;
    myUniformData.sunParams[3] = 0.0f;

    myUniformData.skyParams[0] = gSeaUiState.horizonHaze;
    myUniformData.skyParams[1] = gSeaUiState.fogDensity;
    myUniformData.skyParams[2] = 0.0f;
    myUniformData.skyParams[3] = gSeaUiState.skyExposure;

    myUniformData.shadingParams[0] = gSeaUiState.colorOffset;
    myUniformData.shadingParams[1] = gSeaUiState.colorMultiplier;
    myUniformData.shadingParams[2] = gSeaUiState.fresnelPower;
    myUniformData.shadingParams[3] = gSeaUiState.reflectionStrength;

    myUniformData.lightingParams[0] = gSeaUiState.specularPower;
    myUniformData.lightingParams[1] = gSeaUiState.foamHeight;
    myUniformData.lightingParams[2] = gSeaUiState.foamIntensity;
    myUniformData.lightingParams[3] = gSeaUiState.sunIntensity;

    // Sphere bend (vertex shader); centre = modelMatrix * (0, 0, -sphereRadius / gSeaModelScale, 1)
    float sphereBlend = glm::clamp(gSeaUiState.sphereBlend, 0.0f, 1.0f);
    float smoothBlend = sphereBlend * sphereBlend * (3.0f - 2.0f * sphereBlend);
    float sphereRadiusPlane = glm::max(gSeaUiState.sphereRadius, 0.01f) / gSeaModelScale;
    myUniformData.sphereParams[0] = smoothBlend / sphereRadiusPlane;
    myUniformData.sphereParams[1] = 3.14159265f;
    myUniformData.sphereParams[2] = smoothBlend;
    myUniformData.sphereParams[3] = 0.0f;

    myUniformData.bronzeDarkColor[0] = gSeaUiState.bronzeDarkColor[0];
    myUniformData.bronzeDarkColor[1] = gSeaUiState.bronzeDarkColor[1];
    myUniformData.bronzeDarkColor[2] = gSeaUiState.bronzeDarkColor[2];
    myUniformData.bronzeDarkColor[3] = gSeaUiState.bronzeRoughness;

    myUniformData.bronzeBrightColor[0] = gSeaUiState.bronzeBrightColor[0];
    myUniformData.bronzeBrightColor[1] = gSeaUiState.bronzeBrightColor[1];
    myUniformData.bronzeBrightColor[2] = gSeaUiState.bronzeBrightColor[2];
    myUniformData.bronzeBrightColor[3] = glm::clamp(gSeaUiState.bronzeBlend, 0.0f, 1.0f);

    void *data = NULL;

    vkResult = vkMapMemory(
        vkDevice_sea,
        uniformData_sea.vkDeviceMemory,
        0,
        sizeof(struct MyUniformData),
        0,
        &data
    );
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "updateUniformBuffer(): vkMapMemory() Failed!.\n");
        return (vkResult);
    }

    memcpy(data, &myUniformData, sizeof(struct MyUniformData));

    vkUnmapMemory(vkDevice_sea, uniformData_sea.vkDeviceMemory);

    // Free Data / Set it to NULL
    data = NULL;

    return (vkResult);
}

VkResult createShaders(void) {

    // variables
    VkResult vkResult = VK_SUCCESS;

    // for vertex shader
    const char* szFileName = "shader.vert.spv";
    FILE *fp = NULL;
    size_t fileSize = 0;

    fp = fopen(szFileName, "rb");
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createShaders(): fopen() failed to open Vertex Shader spir-v file!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders(): fopen() succeed to open Vertex Shader spir-v file!.\n");
    }

    fseek(fp, 0l, SEEK_END);
    fileSize = ftell(fp);
    if(fileSize == 0) {
        fprintf(fptr, "createShaders(): ftell() gave file size 0.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } 
    fseek(fp, 0l, SEEK_SET);

    char *shaderData = (char*)malloc(fileSize * sizeof(char));

    size_t retVal = fread(shaderData, fileSize, 1, fp);
    if(retVal != 1) {
        fprintf(fptr, "createShaders(): fread() failed to read Vertex Shader file!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders(): fread() succeed to read Vertex Shader file!.\n");
    }
    fclose(fp);
    fp = NULL;

    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0;
    vkShaderModuleCreateInfo.codeSize = fileSize;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(vkDevice_sea, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createShaders(): vkCreateShaderModule() for Vertex Shader Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders(): vkCreateShaderModule() for Vertex Shader Successful!.\n");
    }

    if(shaderData) {
        free(shaderData);
        shaderData = NULL;
    }
    fprintf(fptr, "createShaders(): Vertex Shader Module Created Successful!.\n");

    // for fragment shader
    szFileName = "shader.frag.spv";
    fileSize = 0;

    fp = fopen(szFileName, "rb");
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createShaders(): fopen() failed to open Fragment Shader spir-v file!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders(): fopen() succeed to open Fragment Shader spir-v file!.\n");
    }

    fseek(fp, 0l, SEEK_END);
    fileSize = ftell(fp);
    if(fileSize == 0) {
        fprintf(fptr, "createShaders(): ftell() gave file size: 0.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    fseek(fp, 0l, SEEK_SET);

    shaderData = (char*)malloc(fileSize * sizeof(char));

    retVal = fread(shaderData, fileSize, 1, fp);
    if(retVal != 1) {
        fprintf(fptr, "createShaders(): fread() failed to read Fragment Shader file!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders(): fread() succeed to read Fragment Shader file!.\n");
    }
    fclose(fp);
    fp = NULL;

    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0;
    vkShaderModuleCreateInfo.codeSize = fileSize;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(vkDevice_sea, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createShaders(): vkCreateShaderModule() for Fragment Shader Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders(): vkCreateShaderModule() for Fragment Shader Successful!.\n");
    }

    if(shaderData) {
        free(shaderData);
        shaderData = NULL;
    }
    fprintf(fptr, "createShaders(): Fragment Shader Module Created Successful!.\n");

    return (vkResult);
}


// Load texture via stb_image -> staging buffer -> VkImage + view + sampler
VkResult createTexture(const char *textureFileName) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Step 1: Load Texture Image Information
    FILE *fp = NULL;
    fp = fopen(textureFileName, "rb");
    if(fp == NULL) {
        fprintf(fptr, "createTexture(): Failed to open texture file: %s\n", textureFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    uint8_t *image_data = NULL;
    int texture_width, texture_height, texture_channels;

    image_data = stbi_load_from_file(fp, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    if(image_data == NULL || texture_width <= 0 || texture_height <= 0 || texture_channels <= 0) {
        fprintf(fptr, "createTexture(): Failed to load texture image data from stbi_load_from_file for file: %s\n", textureFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fclose(fp);
        return (vkResult);
    }

    VkDeviceSize image_size = texture_width * texture_height * 4; // 4 channels (RGBA)

    fprintf(fptr, "createTexture(): Texture Image Loaded Successfully! Width: %d, Height: %d, Channels: %d, Size: %llu bytes\n",
        texture_width, texture_height, texture_channels, (unsigned long long)image_size);

    // Step 2: Create Staging Buffer
    VkBuffer vkBuffer_stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;

    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = image_size;
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkResult = vkCreateBuffer(vkDevice_sea, &vkBufferCreateInfo_stagingBuffer, NULL, &vkBuffer_stagingBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkCreateBuffer() Failed for Staging Buffer!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkCreateBuffer() Successful for Staging Buffer!.\n");
    }

    VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
    memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice_sea, vkBuffer_stagingBuffer, &vkMemoryRequirements_stagingBuffer);

    VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
    memset((void*)&vkMemoryAllocateInfo_stagingBuffer, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_stagingBuffer.pNext = NULL;
    vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
    vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0;

    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties_sea.memoryTypeCount; i++) {
        if((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1) {
            if(vkPhysicalDeviceMemoryProperties_sea.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice_sea, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vkDeviceMemory_stagingBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkAllocateMemory() Failed for Staging Buffer!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkAllocateMemory() Successful for Staging Buffer!.\n");
    }

    vkResult = vkBindBufferMemory(vkDevice_sea, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkBindBufferMemory() Failed for Staging Buffer!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkBindBufferMemory() Successful for Staging Buffer!.\n");
    }

    void *data = NULL;

    vkResult = vkMapMemory(vkDevice_sea, vkDeviceMemory_stagingBuffer, 0, image_size, 0, &data);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkMapMemory() Failed for Staging Buffer!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkMapMemory() Successful for Staging Buffer!.\n");
    }

    memcpy(data, image_data, image_size);

    vkUnmapMemory(vkDevice_sea, vkDeviceMemory_stagingBuffer);

    stbi_image_free(image_data);
    image_data = NULL;
    fprintf(fptr, "createTexture(): Image Data Copied to Staging Buffer Successful & Freed stbi Image Data!.\n");
    fclose(fp);

    // Step 3: Create VkImage for Texture
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));

    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkImageCreateInfo.extent.width = texture_width;
    vkImageCreateInfo.extent.height = texture_height;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(vkDevice_sea, &vkImageCreateInfo, NULL, &vkImage_oceanMask);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkCreateImage() Failed for Texture Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkCreateImage() Successful for Texture Image!.\n");
    }

    VkMemoryRequirements vkMemoryRequirements_image;
    memset((void*)&vkMemoryRequirements_image, 0, sizeof(VkMemoryRequirements));

    vkGetImageMemoryRequirements(vkDevice_sea, vkImage_oceanMask, &vkMemoryRequirements_image);

    VkMemoryAllocateInfo vkMemoryAllocateInfo_image;
    memset((void*)&vkMemoryAllocateInfo_image, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo_image.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_image.pNext = NULL;
    vkMemoryAllocateInfo_image.allocationSize = vkMemoryRequirements_image.size;
    vkMemoryAllocateInfo_image.memoryTypeIndex = 0;

    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties_sea.memoryTypeCount; i++) {
        if((vkMemoryRequirements_image.memoryTypeBits & 1) == 1) {
            if(vkPhysicalDeviceMemoryProperties_sea.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                vkMemoryAllocateInfo_image.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_image.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice_sea, &vkMemoryAllocateInfo_image, NULL, &vkDeviceMemory_oceanMask);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkAllocateMemory() Failed for Texture Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkAllocateMemory() Successful for Texture Image!.\n");
    }

    vkResult = vkBindImageMemory(vkDevice_sea, vkImage_oceanMask, vkDeviceMemory_oceanMask, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkBindImageMemory() Failed for Texture Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkBindImageMemory() Successful for Texture Image!.\n");
    }

    // Step 4: Transition to TRANSFER_DST, copy staging buffer to image, transition to SHADER_READ_ONLY
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
    memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool_sea;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer vkCommandBuffer_texture = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(vkDevice_sea, &vkCommandBufferAllocateInfo, &vkCommandBuffer_texture);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkAllocateCommandBuffers() Failed!.\n");
        return (vkResult);
    }

    VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
    memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

    vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo.pNext = NULL;
    vkCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_texture, &vkCommandBufferBeginInfo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkBeginCommandBuffer() Failed!.\n");
        return (vkResult);
    }

    VkImageMemoryBarrier vkImageMemoryBarrier_toTransferDst;
    memset((void*)&vkImageMemoryBarrier_toTransferDst, 0, sizeof(VkImageMemoryBarrier));

    vkImageMemoryBarrier_toTransferDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier_toTransferDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkImageMemoryBarrier_toTransferDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier_toTransferDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier_toTransferDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier_toTransferDst.image = vkImage_oceanMask;
    vkImageMemoryBarrier_toTransferDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier_toTransferDst.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier_toTransferDst.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier_toTransferDst.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier_toTransferDst.subresourceRange.levelCount = 1;
    vkImageMemoryBarrier_toTransferDst.srcAccessMask = 0;
    vkImageMemoryBarrier_toTransferDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        vkCommandBuffer_texture,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier_toTransferDst
    );

    VkBufferImageCopy vkBufferImageCopy;
    memset((void*)&vkBufferImageCopy, 0, sizeof(VkBufferImageCopy));
    vkBufferImageCopy.bufferOffset = 0;
    vkBufferImageCopy.bufferRowLength = 0;
    vkBufferImageCopy.bufferImageHeight = 0;
    vkBufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkBufferImageCopy.imageSubresource.mipLevel = 0;
    vkBufferImageCopy.imageSubresource.baseArrayLayer = 0;
    vkBufferImageCopy.imageSubresource.layerCount = 1;
    vkBufferImageCopy.imageExtent.width = texture_width;
    vkBufferImageCopy.imageExtent.height = texture_height;
    vkBufferImageCopy.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        vkCommandBuffer_texture,
        vkBuffer_stagingBuffer,
        vkImage_oceanMask,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &vkBufferImageCopy
    );

    VkImageMemoryBarrier vkImageMemoryBarrier_toShaderRead;
    memset((void*)&vkImageMemoryBarrier_toShaderRead, 0, sizeof(VkImageMemoryBarrier));

    vkImageMemoryBarrier_toShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier_toShaderRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier_toShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkImageMemoryBarrier_toShaderRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier_toShaderRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier_toShaderRead.image = vkImage_oceanMask;
    vkImageMemoryBarrier_toShaderRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier_toShaderRead.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier_toShaderRead.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier_toShaderRead.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier_toShaderRead.subresourceRange.levelCount = 1;
    vkImageMemoryBarrier_toShaderRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkImageMemoryBarrier_toShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        vkCommandBuffer_texture,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier_toShaderRead
    );

    vkResult = vkEndCommandBuffer(vkCommandBuffer_texture);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkEndCommandBuffer() Failed!.\n");
        return (vkResult);
    }

    VkSubmitInfo vkSubmitInfo_texture;
    memset((void*)&vkSubmitInfo_texture, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_texture.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_texture.pNext = NULL;
    vkSubmitInfo_texture.commandBufferCount = 1;
    vkSubmitInfo_texture.pCommandBuffers = &vkCommandBuffer_texture;

    vkResult = vkQueueSubmit(vkQueue_sea, 1, &vkSubmitInfo_texture, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkQueueSubmit() Failed!.\n");
        return (vkResult);
    }

    vkResult = vkQueueWaitIdle(vkQueue_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkQueueWaitIdle() Failed!.\n");
        return (vkResult);
    }

    if(vkCommandBuffer_texture) {
        vkFreeCommandBuffers(vkDevice_sea, vkCommandPool_sea, 1, &vkCommandBuffer_texture);
        vkCommandBuffer_texture = VK_NULL_HANDLE;
    }

    // Step 5: Remove staging buffer
    if(vkBuffer_stagingBuffer) {
        vkFreeMemory(vkDevice_sea, vkDeviceMemory_stagingBuffer, NULL);
        vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
    }
    if(vkBuffer_stagingBuffer) {
        vkDestroyBuffer(vkDevice_sea, vkBuffer_stagingBuffer, NULL);
        vkBuffer_stagingBuffer = VK_NULL_HANDLE;
    }

    // Step 6: Create Image View for Texture
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = vkImage_oceanMask;

    vkResult = vkCreateImageView(vkDevice_sea, &vkImageViewCreateInfo, NULL, &vkImageView_oceanMask);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkCreateImageView() Failed for Texture Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkCreateImageView() Successful for Texture Image!.\n");
    }

    // Step 7: Create Sampler for Texture
    VkSamplerCreateInfo vkSamplerCreateInfo;
    memset((void*)&vkSamplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));

    vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCreateInfo.pNext = NULL;
    vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // no wrap at poles
    vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.anisotropyEnable = VK_FALSE;
    vkSamplerCreateInfo.maxAnisotropy = 16.0f;
    vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    vkSamplerCreateInfo.compareEnable = VK_FALSE;
    vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    vkResult = vkCreateSampler(vkDevice_sea, &vkSamplerCreateInfo, NULL, &vkSampler_oceanMask);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkCreateSampler() Failed for Texture Sampler!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkCreateSampler() Successful for Texture Sampler!.\n");
    }

    return (vkResult);
}


VkResult createDescriptorSetLayout(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Initialize Descriptor Set Bindings: 0 -> uniform buffer, 1 -> ocean mask sampler
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_array[2];
    memset((void*)vkDescriptorSetLayoutBinding_array, 0, sizeof(VkDescriptorSetLayoutBinding) * _ARRAYSIZE(vkDescriptorSetLayoutBinding_array));

    vkDescriptorSetLayoutBinding_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorSetLayoutBinding_array[0].binding = 0; // this 0 is  the binding index, we will use this index in shader
    vkDescriptorSetLayoutBinding_array[0].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // this binding will be used in vertex and fragment shaders
    vkDescriptorSetLayoutBinding_array[0].pImmutableSamplers = NULL; // we don't have any immutable samplers for now

    vkDescriptorSetLayoutBinding_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorSetLayoutBinding_array[1].binding = 1;
    vkDescriptorSetLayoutBinding_array[1].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkDescriptorSetLayoutBinding_array[1].pImmutableSamplers = NULL;

    //Create Descriptor Set Layout Create Info
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0;
    vkDescriptorSetLayoutCreateInfo.bindingCount = _ARRAYSIZE(vkDescriptorSetLayoutBinding_array);
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_array;
    
    // Create Descriptor Set Layout
    vkResult = vkCreateDescriptorSetLayout(vkDevice_sea, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createDescriptorSetLayout(): vkCreateDescriptorSetLayout() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createDescriptorSetLayout(): vkCreateDescriptorSetLayout() Successful!.\n");
    }

    return (vkResult);
}

VkResult createPipelineLayout(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Create Pipeline Layout Create Info
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    vkPipelineLayoutCreateInfo.setLayoutCount = 1; // we have only one descriptor set layout
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout_sea;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0; // no push constant range for now
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

    // Create Pipeline Layout
    vkResult = vkCreatePipelineLayout(vkDevice_sea, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createPipelineLayout(): vkCreatePipelineLayout() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createPipelineLayout(): vkCreatePipelineLayout() Successful!.\n");
    }

    return (vkResult);
}

VkResult createDescriptorPool(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Create Descriptor Pool Create Info
    VkDescriptorPoolSize vkDescriptorPoolSize_array[2];
    memset((void*)vkDescriptorPoolSize_array, 0, sizeof(VkDescriptorPoolSize) * _ARRAYSIZE(vkDescriptorPoolSize_array));

    vkDescriptorPoolSize_array[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize_array[0].descriptorCount = 1; // we have only one uniform buffer

    vkDescriptorPoolSize_array[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorPoolSize_array[1].descriptorCount = 1; // ocean mask

    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
    memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));

    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.maxSets = 1; // we have only one descriptor set
    vkDescriptorPoolCreateInfo.poolSizeCount = _ARRAYSIZE(vkDescriptorPoolSize_array);
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSize_array;

    // Create Descriptor Pool
    vkResult = vkCreateDescriptorPool(vkDevice_sea, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createDescriptorPool(): vkCreateDescriptorPool() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createDescriptorPool(): vkCreateDescriptorPool() Successful!.\n");
    }

    return (vkResult);
}

VkResult createDescriptorSet(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Create Descriptor Set Allocate Info
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool_sea;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1; // we have only one descriptor set
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout_sea; // we have only one descriptor set layout

    // Allocate Descriptor Set
    vkResult = vkAllocateDescriptorSets(vkDevice_sea, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createDescriptorSet(): vkAllocateDescriptorSets() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createDescriptorSet(): vkAllocateDescriptorSets() Successful!.\n");
    }

    // Describe whether we want buffer or image as uniform
    // we want buffer as uniform
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));

    vkDescriptorBufferInfo.buffer = uniformData_sea.vkBuffer; // this is the buffer we want to use as uniform
    vkDescriptorBufferInfo.offset = 0; // offset is 0
    vkDescriptorBufferInfo.range = sizeof(struct MyUniformData); // range is size of uniform

    // Ocean mask image as combined image sampler
    VkDescriptorImageInfo vkDescriptorImageInfo;
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));

    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_oceanMask;
    vkDescriptorImageInfo.sampler = vkSampler_oceanMask;

    // Now update the descriptor set with the buffer & image directly to the shader
    // we will write to the shader
    VkWriteDescriptorSet vkWriteDescriptorSet_array[2];
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].pNext = NULL;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_sea; // this is the descriptor set we want to update
    vkWriteDescriptorSet_array[0].dstArrayElement = 0; // we have only one descriptor set, so array element is 0
    vkWriteDescriptorSet_array[0].descriptorCount = 1; // we are only gonna write one descriptor set
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL; // using for tiling of texture but we're not using it now
    vkWriteDescriptorSet_array[0].dstBinding = 0; // this is the binding index we used in descriptor set layout & shader

    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].pNext = NULL;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_sea;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; // ocean mask binding

    // Update Descriptor Set
    vkUpdateDescriptorSets(vkDevice_sea, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);
    // last two parameters are for copy descriptor sets, which are used while copying

    fprintf(fptr, "createDescriptorSet(): vkUpdateDescriptorSets() Successful!.\n");

    return (vkResult);
}

VkResult initializeImGui(void) {
    VkResult vkResult = VK_SUCCESS;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    if(!ImGui_ImplWin32_Init((void*)ghwnd)) {
        fprintf(fptr, "initializeImGui(): ImGui_ImplWin32_Init() Failed!.\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 256 },
    };

    VkDescriptorPoolCreateInfo pool_info;
    memset((void*)&pool_info, 0, sizeof(VkDescriptorPoolCreateInfo));
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 256;
    pool_info.poolSizeCount = _ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    vkResult = vkCreateDescriptorPool(vkDevice_sea, &pool_info, NULL, &vkDescriptorPool_imgui_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initializeImGui(): vkCreateDescriptorPool() Failed!.\n");
        return vkResult;
    }

    ImGui_ImplVulkan_InitInfo init_info;
    memset((void*)&init_info, 0, sizeof(ImGui_ImplVulkan_InitInfo));
    init_info.ApiVersion = VK_API_VERSION_1_3;
    init_info.Instance = vkInstance_sea;
    init_info.PhysicalDevice = vkPhysicalDevice_selected_sea;
    init_info.Device = vkDevice_sea;
    init_info.QueueFamily = graphicsQueueFamilyIndex_selected_sea;
    init_info.Queue = vkQueue_sea;
    init_info.DescriptorPool = vkDescriptorPool_imgui_sea;
    init_info.MinImageCount = (swapchainImageCount_sea < 2) ? 2 : swapchainImageCount_sea;
    init_info.ImageCount = swapchainImageCount_sea;
    init_info.PipelineInfoMain.RenderPass = vkRenderPass_sea;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.UseDynamicRendering = false;
    init_info.Allocator = NULL;

    if(!ImGui_ImplVulkan_Init(&init_info)) {
        fprintf(fptr, "initializeImGui(): ImGui_ImplVulkan_Init() Failed!.\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return vkResult;
}

bool buildImGuiUI(void) {
    bool changed = false;

    if(gShowSeaControls) {
        ImGui::Begin("Raging Sea Controls", &gShowSeaControls);
        ImGui::Text("Wave Controls");
        changed |= ImGui::SliderFloat("Time Scale", &gSeaUiState.timeScale, 0.05f, 2.0f);
        changed |= ImGui::SliderFloat2("Wind Direction", &gSeaUiState.windDirectionX, -1.0f, 1.0f);
        changed |= ImGui::SliderFloat("Primary Wavelength", &gSeaUiState.primaryWavelength, 1.0f, 12.0f);
        changed |= ImGui::SliderFloat("Primary Amplitude", &gSeaUiState.primaryAmplitude, 0.02f, 0.6f);
        changed |= ImGui::SliderFloat("Wave Speed", &gSeaUiState.waveSpeed, 0.1f, 2.5f);
        changed |= ImGui::SliderFloat("Choppiness", &gSeaUiState.choppiness, 0.05f, 0.95f);
        changed |= ImGui::SliderFloat("Detail Height", &gSeaUiState.detailHeight, 0.0f, 0.2f);
        changed |= ImGui::SliderFloat("Detail Frequency", &gSeaUiState.detailFrequency, 0.5f, 8.0f);
        changed |= ImGui::SliderFloat("Detail Speed", &gSeaUiState.detailSpeed, 0.1f, 5.0f);
        changed |= ImGui::SliderFloat("Detail Layers", &gSeaUiState.detailLayers, 0.0f, 4.0f);

        ImGui::Separator();
        ImGui::Text("Color Controls");
        changed |= ImGui::ColorEdit3("Depth Color", gSeaUiState.depthColor);
        changed |= ImGui::ColorEdit3("Surface Color", gSeaUiState.surfaceColor);
        changed |= ImGui::ColorEdit3("Sky Horizon", gSeaUiState.skyBottomColor);
        changed |= ImGui::ColorEdit3("Sky Zenith", gSeaUiState.skyTopColor);
        changed |= ImGui::ColorEdit3("Sun Color", gSeaUiState.sunColor);
        changed |= ImGui::SliderFloat3("Sun Direction", gSeaUiState.sunDirection, -1.0f, 1.0f);
        changed |= ImGui::SliderFloat("Sun Intensity", &gSeaUiState.sunIntensity, 0.5f, 20.0f);
        changed |= ImGui::SliderFloat("Sun Glow", &gSeaUiState.sunGlow, 0.0f, 1.0f);
        changed |= ImGui::SliderFloat("Sun Glitter", &gSeaUiState.glitterStrength, 0.0f, 2.0f);
        changed |= ImGui::SliderFloat("Sky Ambient", &gSeaUiState.skyAmbient, 0.0f, 2.0f);
        changed |= ImGui::SliderFloat("Subsurface", &gSeaUiState.sssStrength, 0.0f, 0.5f);
        changed |= ImGui::SliderFloat("Horizon Haze", &gSeaUiState.horizonHaze, 0.0f, 1.0f);
        changed |= ImGui::SliderFloat("Fog Density", &gSeaUiState.fogDensity, 0.0f, 3.0f);
        changed |= ImGui::SliderFloat("Sky Exposure", &gSeaUiState.skyExposure, 0.35f, 1.6f);
        changed |= ImGui::SliderFloat("Color Offset", &gSeaUiState.colorOffset, -0.2f, 0.6f);
        changed |= ImGui::SliderFloat("Color Multiplier", &gSeaUiState.colorMultiplier, 0.1f, 3.0f);
        changed |= ImGui::SliderFloat("Fresnel Power", &gSeaUiState.fresnelPower, 1.0f, 8.0f);
        changed |= ImGui::SliderFloat("Reflection Strength", &gSeaUiState.reflectionStrength, 0.0f, 1.0f);
        changed |= ImGui::SliderFloat("Specular Power", &gSeaUiState.specularPower, 8.0f, 256.0f);
        changed |= ImGui::SliderFloat("Foam Height", &gSeaUiState.foamHeight, 0.0f, 0.5f);
        changed |= ImGui::SliderFloat("Foam Intensity", &gSeaUiState.foamIntensity, 0.0f, 1.0f);

        ImGui::Separator();
        ImGui::Text("Sphere Controls");
        bool wrapToSphere = gSphereBlendTarget > 0.5f;
        if(ImGui::Checkbox("Wrap To Sphere (B)", &wrapToSphere)) {
            gSphereBlendTarget = wrapToSphere ? 1.0f : 0.0f;
            changed = true;
        }
        if(ImGui::SliderFloat("Sphere Blend", &gSeaUiState.sphereBlend, 0.0f, 1.0f)) {
            gSphereBlendTarget = gSeaUiState.sphereBlend; // hold the dragged value
            changed = true;
        }
        changed |= ImGui::SliderFloat("Sphere Radius", &gSeaUiState.sphereRadius, 1.0f, 40.0f);
        // Sphere closes only while radius <= halfSize_sea * scale / PI
        float closedSphereRadius = halfSize_sea * gSeaModelScale / 3.14159265f;
        if(gSeaUiState.sphereRadius > closedSphereRadius + 0.01f) {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "Open at south pole (closes at radius <= %.2f)", closedSphereRadius);
        }

        ImGui::Separator();
        ImGui::Text("Bronze Controls");
        bool turnToBronze = gBronzeBlendTarget > 0.5f;
        if(ImGui::Checkbox("Turn To Bronze (C)", &turnToBronze)) {
            gBronzeBlendTarget = turnToBronze ? 1.0f : 0.0f;
            changed = true;
        }
        if(ImGui::SliderFloat("Bronze Blend", &gSeaUiState.bronzeBlend, 0.0f, 1.0f)) {
            gBronzeBlendTarget = gSeaUiState.bronzeBlend; // hold the dragged value
            changed = true;
        }
        changed |= ImGui::ColorEdit3("Bronze Dark", gSeaUiState.bronzeDarkColor);
        changed |= ImGui::ColorEdit3("Bronze Bright", gSeaUiState.bronzeBrightColor);
        changed |= ImGui::SliderFloat("Bronze Roughness", &gSeaUiState.bronzeRoughness, 0.05f, 1.0f);
        if(ImGui::Button("Reset Bronze")) {
            // measured from the Atlas model
            gSeaUiState.bronzeDarkColor[0] = 0.30f; gSeaUiState.bronzeDarkColor[1] = 0.23f; gSeaUiState.bronzeDarkColor[2] = 0.15f;
            gSeaUiState.bronzeBrightColor[0] = 0.60f; gSeaUiState.bronzeBrightColor[1] = 0.49f; gSeaUiState.bronzeBrightColor[2] = 0.37f;
            gSeaUiState.bronzeRoughness = 0.53f;
            changed = true;
        }

        ImGui::Separator();
        changed |= ImGui::Checkbox("Show ImGui Demo", &gShowImGuiDemoWindow);
        ImGui::Separator();
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Text("Command buffers are reused until UI/input invalidates them.");
        ImGui::End();
    }

    // Placeholder daytime sky for standalone preview; the sky/god-ray pass replaces this in the demo
    vkClearColorValue_sea.float32[0] = glm::mix(gSeaUiState.skyBottomColor[0], gSeaUiState.skyTopColor[0], 0.35f);
    vkClearColorValue_sea.float32[1] = glm::mix(gSeaUiState.skyBottomColor[1], gSeaUiState.skyTopColor[1], 0.35f);
    vkClearColorValue_sea.float32[2] = glm::mix(gSeaUiState.skyBottomColor[2], gSeaUiState.skyTopColor[2], 0.35f);
    vkClearColorValue_sea.float32[3] = 1.0f;

    if(gShowImGuiDemoWindow) {
        ImGui::ShowDemoWindow(&gShowImGuiDemoWindow);
    }

    return changed;
}

void uninitializeImGui(void) {
    if(ImGui::GetCurrentContext() != NULL) {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        fprintf(fptr, "uninitializeImGui(): ImGui backend shutdown successful!.\n");
    }

    if(vkDescriptorPool_imgui_sea) {
        vkDestroyDescriptorPool(vkDevice_sea, vkDescriptorPool_imgui_sea, NULL);
        vkDescriptorPool_imgui_sea = VK_NULL_HANDLE;
        fprintf(fptr, "uninitializeImGui(): vkDestroyDescriptorPool() for ImGui successful!.\n");
    }
}

VkResult createRenderPass(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Code
    //Step 1: Create Attachment Description stcture array
    VkAttachmentDescription vkAttachmentDescription_array[2];
    memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));

    vkAttachmentDescription_array[0].flags = 0;
    vkAttachmentDescription_array[0].format =  vkFormat_color_sea;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // For Depth Attachment
    vkAttachmentDescription_array[1].flags = 0;
    vkAttachmentDescription_array[1].format =  vkFormat_depth_sea;
    vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Step 2: Create Attachment Reference Structure
    VkAttachmentReference vkAttachmentReference_color;
    memset((void*)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));

    vkAttachmentReference_color.attachment = 0; // From the array of attachment description, refer to 0th index, oth will be color attachment
    vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 

    // Create Attachment Reference for Depth Attachment
    VkAttachmentReference vkAttachmentReference_depth;
    memset((void*)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));

    vkAttachmentReference_depth.attachment = 1; // From the array of attachment description, refer to 1st index, 1st will be depth attachment
    vkAttachmentReference_depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Depth Attachment Layout

    // STep 3: create sub pass description strcture
    VkSubpassDescription vkSubpassDescription;
    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
    
    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = 1; // this count should be count of vkAttachmentReference array
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference_color;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = &vkAttachmentReference_depth; // this is the depth attachment reference;
    vkSubpassDescription.preserveAttachmentCount = 0;
    vkSubpassDescription.pPreserveAttachments = NULL;

    // Step 4: Render Pass Create Info
    VkRenderPassCreateInfo vkRenderPassCreateInfo;
    memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));

    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;
    

    // Step 5: Create Render Pass
    vkResult = vkCreateRenderPass(
        vkDevice_sea,
        &vkRenderPassCreateInfo,
        NULL,
        &vkRenderPass_sea
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createRenderPass(): vkCreateRenderPass() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createRenderPass(): vkCreateRenderPass() Successful!.\n");
    }

    return (vkResult);
}

VkResult createPipeline(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Vertex Input Binding Description [ Vertex Input State]
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    vkVertexInputBindingDescription_array[0].binding = AMK_ATTRIBUTE_POSITION; // 0th binding index for position
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    // Position Attribute
    vkVertexInputAttributeDescription_array[0].binding = AMK_ATTRIBUTE_POSITION;
    vkVertexInputAttributeDescription_array[0].location = AMK_ATTRIBUTE_POSITION;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;
    
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    // Input Assembly State
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));

    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Rasterization State
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // Wireframe modes
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE; // No culling
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

    // Color Blending State
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));

    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;


    // Viewport Scissor State
    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));

    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;

    // Set the viewport/s
    vkPipelineViewportStateCreateInfo.viewportCount = 1;

    memset((void*)&vkViewport_sea, 0, sizeof(VkViewport));
    vkViewport_sea.x = 0;
    vkViewport_sea.y = 0;
    vkViewport_sea.width = (float)vkExtent2D_swapchain_sea.width;
    vkViewport_sea.height = (float)vkExtent2D_swapchain_sea.height;
    vkViewport_sea.minDepth = 0.0f;
    vkViewport_sea.maxDepth = 1.0f;

    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport_sea;

    // Set the scissor rect/s
    vkPipelineViewportStateCreateInfo.scissorCount = 1;
    
    memset((void*)&vkRect2D_scissor_sea, 0, sizeof(VkRect2D));
    vkRect2D_scissor_sea.offset.x = 0;
    vkRect2D_scissor_sea.offset.y = 0;
    vkRect2D_scissor_sea.extent.width = vkExtent2D_swapchain_sea.width;
    vkRect2D_scissor_sea.extent.height = vkExtent2D_swapchain_sea.height;

    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor_sea;

    // Depth Stencil State
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));

    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back; // front and back are same

    // Dynamic State
    // We don't have any dynamic state;

    // Multisample State
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));

    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


    // Shader Stage State
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
    memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

    // Vertex Shader Stage
    vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[0].flags = 0;
    vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_sea;
    vkPipelineShaderStageCreateInfo_array[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;

    // Fragment Shader Stage
    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[1].flags = 0;
    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_sea;
    vkPipelineShaderStageCreateInfo_array[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;


    // Tessellation State
    // We don't have tessellation shaders so we can skip this state


    // Pipelines are created in a pipeline cache, we will create Pipeline cache object
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));

    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;

    VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;

    vkResult = vkCreatePipelineCache(vkDevice_sea, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createPipeline(): vkCreatePipelineCache() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createPipeline(): vkCreatePipelineCache() Successful!.\n");
    }

    // Create Graphics Pipeline
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));

    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;   
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // we don't have dynamic state
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
    vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout_sea;
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass_sea;
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline handle
    vkGraphicsPipelineCreateInfo.basePipelineIndex = 0; // no base pipeline index

    // Create Graphics Pipeline
    vkResult = vkCreateGraphicsPipelines(vkDevice_sea, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createPipeline(): vkCreateGraphicsPipelines() Failed!.\n");
        // Destroy Pipeline Cache
        vkDestroyPipelineCache(vkDevice_sea, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        return (vkResult);
    } else {
        fprintf(fptr, "createPipeline(): vkCreateGraphicsPipelines() Successful!.\n");
    }

    // Destroy Pipeline Cache
    vkDestroyPipelineCache(vkDevice_sea, vkPipelineCache, NULL);
    vkPipelineCache = VK_NULL_HANDLE;

    return (vkResult);
}

VkResult createFramebuffers(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // allocate frame buffers array and creat efream buffers in loop with counts of allocated swapchain images
    vkFramebuffer_array_sea = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchainImageCount_sea);

    for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {

        // Step 1: create VkImageView array for color and depth attachments
        VkImageView vkImageView_attachments_array[2];
        memset((void*)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

        // Step 2: Create VkFrameBufferCreateInfo structure
        VkFramebufferCreateInfo vkFrameBufferCreateInfo;
        memset((void*)&vkFrameBufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

        vkFrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFrameBufferCreateInfo.flags = 0;
        vkFrameBufferCreateInfo.pNext = NULL;
        vkFrameBufferCreateInfo.renderPass = vkRenderPass_sea;
        vkFrameBufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
        vkFrameBufferCreateInfo.pAttachments = vkImageView_attachments_array;
        vkFrameBufferCreateInfo.width = vkExtent2D_swapchain_sea.width;
        vkFrameBufferCreateInfo.height = vkExtent2D_swapchain_sea.height;
        vkFrameBufferCreateInfo.layers = 1; // VALIDATION USE CASE 2: Comment this line to see the error

        vkImageView_attachments_array[0] = swapchainImageView_array_sea[i];
        vkImageView_attachments_array[1] = vkImageView_depth_sea; // this is the depth attachment image view

        vkResult = vkCreateFramebuffer(vkDevice_sea, &vkFrameBufferCreateInfo, NULL, &vkFramebuffer_array_sea[i]);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "createFramebuffers(): vkCreateFramebuffer() Failed at {%d}!.\n", i);
            return (vkResult);
        } else {
            fprintf(fptr, "createFramebuffers(): vkCreateFramebuffer() Successful for {%d}!.\n", i);
        }
    }

    return (vkResult);
}

VkResult createSemaphores(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Create Semaphore info
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;
    vkSemaphoreCreateInfo.flags = 0; // it's reserved must be zero

    // By defualt if no type is specified, binary semaphore is created!

    // create semaphore for backbuffer
    vkResult = vkCreateSemaphore(vkDevice_sea, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backbuffer_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSemaphores(): vkCreateSemaphore() Failed for Back Buffer Semaphore!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSemaphores(): vkCreateSemaphore() Successful for Back Buffer Semaphore!.\n");
    }

    // create semaphore for render complete
    vkResult = vkCreateSemaphore(vkDevice_sea, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_rendercomplete_sea);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSemaphores(): vkCreateSemaphore() Failed for Render Complete Semaphore!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSemaphores(): vkCreateSemaphore() Successful for Render Complete Semaphore!.\n");
    }

    return (vkResult);
}

VkResult createFences(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // VkFenceCreateInfo
    VkFenceCreateInfo vkFenceCreateInfo;
    memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkFence_array_sea = (VkFence*) malloc(sizeof(VkFence) * swapchainImageCount_sea);


    for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
        vkResult = vkCreateFence(vkDevice_sea, &vkFenceCreateInfo, NULL, &vkFence_array_sea[i]);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "createFences(): vkCreateFence() Failed at {%d}!.\n", i);
            return (vkResult);
        } else {
            fprintf(fptr, "createFences(): vkCreateFence() Successful for {%d}!.\n", i);
        }
    }

    return (vkResult);
}

VkResult buildCommandBuffers(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    for(uint32_t i = 0; i < swapchainImageCount_sea; i++) {
        vkResult = vkResetCommandBuffer(vkCommandBuffer_array_sea[i], 0);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "buildCommandBuffers(): vkResetCommandBuffer() Failed for {%d}!.\n", i);
            return (vkResult);
        }

        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.flags = 0;

        vkResult = vkBeginCommandBuffer(vkCommandBuffer_array_sea[i], &vkCommandBufferBeginInfo);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "buildCommandBuffers(): vkBeginCommandBuffer() Failed for {%d}!.\n", i);
            return (vkResult);
        }

        VkClearValue vkClearValue_array[2];
        memset((void*)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));

        vkClearValue_array[0].color = vkClearColorValue_sea;
        vkClearValue_array[1].depthStencil = vkClearDepthStencilValue_sea;

        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = vkRenderPass_sea;
        vkRenderPassBeginInfo.renderArea.offset.x = 0;
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain_sea.width;
        vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain_sea.height;
        vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
        vkRenderPassBeginInfo.framebuffer = vkFramebuffer_array_sea[i];

        vkCmdBeginRenderPass(vkCommandBuffer_array_sea[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(vkCommandBuffer_array_sea[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline_sea);

        vkCmdBindDescriptorSets(
            vkCommandBuffer_array_sea[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vkPipelineLayout_sea,
            0, 1,
            &vkDescriptorSet_sea,
            0, NULL
        );

        VkDeviceSize vkDeviceSize_offset_position_array[1];
        memset((void*)vkDeviceSize_offset_position_array, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position_array));

        vkCmdBindVertexBuffers(
            vkCommandBuffer_array_sea[i],
            AMK_ATTRIBUTE_POSITION, 1,
            &vertexData_position_sea.vkBuffer,
            vkDeviceSize_offset_position_array
        );

        vkCmdDraw(vkCommandBuffer_array_sea[i], (uint32_t)vertexData_array_sea.size(), 1, 0, 0);

        ImDrawData* drawData = ImGui::GetDrawData();
        if(drawData != NULL) {
            ImGui_ImplVulkan_RenderDrawData(drawData, vkCommandBuffer_array_sea[i]);
        }

        vkCmdEndRenderPass(vkCommandBuffer_array_sea[i]);

        vkResult = vkEndCommandBuffer(vkCommandBuffer_array_sea[i]);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "buildCommandBuffers(): vkEndCommandBuffer() Failed for {%d}!.\n", i);
            return (vkResult);
        }
    }

    return (vkResult);
}

// Always Keep this function at the end of this file
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
    VkDebugReportFlagsEXT vkDebugReportFlagsEXIT, 
    VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXIT, 
    uint64_t object, 
    size_t location, 
    int32_t messageCode, 
    const char* pLayerPrefix, 
    const char* pMessage, 
    void* pUserData
) {
    fprintf(fptr, "AMK_VALIDATION: debugReportCallback() :  %s (%d) = %s\n", pLayerPrefix, messageCode, pMessage);
    return VK_FALSE; // return false to ignore this message
}