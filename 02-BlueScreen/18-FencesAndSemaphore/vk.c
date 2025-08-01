#include<windows.h>
#include<stdio.h>
#include<stdlib.h>

#include "vk.h"

// vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR
#include<vulkan/vulkan.h>

// vulkan related libraries
#pragma comment(lib, "vulkan-1.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global variables
BOOL gbFullScreen = FALSE;
DWORD dwStyle = 0;
WINDOWPLACEMENT wpPrev;
HWND ghwnd = NULL;
BOOL gbActiveWindow = FALSE;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
FILE *fptr = NULL;
const CHAR *gpszAppName = "ARTR: Vulkan";

// instance extension related variables
uint32_t enabledInstanceExtensionCount = 0; 
const char *enabledInstanceExtensionNames_array[2]; // VK_KHR_SURFACE_EXTENSION_NAME & VK_KHR_WIN32_SURFACE_EXTENSION_NAME
// vulkan instance
VkInstance vkInstance = VK_NULL_HANDLE;

//vulkan presentation surface object
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

// vulkan physical device related variables
VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

//
uint32_t physicalDeviceCount = 0;
VkPhysicalDevice *vkPhysicalDevice_array = NULL;

// Device Extension related variables
uint32_t enabledDeviceExtensionCount = 0;
const char *enabledDeviceExtensionNames_array[1]; // VK_KHR_SWAPCHAIN_EXTENSION_NAME

// Vulkan Device
VkDevice vkDevice = VK_NULL_HANDLE;

// Device Queue
VkQueue vkQueue = VK_NULL_HANDLE;

// Surface Format & Surcae ColorSpace
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// Presentation Mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

// Swapchain
int winWidth = WIN_WIDTH, winHeight = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

// Swapchain Images & Image Views
uint32_t swapchainImageCount = UINT32_MAX;
VkImage *swapchainImage_array = NULL;
VkImageView *swapchainImageView_array = NULL;

// Command Pool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

// Command Buffer
VkCommandBuffer *vkCommandBuffer_array;

// Render Pass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

// Frame Buffer
VkFramebuffer *vkFramebuffer_array = NULL;

// Fences & Semaphore
VkSemaphore vkSemaphore_backbuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_rendercomplete = VK_NULL_HANDLE;
VkFence *vkFence_array = NULL;

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {
    // Func
    VkResult initialize(void);
    void display(void);
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
            TEXT("AMK_Vulkan : Fences & Semaphore"),
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
            if(gbActiveWindow == TRUE) {
                update();
            }
            display();
        }
    }

    uninitialize();
    
    return((int)msg.wParam);
}


LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    //func
    void ToggleFullScreen(void);
    void resize(int, int);

    //var
    BOOL bIsMax = FALSE;

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
            resize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN:
            switch(wParam) {

                case VK_ESCAPE:
                    DestroyWindow(hwnd);
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
		ShowCursor(FALSE);
		gbFullScreen = TRUE;
	}
	else {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
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
    VkResult createRenderPass(void);
    VkResult createFramebuffers(void);
    VkResult createSemaphores(void);
    VkResult createFences(void);

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

    // Render Pass
    vkResult = createRenderPass();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createRenderPass() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createRenderPass() Successful!.\n\n");
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

    return (vkResult);
}

void resize(int width, int height) {

}

void display(void) {

}

void uninitialize(void){

    // to do toggle window full screen!

    if(ghwnd) {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // wait til vkDevice is idle
    if(vkDevice) {
        vkDeviceWaitIdle(vkDevice); // this basically waits on til all the operations are done using the device and then this function call returns
        fprintf(fptr, "\nuninitialize(): vkDeviceWaitIdle is done!\n");
    }

    // Destroy Fence
    for(uint32_t i = 0; i < swapchainImageCount; i++) {
        vkDestroyFence(vkDevice, vkFence_array[i], NULL);
        fprintf(fptr, "uninitialize(): vkDestroyFence() Succeed for {%d}!.\n", i);
        vkFence_array[i] = VK_NULL_HANDLE;
    }

    if(vkFence_array) {
        free(vkFence_array);
        fprintf(fptr, "uninitialize(): freed vkFence_array!.\n");
        vkFence_array = NULL;
    }

    // Destroy Semaphore
    if(vkSemaphore_rendercomplete) {
        vkDestroySemaphore(vkDevice, vkSemaphore_rendercomplete, NULL);
        fprintf(fptr, "uninitialize(): vkDestroySemaphore() for Render Complete Succeed!\n");
        vkSemaphore_rendercomplete = VK_NULL_HANDLE;
    }

    if(vkSemaphore_backbuffer) {
        vkDestroySemaphore(vkDevice, vkSemaphore_backbuffer, NULL);
        fprintf(fptr, "uninitialize(): vkDestroySemaphore() for Back Buffer Succeed!\n");
        vkSemaphore_backbuffer = VK_NULL_HANDLE;
    }

    // Destroy Frame Buffers
    for(uint32_t i = 0; i < swapchainImageCount; i++) {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
        fprintf(fptr, "uninitialize(): vkDestroyFramebuffer() Succeed for {%d}!.\n", i);
        vkFramebuffer_array[i] = VK_NULL_HANDLE;
    }

    if(vkFramebuffer_array) {
        free(vkFramebuffer_array);
        fprintf(fptr, "uninitialize(): freed vkFramebuffer_array!.\n");
        vkFramebuffer_array = NULL;
    }

    // Destroy Render Pass
    if(vkRenderPass) {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyRenderPass() Succeed!\n");
        vkRenderPass = VK_NULL_HANDLE;
    }

    // Destroy  Command Buffers
    if(vkCommandBuffer_array) {
        for(uint32_t i = 0; i < swapchainImageCount; i++) {
            vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
            fprintf(fptr, "uninitialize(): vkFreeCommandBuffers() Succeed for {%d}\n", i);
            vkCommandBuffer_array[i] = VK_NULL_HANDLE;
        }
    }

    if(vkCommandBuffer_array) {
        free(vkCommandBuffer_array);
        fprintf(fptr, "uninitialize(): freed vkCommandBuffer_array!.\n");
        vkCommandBuffer_array = NULL;
    }

    // Destroy the command pool
    if(vkCommandPool) {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        fprintf(fptr, "uninitialize(): vkDestroyCommandPool Successful!.\n");
        vkCommandPool = VK_NULL_HANDLE;
    }

    // Destroy Vulkan Swapchain Image Views
    if(swapchainImageView_array) {
        for(uint32_t i = 0; i < swapchainImageCount; i++) {
            if(swapchainImageView_array[i]) {
                vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
                fprintf(fptr, "uninitialize(): vkDestroyImageView() Succeed for {%d}\n", i);
                swapchainImageView_array[i] = VK_NULL_HANDLE;
            }
        }
    }

    if(swapchainImageView_array) {
        free(swapchainImageView_array);
        fprintf(fptr, "uninitialize(): freed swapchainImageView_array!.\n");
        swapchainImageView_array = NULL;
    }

    /* // Destroy vulkan Images
    if(swapchainImage_array) {
        for(uint32_t i = 0; i < swapchainImageCount; i++) {
            if(swapchainImage_array[i]) {
                vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
                fprintf(fptr, "uninitialize(): vkDestroyImage() Succeed for {%d}\n", i);
                swapchainImage_array[i] = VK_NULL_HANDLE;
            }
        }
    } */

    if(swapchainImage_array) {
        free(swapchainImage_array);
        fprintf(fptr, "uninitialize(): freed swapchainImage_array!.\n");
        swapchainImage_array = NULL;
    }


    // Destroy Vulkan Swapchain
    if(vkSwapchainKHR) {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
    }
    
    // No need to destroy device queue

    // Destroy Vulkan Device
    if(vkDevice) {
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
    }
    
    //No need to destroy selected physical device!

    // destroy surface
    if(vkSurfaceKHR) {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
        vkSurfaceKHR = VK_NULL_HANDLE;
		fprintf(fptr,"uninitialize(): vkDestroySurfaceKHR() Succeed\n");
    }

    // destroy vkInstance
    if(vkInstance) {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
		fprintf(fptr,"uninitialize(): vkDestroyInstance() Succeed\n");
    }

	if(fptr){
		fprintf(fptr,"uninitialize(): File Closed Successfully..\n");
        fclose(fptr);
		fptr = NULL;
	}
}

void update(void) {

}

//! //////////////////////////////////////// Definations of vulkan Related Functions ///////////////////////////////////////////////

VkResult createVulkanInstance (void) {
    // function declarations
    VkResult fillInstanceExtensionNames(void);

    // varibales
    VkResult vkResult = VK_SUCCESS;

    // code
    vkResult = fillInstanceExtensionNames();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVulkanInstance(): fillInstanceExtensionNames() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVulkanInstance(): fillInstanceExtensionNames() Successful!.\n");
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
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;

    // Step 4: Create instance using vkCreateInstance
    vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
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

    // Step 3: 
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
    for(uint32_t i = 0; i < instanceExtensionCount; i++) {
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0) {
            surfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) ==  0) {
            win32vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
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

    // step 8:
    for(uint32_t i = 0; i < enabledInstanceExtensionCount; i++) {
        fprintf(fptr, "fillInstanceExtensionNames(): Enabled Vulkan Instance Extension Name = %s \n", enabledInstanceExtensionNames_array[i]);
    }

    return vkResult;
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
        vkInstance, 
        &vkWin32SurfaceCreateInfoKHR,
        NULL,
        &vkSurfaceKHR
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
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() First Call Failed!.\n");
        return (vkResult);
    } else if(physicalDeviceCount == 0) {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() Resulted in Zero Physical Devices!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() First Call Successful!.\n");
    }

    vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);

    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() Second Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "getPhysicalDevice(): vkEnumeratePhysicalDevices() Second Call Successful!.\n");
    }

    VkBool32 bFound = VK_FALSE;

    for(uint32_t i = 0; i < physicalDeviceCount; i++) {
        uint32_t queueCount = UINT32_MAX;

        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);
        VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

        VkBool32 *isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * queueCount);

        for(uint32_t j = 0; j < queueCount; j++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(
                vkPhysicalDevice_array[i],
                j,
                vkSurfaceKHR,
                &isQueueSurfaceSupported_array[j]
            );
        }

        for(uint32_t j = 0; j < queueCount; j++) {
            if(vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT
                && isQueueSurfaceSupported_array[j] == VK_TRUE) {
                vkPhysicalDevice_selected = vkPhysicalDevice_array[i];
                graphicsQueueFamilyIndex_selected = j;
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
        if(vkPhysicalDevice_array) {
            free(vkPhysicalDevice_array);
            vkPhysicalDevice_array = NULL;
            fprintf(fptr, "getPhysicalDevice(): freed vkPhysicalDevice_array!.\n");
        }
        fprintf(fptr, "getPhysicalDevice(): Failed to get required graphics enabled physical device!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected, &vkPhysicalDeviceMemoryProperties);

    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected, &vkPhysicalDeviceFeatures);

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

    for(uint32_t i = 0; i < physicalDeviceCount; i++) {

        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));

        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);

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

    if(vkPhysicalDevice_array) {
        free(vkPhysicalDevice_array);
        vkPhysicalDevice_array = NULL;
        fprintf(fptr, "printVKInfo(): freed vkPhysicalDevice_array!.\n");
    }

    return (vkResult);
}

VkResult fillDeviceExtensionNames (void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Step 1: Find how many devices extension are supported by this vulkan driver & keep it in local variable
    uint32_t devicesExtensionCount = 0;

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &devicesExtensionCount, NULL);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "fillDeviceExtensionNames(): vkEnumerateDeviceExtensionProperties() First Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "fillDeviceExtensionNames(): vkEnumerateDeviceExtensionProperties() First Call Successful!.\n");
    }

    // step 2: Allocate & fill struct vk Extenstions array correspoinding to above count
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * devicesExtensionCount);
    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &devicesExtensionCount, vkExtensionProperties_array);
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
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
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
    for(uint32_t i = 0; i < enabledDeviceExtensionCount; i++) {
        fprintf(fptr, "fillDeviceExtensionNames(): Enabled Vulkan Device Extension Name = %s \n", enabledDeviceExtensionNames_array[i]);
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
    vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    // initialize VkDeviceCreateInfo structure
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.enabledLayerCount = 0; // these are deprecated in current version
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL; // these are deprecated in current version
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    // !NEWLY ADDED CODE : set VkDeviceQueueCreateInfo
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

    vkResult = vkCreateDevice(
        vkPhysicalDevice_selected,
        &vkDeviceCreateInfo,
        NULL, &vkDevice
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
        vkDevice,
        graphicsQueueFamilyIndex_selected,
        0, &vkQueue
    );

    if(vkQueue == VK_NULL_HANDLE) {
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
        vkPhysicalDevice_selected, 
        vkSurfaceKHR, &formatCount,
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
        vkPhysicalDevice_selected, 
        vkSurfaceKHR, &formatCount,
        vkSurfaceFormatKHR_array
    );
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDeviceSurfaceFormatAndColorSpace(): vkGetPhysicalDeviceSurfaceFormatsKHR() Second Call Failed!.\n");
    } else {
        fprintf(fptr, "getPhysicalDeviceSurfaceFormatAndColorSpace(): vkGetPhysicalDeviceSurfaceFormatsKHR() Second Call Successful!.\n");
    }

    // Decide the surface color format first!
    if(formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED) {
        vkFormat_color = VK_FORMAT_B8G8R8G8_422_UNORM;
    } else {
        vkFormat_color = vkSurfaceFormatKHR_array[0].format;
    }

    // Decide the Color Space
    vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;

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
        vkPhysicalDevice_selected, 
        vkSurfaceKHR, &modeCount,
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
        vkPhysicalDevice_selected, 
        vkSurfaceKHR, &modeCount,
        vkPresentModeKHR_array
    );
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): vkGetPhysicalDeviceSurfacePresentModesKHR() Second Call Failed!.\n");
    } else {
        fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): vkGetPhysicalDeviceSurfacePresentModesKHR() Second Call Successful!.\n");
    }

    for(uint32_t i = 0 ;  i < modeCount; i++) {
        if(vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            vkPresentModeKHR = vkPresentModeKHR_array[i];
            fprintf(fptr, "getPhysicalDeviceSurfacePresentMode(): VK_PRESENT_MODE_MAILBOX_KHR Present Mode found!.\n");
            break;
        }
    }

    if(vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR) {
        // since we don't have mailbox as supported format let's settle for FIFO then!
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
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

    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
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
    memset((void*)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));

    if(vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX) {
        vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;

        fprintf(
            fptr, 
            "createSwapchain(): Swapchain Image Width : %d X Height : %d\n", 
            vkExtent2D_swapchain.width, vkExtent2D_swapchain.height
        );
    } else {
        // if surface size is already defined then swapchain image size must match with it!
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));

        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;

        vkExtent2D_swapchain.width = max(vkSurfaceCapabilitiesKHR.minImageExtent.width, min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
        vkExtent2D_swapchain.height = max(vkSurfaceCapabilitiesKHR.minImageExtent.height, min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));

        fprintf(
            fptr, 
            "createSwapchain(): Swapchain Image (Derived from best of minImageExtent, maxImageExtent & Window Size) Width  : %d X Height : %d\n", 
            vkExtent2D_swapchain.width, vkExtent2D_swapchain.height
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
    vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchain(): vkCreateSwapchainKHR() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchain(): vkCreateSwapchainKHR() Successful!.\n");
    }

    return (vkResult);
}

VkResult createSwapchainImagesAndImageViews(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Step 1: Get Swapchain Image Count
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() First Call Failed!.\n");
        return (vkResult);
    } else if( swapchainImageCount == 0) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() Failed: 0 Swapchain Images found!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() Successful!. : Swapchain Image Count : [%d]\n", swapchainImageCount);
    }

    // Step 2: Allocate Swapchain Image Array
    swapchainImage_array = (VkImage*)malloc(sizeof(VkImage) * swapchainImageCount);

    // Step 3: Fill Swapchain Image Array
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() Second Call Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews(): vkGetSwapchainImagesKHR() Second Call Successful!.\n");
    }

    // Setp 4: Allocate Swapchain Image Views Array
    swapchainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * swapchainImageCount);

    // Step 5: vkCreateImageView for each Swapchain Image
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_color;
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
    for(uint32_t i = 0; i < swapchainImageCount; i++) {
        vkImageViewCreateInfo.image = swapchainImage_array[i];
        vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[i]);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "createSwapchainImagesAndImageViews(): vkCreateImageView() Failed at {%d}!.\n", i);
            return (vkResult);
        } else {
            fprintf(fptr, "createSwapchainImagesAndImageViews(): vkCreateImageView() Successful for {%d}!.\n", i);
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
    vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;

    vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);
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
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    // Step 2: Allocate Command Buffer Array to the size of swapchainImageCount
    vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapchainImageCount);

    // Step 3: Allocat eeach command buffer in loop with allocateInfo struct
    for(uint32_t i = 0; i < swapchainImageCount; i++) {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "createCommandBuffers(): vkCreatvkAllocateCommandBufferseImageView() Failed at {%d}!.\n", i);
            return (vkResult);
        } else {
            fprintf(fptr, "createCommandBuffers(): vkAllocateCommandBuffers() Successful for {%d}!.\n", i);
        }
    }

    return (vkResult);
}


VkResult createRenderPass(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Code
    //Step 1: Create Attachment Description stcture array
    VkAttachmentDescription vkAttachmentDescription_array[1];
    memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));

    vkAttachmentDescription_array[0].flags = 0;
    vkAttachmentDescription_array[0].format =  vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Step 2: Create Attachment Reference Structure
    VkAttachmentReference vkAttachmentReference;
    memset((void*)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));

    vkAttachmentReference.attachment = 0; // From the array of attachment description, refer to 0th index, oth will be color attachment
    vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 

    // STep 3: create sub pass description strcture
    VkSubpassDescription vkSubpassDescription;
    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
    
    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = 1;
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = NULL;
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
        vkDevice,
        &vkRenderPassCreateInfo,
        NULL,
        &vkRenderPass
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createRenderPass(): vkCreateRenderPass() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createRenderPass(): vkCreateRenderPass() Successful!.\n");
    }

    return (vkResult);
}

VkResult createFramebuffers(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Step 1: create VkImageView array same as size of attachments
    VkImageView vkImageView_attachments_array[1];
    memset((void*)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

    // Step 2: Create VkFrameBufferCreateInfo structure
    VkFramebufferCreateInfo vkFrameBufferCreateInfo;
    memset((void*)&vkFrameBufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

    vkFrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFrameBufferCreateInfo.flags = 0;
    vkFrameBufferCreateInfo.pNext = NULL;
    vkFrameBufferCreateInfo.renderPass = vkRenderPass;
    vkFrameBufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
    vkFrameBufferCreateInfo.pAttachments = vkImageView_attachments_array;
    vkFrameBufferCreateInfo.width = vkExtent2D_swapchain.width;
    vkFrameBufferCreateInfo.height = vkExtent2D_swapchain.height;
    vkFrameBufferCreateInfo.layers = 1;

    // allocate frame buffers array and creat efream buffers in loop with counts of allocated swapchain images
    vkFramebuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchainImageCount);
    for(uint32_t i = 0; i < swapchainImageCount; i++) {

        vkImageView_attachments_array[0] = swapchainImageView_array[i];

        vkResult = vkCreateFramebuffer(vkDevice, &vkFrameBufferCreateInfo, NULL, &vkFramebuffer_array[i]);
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
    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backbuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSemaphores(): vkCreateSemaphore() Failed for Back Buffer Semaphore!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSemaphores(): vkCreateSemaphore() Successful for Back Buffer Semaphore!.\n");
    }

    // create semaphore for render complete
    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_rendercomplete);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSemaphores(): vkCreateSemaphore() Failed for Render Complete Semaphore!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSemaphores(): vkCreateSemaphore() Successful for Render Complete+ Semaphore!.\n");
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

    vkFence_array = (VkFence*) malloc(sizeof(VkFence) * swapchainImageCount);


    for(uint32_t i = 0; i < swapchainImageCount; i++) {
        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
        if(vkResult != VK_SUCCESS) {
            fprintf(fptr, "createFences(): vkCreateFence() Failed at {%d}!.\n", i);
            return (vkResult);
        } else {
            fprintf(fptr, "createFences(): vkCreateFence() Successful for {%d}!.\n", i);
        }
    }

    return (vkResult);
}