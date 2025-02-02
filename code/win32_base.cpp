/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#include <windows.h>
#include <WinUser.h>


LRESULT CALLBACK
MainWindowCallback(HWND Window,
        UINT Message,
        WPARAM WParam,
        LPARAM LParam) {
    switch (Message) {
        case WM_SIZE:
            {
                OutputDebugStringA("WM_SIZE\n");
            } break;
        case WM_DESTROY:
            {
                OutputDebugStringA("WM_DESTROY\n");
            } break;
        case WM_CLOSE:
            {
                OutputDebugStringA("WM_CLOSE\n");
            } break;
        case WM_ACTIVATEAPP:
            {
                OutputDebugStringA("WM_ACTIVATEAPP\n");
            } break;
        default:
            {
//                OutputDebugStringA("default\n");
            } break;
    }
}


int WINAPI
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        PSTR CommandLine,
        int ShowCode) {
    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWinDowCallback;
    WindowClass.hInstance = Instance;
//    WindowClass.hIcon = ;
    WindowClass.lpszClassName = "SteamProjectWindowClass";

    return (0);
}
