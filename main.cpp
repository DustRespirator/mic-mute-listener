#include <windows.h>
#include "MicMuteListener.h"
#include "ConfigLoader.h"
#include "TrayIcon.h"
#include <filesystem>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    // Get path of config and icons
    std::filesystem::path exePath = std::filesystem::current_path();
    std::filesystem::path configPath = exePath / "config.ini";
    std::filesystem::path iconPath = exePath / "icon";

    // Load config
    SetHotkeyCombos(LoadHotkeysFromIni(configPath));
    // Initialize Core Audio COM interface
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    // Initialize tray icon
    InitTrayIcon(hInstance, iconPath);
    // Set hook
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
    if (!hHook) return 1;
    // Keep thread alive
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    CoUninitialize();
    return 0;
}