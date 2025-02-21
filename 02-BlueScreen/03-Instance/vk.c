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
DWORD dwStyle = NULL;
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
            TEXT("Vulkan"),
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

    // varibales
    VkResult vkResult = VK_SUCCESS;

    // code
    vkResult = createVulkanInstance();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "initialize(): createVulkanInstance() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "initialize(): createVulkanInstance() Successful!.\n");
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


    // destroy vkInstance
    if(vkInstance) {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
		fprintf(fptr,"\nuninitialize(): vkDestroyInstance() Succeed\n");

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