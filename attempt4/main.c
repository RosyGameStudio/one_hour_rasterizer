#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

f64 PI = 3.14159265;

int bitmap_width;
int bitmap_height;
void *bitmap_bits;
int pixel_byte_size = 4;
int bitmap_full_size;
f64 t = 0.0;
bool should_run = true;


void OHRDraw(void) {
    int pitch = bitmap_width * pixel_byte_size;
    u8 *row = (u8 *)bitmap_bits;
    u8 *pixel = (u8 *)row;

    f64 blue_duration = 100.0;
    f64 green_duration = 150.0;
    f64 red_duration = 50.0;


    f64 blue_mod = (fmod(t, blue_duration) / blue_duration) * 2 * PI; // 0 - 360 in radians
    blue_mod = sin(blue_mod); // -1 to 1
    blue_mod = (blue_mod + 1.0) / 2.0;

    f64 green_mod = (fmod(t, green_duration) / green_duration) * 2 * PI; // 0 - 360 in radians
    green_mod = sin(green_mod); // -1 to 1
    green_mod = (green_mod + 1.0) / 2.0;

     f64 red_mod = (fmod(t, red_duration) / red_duration) * 2 * PI; // 0 - 360 in radians
    red_mod = sin(red_mod); // -1 to 1
    red_mod = (red_mod + 1.0) / 2.0;

    u8 blue = (u8)(blue_mod * 255.f);
    u8 green = (u8)(green_mod * 255.f);
    u8 red = (u8)(red_mod * 255.f);

    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            *pixel = blue;
            pixel++;

            *pixel = green;
            pixel++;

            *pixel = red;
            pixel++;

            *pixel = 0;
            pixel++;
        }
       row += pitch;
    }

}

LRESULT WndProc(
    HWND window,
    UINT msg,
    WPARAM w_param,
    LPARAM l_param) {
    switch (msg) {
        case WM_SIZE:
            {
                if (bitmap_bits) {
                    VirtualFree(bitmap_bits, 0, MEM_RELEASE);
                }

                RECT rect;
                if (!GetClientRect(window, &rect)) abort();
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;

                bitmap_width = width;
                bitmap_height = height;
                bitmap_full_size = bitmap_width * bitmap_height * pixel_byte_size;

                bitmap_bits = VirtualAlloc(0, bitmap_full_size, MEM_COMMIT, PAGE_READWRITE);
                if (!bitmap_bits) abort();

                return 0;
            }
        case WM_QUIT:
            {
                should_run = false;
                return 0;
            }
        case WM_DESTROY:
            {
                should_run = false;
                PostQuitMessage(0);
                return 0;
            }
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(window, &ps);
                // blits
                BITMAPINFO bitmap_info = {};
                bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
                bitmap_info.bmiHeader.biWidth = bitmap_width;
                bitmap_info.bmiHeader.biHeight = -bitmap_height;
                bitmap_info.bmiHeader.biBitCount = 32;
                bitmap_info.bmiHeader.biCompression = BI_RGB;
                bitmap_info.bmiHeader.biPlanes = 1;

                OHRDraw();

                RECT rect;
                if (!GetClientRect(window, &rect)) abort();
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;


                if (!StretchDIBits(hdc, 0, 0, width, height, 0, 0, bitmap_width, bitmap_height, (void *)bitmap_bits, &bitmap_info ,DIB_RGB_COLORS, SRCCOPY)) {
                    abort();
                }
                EndPaint(window, &ps);
            }
    }
    return DefWindowProcA(window, msg, w_param, l_param);
}

int WinMain(
    HINSTANCE app_instance,
    HINSTANCE prev_instance,
    LPSTR cmdline,
    int show_cmd) {
    OutputDebugStringA("OHR!\n");

    const char* class_name = "ohr_text";

    WNDCLASSEXA wnex = {0};
    wnex.cbSize = sizeof(wnex);
    wnex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wnex.hInstance = app_instance;
    wnex.lpszClassName = class_name;
    wnex.lpfnWndProc = WndProc;

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
        app_instance,
        NULL);
    if (!window) abort();

    ShowWindow(window, show_cmd);

    while (should_run) {
        MSG msg;
        while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                should_run = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!should_run) break;
        LARGE_INTEGER StartingTime, EndingTime;
        LARGE_INTEGER Frequency;
        f64 elapsed = 0.0;

        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&StartingTime);

        QueryPerformanceCounter(&EndingTime);
        elapsed = (f64)(EndingTime.QuadPart - StartingTime.QuadPart);

        elapsed *= 1000000.0;
        elapsed /= Frequency.QuadPart;

        f64 min = 0.25;
        if (elapsed < min) elapsed = min;

        t += elapsed;
        OHRDraw();

        BITMAPINFO bitmap_info = {};
        bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
        bitmap_info.bmiHeader.biWidth = bitmap_width;
        bitmap_info.bmiHeader.biHeight = -bitmap_height;
        bitmap_info.bmiHeader.biBitCount = 32;
        bitmap_info.bmiHeader.biCompression = BI_RGB;
        bitmap_info.bmiHeader.biPlanes = 1;

        HDC hdc = GetDC(window);

        RECT rect;
        if (!GetClientRect(window, &rect)) abort();
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;


        if (!StretchDIBits(hdc, 0, 0, width, height, 0, 0, bitmap_width, bitmap_height, (void *)bitmap_bits, &bitmap_info ,DIB_RGB_COLORS, SRCCOPY)) {
            abort();
        }
    }

}
