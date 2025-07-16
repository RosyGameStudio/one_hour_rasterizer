#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#pragma warning(push)
#pragma warning(disable: 4668)
#include <windows.h>
#pragma warning(pop)

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

static int bitmap_width;
static int bitmap_height;
static void *bitmap_bits;
static int pixel_byte_size = 4;
static BITMAPINFO bitmap_info = {0};

void OHRDraw(u64 t) {
    int pitch = bitmap_width * pixel_byte_size;
    u8 *Row = (u8 *)bitmap_bits;
    u8 *Pixel = (u8 *)Row;

    float dt = (float)t;
    float duration = 10000;
    float current_t = fmodf(dt, duration) / duration;

    const u8 blue = (u8)floor(255.f * current_t);

    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            *Pixel = blue;
            Pixel++;

            *Pixel = 0;
            Pixel++;

            *Pixel = (u8)255;
            Pixel++;

            *Pixel = 0;
            Pixel++;
        }
        Row += pitch;
    }
}

LRESULT Wndproc(
  HWND window,
  UINT msg,
  WPARAM wParam,
  LPARAM lParam
)
{
    switch (msg) {
        case WM_SIZE:
            {
                if (bitmap_bits) {
                    VirtualFree(&bitmap_bits, 0, MEM_RELEASE);
                }
                RECT rect;
                if (!GetClientRect(window, &rect)) abort();
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;

                bitmap_width = width;
                bitmap_height = height;

                bitmap_bits = VirtualAlloc(0, bitmap_width*bitmap_height*pixel_byte_size, MEM_COMMIT, PAGE_READWRITE);
                if (!bitmap_bits) abort(); // lol
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

                RECT rect;
                if (!GetClientRect(window, &rect)) abort();
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;

                bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
                bitmap_info.bmiHeader.biWidth = bitmap_width;
                bitmap_info.bmiHeader.biHeight = bitmap_height;
                bitmap_info.bmiHeader.biPlanes = 1;
                bitmap_info.bmiHeader.biBitCount = 32;
                bitmap_info.bmiHeader.biCompression  = BI_RGB;

                int res = StretchDIBits(hdc, 0, 0, width, height, 0, 0, bitmap_width, bitmap_height, (void *)bitmap_bits, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
                if (!res) abort();

                EndPaint(window, &ps);
                return 0;
            }
    }
    return DefWindowProcA(window, msg, wParam, lParam);
}

int WinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR     lpCmdLine,
   int       nShowCmd
) {
    OutputDebugStringA("OHR!\n");

    char *class_name = "OHRClassName";

    WNDCLASSEXA wnex = {0};
    wnex.cbSize = sizeof(WNDCLASSEX);
    wnex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wnex.hInstance = hInstance;
    wnex.lpfnWndProc = Wndproc;
    wnex.lpszClassName = class_name;

    RegisterClassExA(&wnex);

    HWND window = CreateWindowExA(
        0,
        class_name,
        "OHR!",
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

    if (!window) abort();

    ShowWindow(window, nShowCmd);

    LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
    LARGE_INTEGER Frequency;


    u64 t = 0;
    boolean should_run = true;
    while (should_run) {
        MSG msg;
        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&StartingTime);
        while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                should_run = false;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        QueryPerformanceCounter(&EndingTime);
        ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
        ElapsedMicroseconds.QuadPart *= 1000000;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
        u64 dt = (u64)ElapsedMicroseconds.QuadPart / 1000;
        if (dt < 100) dt = 100;
        t += dt;

        bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
        bitmap_info.bmiHeader.biWidth = bitmap_width;
        bitmap_info.bmiHeader.biHeight = bitmap_height;
        bitmap_info.bmiHeader.biPlanes = 1;
        bitmap_info.bmiHeader.biBitCount = 32;
        bitmap_info.bmiHeader.biCompression  = BI_RGB;

        RECT rect;
        if (!GetClientRect(window, &rect)) abort();
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        OHRDraw(t);

        HDC hdc = GetDC(window);
        int res = StretchDIBits(hdc, 0, 0, width, height, 0, 0, bitmap_width, bitmap_height, (void *)bitmap_bits, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
        if (!res) abort();

    }
}
