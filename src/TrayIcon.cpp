#include "ConfigManager.h"
#include "MicMuteListener.h"
#include "TrayIcon.h"
#include <shellapi.h>
#include <gdiplus.h>

using namespace Gdiplus;

static ULONG_PTR gdiplusToken;
static NOTIFYICONDATA nid = {};
static HICON hIconMuted = nullptr;
static HICON hIconUnmuted = nullptr;
static HICON hIconMutedLight = nullptr;
static HICON hIconUnmutedLight = nullptr;
static HICON hIconMutedDark = nullptr;
static HICON hIconUnmutedDark = nullptr;
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
                AppendMenu(hMenu, MF_STRING, 101, L"Open Config File");
                AppendMenu(hMenu, MF_STRING, 102, L"Reload Config");
                AppendMenu(hMenu, MF_STRING, 0, nullptr);
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
        case WM_SETTINGCHANGE: {
            if (lParam && wcscmp((LPCWSTR)lParam, L"ImmersiveColorSet") == 0) {
                ApplyThemeIcons(IsDarkTheme());
            }
            return 0;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 101: {
                    ShellExecute(nullptr, L"open", L"config.ini", nullptr, nullptr, SW_SHOW);
                    break;
                }
                case 102: {
                    ReloadConfig();
                    break;
                }
                case 1: {
                    PostQuitMessage(0);
                    break;
                }
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
    hIconMutedLight = LoadPngIcon(path / "muted_light.png");
    hIconUnmutedLight = LoadPngIcon(path / "unmuted_light.png");
    hIconMutedDark = LoadPngIcon(path / "muted_dark.png");
    hIconUnmutedDark = LoadPngIcon(path / "unmuted_dark.png");
    if (!hIconMutedLight || !hIconUnmutedLight || !hIconMutedDark || !hIconUnmutedDark) return false;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    if (IsDarkTheme()) {
        hIconMuted = hIconMutedDark;
        hIconUnmuted = hIconUnmutedDark;
    } else {
        hIconMuted = hIconMutedLight;
        hIconUnmuted = hIconUnmutedLight;
    }
    nid.hIcon = GetMicMuteState() ? hIconMuted : hIconUnmuted;
    wcscpy_s(nid.szTip, L"MicToggleSwitch");
    
    //trayHwnd = hwnd;
    int retry = 20;
    while (!IsTaskbarReady()) {
        Sleep(200);
        retry--;
    }
    return Shell_NotifyIcon(NIM_ADD, &nid);
}

void UpdateTrayIcon(bool isMuted) {
    nid.hIcon = isMuted ? hIconMuted : hIconUnmuted;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (hIconMutedLight) DestroyIcon(hIconMutedLight);
    if (hIconUnmutedLight) DestroyIcon(hIconUnmutedLight);
    if (hIconMutedDark) DestroyIcon(hIconMutedDark);
    if (hIconUnmutedDark) DestroyIcon(hIconUnmutedDark);
    if (hIconMuted) DestroyIcon(hIconMuted);
    if (hIconUnmuted) DestroyIcon(hIconUnmuted);
    GdiplusShutdown(gdiplusToken);
}

void ApplyThemeIcons(bool isDarkTheme) {
    hIconMuted = isDarkTheme ? hIconMutedDark : hIconMutedLight;
    hIconUnmuted = isDarkTheme ? hIconUnmutedDark : hIconUnmutedLight;
    UpdateTrayIcon(GetMicMuteState());
}

bool IsDarkTheme() {
    DWORD value = 1;
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD size = sizeof(DWORD);
        RegQueryValueEx(hKey, L"SystemUsesLightTheme", nullptr, nullptr, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }
    return value == 0; // 1 == light, 0 == dark
}

bool IsTaskbarReady() {
    HWND hTray = FindWindow(L"Shell_TrayWnd", nullptr);
    return hTray != nullptr && IsWindowVisible(hTray);
}