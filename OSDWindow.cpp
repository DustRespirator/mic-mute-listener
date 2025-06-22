#define OSD_WIDTH  240
#define OSD_HEIGHT 100
#define OSD_DURATION 1000
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include "OSDWindow.h"
#include <thread>

LRESULT CALLBACK OSDWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static std::wstring displayText;

    switch (msg) {
        case WM_CREATE: {
            displayText = *(std::wstring*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
            HDC hdc = GetDC(hwnd);
            HFONT hFont = CreateFontW(
                -MulDiv(14, GetDeviceCaps(hdc, LOGPIXELSY), 72),
                0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SWISS,
                L"Segoe UI"
            );
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hFont);
            SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
            ReleaseDC(hwnd, hdc);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            HFONT hFont = (HFONT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            SelectObject(hdc, hFont);
            GetClientRect(hwnd, &rc);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawText(hdc, displayText.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY: {
            HFONT hFont = (HFONT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (hFont) {
                DeleteObject(hFont);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            }
            return 0;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

void ShowOSDText(const std::wstring &text) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    //wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = OSDWindowProc;
    //wc.cbClsExtra = 0;
    //wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(nullptr);
    //wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
    //wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"OSDWindowClass";
    //wc.hIconSm = nullptr;


    UINT dpi = GetDpiForSystem();  // 100 -> 96dpi, 125 -> 120dpi, 150 -> 144dpi
    float scale = dpi / 96.0f;
    int width = int(OSD_WIDTH * scale);
    int height = int(OSD_HEIGHT * scale);
    RegisterClassEx(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - width) / 2;
    int y = screenHeight * 5 / 6;

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
        wc.lpszClassName,
        nullptr,
        WS_POPUP,
        x, y, width, height,
        nullptr, nullptr, wc.hInstance, (LPVOID)&text);

    if (hwnd) {
        // Initialize in CreateWindowEx()
        //LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        //SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
        SetLayeredWindowAttributes(hwnd, 0, 220, LWA_ALPHA);
        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
        UpdateWindow(hwnd);

        std::thread([hwnd]() {
            // Fade in
            for (int alpha = 0; alpha <= 220; alpha += 20) {
                SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }
            // 1s
            std::this_thread::sleep_for(std::chrono::milliseconds(OSD_DURATION));
            // Fade out
            for (int alpha = 220; alpha >= 0; alpha -= 20) {
                SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }
            DestroyWindow(hwnd);
        }).detach();

        std::thread([hwnd]() {
            MSG msg;
            while (GetMessage(&msg, nullptr, 0, 0)) {
                if (!IsWindow(hwnd)) break;
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }).detach();
    }
}