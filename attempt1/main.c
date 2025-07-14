#include <stdio.h>
#include <stdbool.h>
#pragma warning(push)
#pragma warning(disable:4668)
#include <windows.h>
#pragma warning(pop)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY:
            {
                OutputDebugStringA("destroying");
                PostQuitMessage(0);
                return 0;
            }
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW));
                EndPaint(hwnd, &ps);
                return 0;
            }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
) {
    OutputDebugStringA("OHR!");


    // Register the window class.
    static char className[]  = "OHR Window Class";
    static char windowTitle[]  = "OHR Window Title";

    WNDCLASSEX wcex = { };

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = className;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if(!RegisterClassExA(&wcex)) {
        OutputDebugStringA("failed to register class\n");
        exit(1);
    }

    HWND hwnd = CreateWindowExA(
        0,
        className,
        windowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        OutputDebugStringA("no window!");
        exit(1);
    }

    ShowWindow(hwnd, nShowCmd);

    MSG msg = {};
    boolean should_run = true;
    while (should_run) {
        while (PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                should_run = false;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
}
