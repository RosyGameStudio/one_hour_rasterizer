#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef float f32;
typedef double f64;



static boolean should_run = true;
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
} pixel_coord;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} coord_3d;

static pixel_coord P = {  0.0f,  0.5f };
static pixel_coord Q = { -0.5f, -0.5f };
static pixel_coord R = {  0.0f, -0.5f };

pixel_coord pixel_to_sc(f32 x, f32 y) {
    pixel_coord c = {0};
    c.x = (x/bitmap_width) * 2.f - 1.f;
    c.y = (y/bitmap_height) * 2.f - 1.f;
    c.y *= -1;
    return c;
}

coord_3d cross(coord_3d u, coord_3d v) {
    coord_3d w = {0};
    w.x = u.y * v.z - v.y * u.z;
    w.y = u.z * v.x - v.z * u.x;
    w.z = u.x * v.y - v.x * u.y;
    return w;
}

/*
coord_3d mul(coord_3d u, f32 a) {
}

coord_3d bayrcentric_coord(coord_3d u, coord_3d v, coord_3d w) {
    coord_3d bc = {0};
    bc.x = 0.5f * cross(u, w);
    bc.y = 0.5f * cross(u, w);
    bc.z = 0.5f * cross(u, w);
    return bc;
}
*/

void OHRDraw() {
    int pitch = bitmap_width * pixel_num_bytes;
    u8 *row = (u8 *)bitmap_bits;
    u8 *pixel = (u8 *)row;

    f64 duration = 100.0;
    f64 mod = fmod(t, duration) / duration; // 0 - 1
    mod = sin(mod * 2 * PI); // -1 to 1
    mod = (mod + 1) / 2;

    u8 min = (u8)(75.0 * mod);
    u8 color =  (u8)(255.0 * mod);

    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            pixel_coord c = pixel_to_sc((f32)x, (f32) y);
            if (c.x <= 0.f && c.y > 0.f) {
                *pixel = min;
                pixel++;

                *pixel = min;
                pixel++;

                *pixel = color;
                pixel++;

                *pixel = 0;
                pixel++;
            } else if (c.x > 0.f && c.y > 0.f) {
                *pixel = min;
                pixel++;

                *pixel = color;
                pixel++;

                *pixel = min;
                pixel++;

                *pixel = 0;
                pixel++;
            } else if (c.x <= 0.f && c.y <= 0.f) {
                *pixel = color;
                pixel++;

                *pixel = min;
                pixel++;

                *pixel = min;
                pixel++;

                *pixel = 0;
                pixel++;
            } else {
                *pixel = color;
                pixel++;

                *pixel = color;
                pixel++;

                *pixel = color;
                pixel++;

                *pixel = 0;
                pixel++;
            }
        }
        row += pitch;
    }
}

void OHRPaint(HWND window, HDC hdc) {
    // do stuff
    RECT rect;
    if (!GetClientRect(window, &rect)) abort();
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    OHRDraw();

    BITMAPINFO bm_info = {0};
    bm_info.bmiHeader.biSize = sizeof(bm_info.bmiHeader);
    bm_info.bmiHeader.biWidth = bitmap_width;
    bm_info.bmiHeader.biHeight = -bitmap_height;
    bm_info.bmiHeader.biPlanes = 1;
    bm_info.bmiHeader.biBitCount = 32;
    bm_info.bmiHeader.biCompression = BI_RGB;

    if (!StretchDIBits(hdc, 0, 0, width, height, 0, 0, bitmap_width, bitmap_height, bitmap_bits, &bm_info, DIB_RGB_COLORS, SRCCOPY)) {
        abort();
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
                bitmap_bits = VirtualAlloc(0,  bitmap_size, MEM_COMMIT, PAGE_READWRITE);
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

                OHRPaint(window, hdc);

                EndPaint(window, &ps);
                return 0;
            }
    }
    return DefWindowProcA(window, msg, w_param, l_param);
}

int WinMain(
    HINSTANCE app_instance,
    HINSTANCE prev_instance,
    LPSTR cmd_line,
    int show_cmd) {
    OutputDebugStringA("OHR!");

    const char *class_name = "OHRCN";

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


    while (should_run) {
        MSG msg;
        while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                should_run = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!should_run) break;
        HDC hdc = GetDC(window);
        OHRPaint(window, hdc);
        t += 0.25;
    }

    return 0;
}
