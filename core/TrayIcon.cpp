#include <windows.h>
#include "TrayIcon.h"
#include <shellapi.h>
#include <gdiplus.h>

using namespace Gdiplus;

extern BOOL ToggleMicMute();

static ULONG_PTR gdiplusToken;
static NOTIFYICONDATA nid = {};
static HICON hIconMuted = nullptr;
static HICON hIconUnmuted = nullptr;
//static HWND trayHwnd = nullptr;
static UINT WM_TASKBARCREATED = RegisterWindowMessage(L"TaskbarCreated");

LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TASKBARCREATED) {
        Shell_NotifyIcon(NIM_ADD, &nid);
        return 0;
    }
    switch (msg) {
        case WM_APP + 1: {
            if (lParam == WM_LBUTTONUP) {
                BOOL isMuted = ToggleMicMute();
                UpdateTrayIcon(isMuted);
            }
            if (lParam == WM_RBUTTONUP) {
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 1, L"Exit");

                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                PostMessage(hwnd, WM_NULL, 0, 0);
                TrackPopupMenuEx(hMenu,
                    TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
                    pt.x, pt.y,
                    hwnd,
                    nullptr);
                DestroyMenu(hMenu);
            }
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                PostQuitMessage(0);
            }
            return 0;
        }
        case WM_DESTROY: {
            RemoveTrayIcon();
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

HICON LoadPngIcon(const std::wstring& path) {
    std::unique_ptr<Bitmap> bitmap(Bitmap::FromFile(path.c_str(), FALSE));
    if (!bitmap || bitmap->GetLastStatus() != Ok) {
        return nullptr;
    }
    HICON hIcon = nullptr;
    bitmap->GetHICON(&hIcon);
    return hIcon;
}

bool InitTrayIcon(HINSTANCE hInstance, const std::filesystem::path& path) {
    GdiplusStartupInput gdiplusStartupInput;
    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Ok) {
        return false;
    }
    // Create hidden window
    WNDCLASS wc = {};
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TrayWindowClass";
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(
        0, L"TrayWindowClass", nullptr, 0,
        0, 0, 0, 0,
        nullptr, nullptr, hInstance, nullptr);

    // Setup tray icon
    hIconMuted = LoadPngIcon(path / "muted.png");
    hIconUnmuted = LoadPngIcon(path / "unmuted.png");
    if (!hIconMuted || !hIconUnmuted) return false;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = hIconUnmuted;
    wcscpy_s(nid.szTip, L"MicToggleSwitch");
    
    //trayHwnd = hwnd;
    return Shell_NotifyIcon(NIM_ADD, &nid);
}

void UpdateTrayIcon(bool isMuted) {
    nid.hIcon = isMuted ? hIconMuted : hIconUnmuted;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (hIconMuted) DestroyIcon(hIconMuted);
    if (hIconUnmuted) DestroyIcon(hIconUnmuted);
    GdiplusShutdown(gdiplusToken);
}