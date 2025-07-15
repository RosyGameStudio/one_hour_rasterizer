#include <stdbool.h>
#include <stdint.h>
#pragma warning(push)
#pragma warning(disable:4668)
#include <windows.h>
#pragma warning(pop)

typedef uint8_t u8;
typedef uint32_t u32;

static u8 *bm;
static u32 byteSize = 4;
static u32 bmiWidth;
static u32 bmiHeight;

void OHRUpdateWindow(HDC hdc, u32 windowWidth, u32 winHeight) {
    OutputDebugStringA("Update window?\n");
    //
    for (u8 x = 0; x < bmiWidth; x++) {
        for (u8 y = 0; x < bmiWidth; x++) {
            //
        }
    }
}

LRESULT Wndproc(
  HWND window,
  UINT message,
  WPARAM wParam,
  LPARAM lParam
)
{
    switch (message) {
        case WM_SIZE:
            {
                RECT wrect;
                if (!GetClientRect(window, &wrect)) abort();
                u32 width = wrect.right - wrect.left;
                u32 height = wrect.bottom - wrect.top;

                bmiWidth = width;
                bmiHeight = height;

                if (bm) {
                    VirtualFree(&bm, 0, MEM_RELEASE);
                }
                bm = VirtualAlloc(0, width*height*byteSize, MEM_COMMIT, PAGE_READWRITE);
                OutputDebugStringA("size\n");
                return 0;
            }
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            }
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(window, &ps);


                RECT wrect;
                if (!GetClientRect(window, &wrect)) abort();
                u32 width = wrect.right - wrect.left;
                u32 height = wrect.bottom - wrect.top;

                OHRUpdateWindow(hdc, width, height);

                BITMAPINFO bminfo = {0};
                bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bminfo.bmiHeader.biWidth = bmiWidth;
                bminfo.bmiHeader.biHeight = bmiHeight;
                bminfo.bmiHeader.biPlanes = 1;
                bminfo.bmiHeader.biBitCount = 32;
                bminfo.bmiHeader.biCompression = BI_RGB;


                if (!StretchDIBits(hdc, 0, 0, width, height, 0, 0, bmiWidth, bmiHeight, (void *)bm, &bminfo, 0, 0)) {
                    abort();
                }
                EndPaint(window, &ps);
                return 0;
            }
    }
    return DefWindowProc(window, message, wParam, lParam);
}

int WinMain(
    HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR     lpCmdLine,
    int       nShowCmd
) {
    OutputDebugStringA("OHR!\n");

    char *OHRClassName = "OHRClassName";

    WNDCLASSEXA wnex = {0};
    wnex.cbSize = sizeof(WNDCLASSEX);
    wnex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wnex.hInstance = hInstance;
    wnex.lpfnWndProc = Wndproc;
    wnex.lpszClassName = OHRClassName;

    RegisterClassExA(&wnex);

    HWND window = CreateWindowExA(
        0,
        OHRClassName,
        "OHR!",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!window) abort();


    ShowWindow(window, nShowCmd);

    boolean should_run = true;
    while (should_run) {
        MSG msg = {0};
        while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE) != 0) {
            if (msg.message == WM_QUIT) {
                should_run = false;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    return 0;
}
