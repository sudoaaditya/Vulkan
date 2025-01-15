#include<windows.h>
#include<stdio.h>

#include "vk.h"

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

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {
    // Func
    int initialize(void);
    void display(void);
    void update(void);
    void uninitialize(void);
    
    // Vars
    WNDCLASSEX wndclass;
    MSG msg;
    HWND hwnd;
    TCHAR szAppName[] = TEXT("Vulkan Window");
    BOOL bDone = FALSE;
    int iRet = 0;

    fptr = fopen("_VulkanWindowLog.txt", "w");
    if(fptr == NULL) {
        MessageBox(NULL, TEXT("Cannot Create Log!!.."), TEXT("ErrMsg"), MB_OK);
        exit(0);
    }
    else {
        fprintf(fptr, "Log Created Successful!!\n\n");
    }

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

    iRet = initialize();
    if(iRet != 0) {
        fprintf(fptr, "WinMain: Initialization Failed!!!...\n");
        DestroyWindow(hwnd);
    } else {
        fprintf(fptr, "WinMain: Initialization Successful!!!...\n");
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

        case WM_ERASEBKGND:
            return(0);

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


int initialize(void) {



    return(0);
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

	if(fptr){
		fprintf(fptr,"\nuninitialize:File Closed Successfully..\n");
        fclose(fptr);
		fptr = NULL;
	}
}

void update(void) {

}