#include <stdbool.h>
#include <stdint.h>
#include <windows.h>
#include <math.h>

static bool should_run = true;
typedef uint8_t u8;
typedef uint32_t u32;
typedef float f32;
typedef double f64;

static int bitmap_width;
static int bitmap_height;
static void *bitmap_bits;
static int pixel_num_bytes = 4;
static int bitmap_size;

static f64 t;
static f64 PI = 3.1415926535897932385;

typedef struct {
    f32 x;
    f32 y;
} Pos;

Pos pixel_to_coord(f32 x, f32 y) {
    Pos c = {0};
    c.x = (x/((f32)bitmap_width) * 2.0) - 1.0;
    c.y = -1 * (y/((f32)bitmap_height) * 2.0) - 1.0;
    return c;

}

void OHRDraw() {
    int pitch = bitmap_width * pixel_num_bytes;
    u8 *row = (u8 *)bitmap_bits;
    u8 *pixel = (u8 *)row;

    f64 duration = 5000.0;

    f64 color_mod = 1.0;
    color_mod = fmod(t, duration)/duration; // 0 to 1
    color_mod = sin(color_mod * 2 * PI); // -1 to 1
    color_mod = (color_mod + 1) / 2;

    u8 min = 75;

    u8 mod = (u8)(255.0 * color_mod);

    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            Pos c = pixel_to_coord((f32)x, (f32)y);
            if (c.x < 0 && c.y < 0) {
                *pixel = mod;
                pixel++;

                *pixel = mod;
                pixel++;

                *pixel = mod;
                pixel++;

                *pixel = 0;
                pixel++;

            } else if (c.x < 0) {
                *pixel = min;
                pixel++;

                *pixel = mod;
                pixel++;

                *pixel = min;
                pixel++;

                *pixel = 0;
                pixel++;

            } else if (c.y < 0) {
                *pixel = min;
                pixel++;

                *pixel = min;
                pixel++;

                *pixel = mod;
                pixel++;

                *pixel = 0;
                pixel++;

            } else {
                *pixel = mod;
                pixel++;

                *pixel = min;
                pixel++;

                *pixel = min;
                pixel++;

                *pixel = 0;
                pixel++;
            }

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

                bitmap_size = bitmap_width * bitmap_height * pixel_num_bytes;
                bitmap_bits = VirtualAlloc(0, bitmap_size, MEM_COMMIT, PAGE_READWRITE);
                if (!bitmap_bits) abort();
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

                BITMAPINFO bm_info = {0};
                bm_info.bmiHeader.biSize = sizeof(bm_info.bmiHeader);
                bm_info.bmiHeader.biWidth = bitmap_width;
                bm_info.bmiHeader.biHeight = -bitmap_height;
                bm_info.bmiHeader.biBitCount = 32;
                bm_info.bmiHeader.biPlanes = 1;
                bm_info.bmiHeader.biCompression = BI_RGB;

                RECT rect;
                if (!GetClientRect(window, &rect)) abort();
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;

                OHRDraw();

                if (!StretchDIBits(hdc, 0, 0, width, height, 0, 0, bitmap_width, bitmap_height, (void *)bitmap_bits, &bm_info, DIB_RGB_COLORS, SRCCOPY)) {
                    abort();
                }

                EndPaint(window, &ps);
                return 0;
            }

    }
    return DefWindowProcA(window, msg, w_param, l_param);
}

int WinMain(HINSTANCE app_instance,
            HINSTANCE prev_instance,
            LPSTR cmd_line,
            int show_cmd) {
    OutputDebugStringA("OHR!\n");


    const char* class_name = "OHRClass";

    WNDCLASSEXA wnex = {0};

    wnex.cbSize = sizeof(wnex);
    wnex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
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

    while(should_run)  {
        MSG msg;
        while(PeekMessage(&msg, window, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                should_run = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!should_run) break;

        HDC hdc = GetDC(window);
        BITMAPINFO bm_info = {0};
        bm_info.bmiHeader.biSize = sizeof(bm_info.bmiHeader);
        bm_info.bmiHeader.biWidth = bitmap_width;
        bm_info.bmiHeader.biHeight = -bitmap_height;
        bm_info.bmiHeader.biBitCount = 32;
        bm_info.bmiHeader.biPlanes = 1;
        bm_info.bmiHeader.biCompression = BI_RGB;

        RECT rect;
        if (!GetClientRect(window, &rect)) abort();
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        t += 25;

        OHRDraw();

        if (!StretchDIBits(hdc, 0, 0, width, height, 0, 0, bitmap_width, bitmap_height, (void *)bitmap_bits, &bm_info, DIB_RGB_COLORS, SRCCOPY)) {
            abort();
        }
    }

    return 0;
}
