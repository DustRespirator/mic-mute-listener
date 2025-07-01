#include <windows.h>
#include "MicMuteListener.h"
#include "ConfigLoader.h"
#include "ConfigManager.h"
#include "TrayIcon.h"
#include <filesystem>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    // Get path of config and icons
    wchar_t exePath[MAX_PATH];
    GetModuleFileName(nullptr, exePath, MAX_PATH);
    std::filesystem::path directory = std::filesystem::path(exePath).parent_path();
    std::filesystem::path configPath = directory / "config.ini";
    std::filesystem::path iconPath = directory / "icon";

    // Load config
    SetHotkeyCombos(LoadHotkeysFromIni(configPath));
    // Initialize watching config file path
    StartWatchingConfig(configPath);
    SetConfigReloadCallback(SetHotkeyCombos);
    // Initialize Core Audio COM interface
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    CheckMicMute();
    // Register volume change callback
    InitMicVolumeCallback();
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

    UninitMicVolumeCallback();
    StopWatchingConfig();
    UnhookWindowsHookEx(hHook);
    CoUninitialize();
    return 0;
}