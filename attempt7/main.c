#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef float f32;
typedef double f64;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} coord;

typedef struct {
    coord p;
    coord r;
    coord q;
} triangle;

typedef struct {
    f32 min_x;
    f32 min_y;
    f32 max_x;
    f32 max_y;
} bb;

static int bitmap_width;
static int bitmap_height;
static void *bitmap_bits;
static int pixel_num_bytes = 4;
static int bitmap_size;
static triangle t1;

static f64 t;
static f64 PI = 3.1415926535897932385;


coord pixel_to_sc(f32 x, f32 y) {
    coord c = {0};

    c.x = x / (f32)bitmap_width;
    c.x = c.x * 2.f - 1.f;
    c.y = y / (f32)bitmap_height;
    c.y = c.y * 2.f - 1.f;

    c.y *= -1.f;

    return c;
}

coord sc_to_pixel(coord c) {
    coord sc = {0};
    sc.x = (c.x - 1.f) * 2) * (f32)bitmap_width;
    sc.y = (c.x - 1.f) * 2) * (f32)bitmap_height * - 1.f;
    sc.z = 0.f;
}

coord mul_coord(coord c, f32 v) {
    coord nc = {0};
    c.x *= v;
    c.y *= v;
    c.z *= v;
    return c;
}

coord cross(coord u, coord v) {
    coord w = {0};
    w.x = u.y * v.z - v.y * u.z;
    w.x = u.z * v.x - v.z * u.x;
    w.x = u.x * v.y - v.x * u.y;
    return w;
}

f32 vec_len(coord c) {
    return sqrtf(c.x * c.x + c.y * c.y + c.z * c.z);
}

coord barycentric_coords(coord u, coord v, coord w) {
    coord bc = {0};

    bc.x = vec_len(cross(u, w)) * 0.5f;
    bc.y = vec_len(cross(v, w)) * 0.5f;
    bc.z = vec_len(cross(u, v)) * 0.5f;

    return bc;
}

bb bounds(triangle t) {
    bb b = {0};
    b.max_x = -9999.f;
    b.max_y = -9999.f;
    b.min_x = 9999.f;
    b.min_y = 9999.f;

    if (t.p.x < b.min_x) b.min_x = t.p.x;
    if (t.p.x > b.max_x) b.max_x = t.p.x;
    if (t.p.y < b.min_y) b.min_y = t.p.x;
    if (t.p.y > b.max_y) b.max_y = t.p.x;

    if (t.r.x < b.min_x) b.min_x = t.r.x;
    if (t.r.x > b.max_x) b.max_x = t.r.x;
    if (t.r.y < b.min_y) b.min_y = t.r.x;
    if (t.r.y > b.max_y) b.max_y = t.r.x;

    if (t.q.x < b.min_x) b.min_x = t.q.x;
    if (t.q.x > b.max_x) b.max_x = t.q.x;
    if (t.q.y < b.min_y) b.min_y = t.q.x;
    if (t.q.y > b.max_y) b.max_y = t.q.x;

    return b;
}


void OHRDraw() {
    int pitch = bitmap_width * pixel_num_bytes;
    u8 *row = (u8 *)bitmap_bits;
    u8 *pixel = (u8 *)row;

    f64 duration = 100.0;
    f64 mod = fmod(t, duration) / duration;
    mod = sin(mod * 2.0 * PI);
    mod = (mod + 1.0) / 2.0;

    u8 min = (u8)(75.0 * mod);
    u8 color = (u8)(255.0 * mod);

    bb b = bounds(t1);

    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            u8 blue = min;
            u8 green = min;
            u8 red = min;

            coord c = pixel_to_sc((f32)x, (f32)y);

            if (x <= b.max_x
            } else if (c.x <= 0 && c.y > 0) {
                red = color;
            } else if (c.x > 0 && c.y > 0) {
                green = color;
            } else if (c.x <= 0 && c.y <= 0) {
                blue = color;
            } else {
                red = color;
                green = color;
                blue = color;
            }

            *pixel = blue;
            pixel++;

            *pixel = green;
            pixel++;

            *pixel = red;
            pixel++;

            *pixel = min;
            pixel++;

        }
        row += pitch;
    }
}

void OHRPaint(HWND window, HDC hdc) {
    RECT rekt;
    if (!GetClientRect(window, &rekt)) abort();
    int width = rekt.right - rekt.left;
    int height = rekt.bottom - rekt.top;

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
    LPARAM l_param)
{
    switch (msg) {
        case WM_SIZE:
            {
                if (bitmap_bits) {
                    VirtualFree(bitmap_bits, 0, MEM_RELEASE);
                }
                RECT rekt;
                if (!GetClientRect(window, &rekt)) abort();
                int width = rekt.right - rekt.left;
                int height = rekt.bottom - rekt.top;

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
                OHRPaint(window, hdc);
                EndPaint(window, &ps);
                return 0;
            }
    }
    return DefWindowProc(window, msg, w_param, l_param);
}

int WinMain(HINSTANCE app_instance,
            HINSTANCE prev_instance,
            LPSTR cmdline,
            int show_cmd) {
    OutputDebugStringA("OHR!\n");

    const char *class_name = "OHRC";

    WNDCLASSEXA wnex = {0};
    wnex.cbSize = sizeof(wnex);
    wnex.hInstance = app_instance;
    wnex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
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

    t1.p = (coord){ 0.0f, 0.5f, 0.f};
    t1.r = (coord){-0.5f, 0.0f, 0.f};
    t1.q = (coord){ 0.5f, 0.0f, 0.f};

    while (should_run) {
        MSG msg;
        while (PeekMessage(&msg, window, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                should_run = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!should_run) break;
        HDC hdc = GetDC(window);

        OHRPaint(window, hdc);

        t += 0.25;
    }


    return 0;
}
