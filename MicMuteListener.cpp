#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <initguid.h>
#include <shlwapi.h>
#include <iostream>
#include <vector>
#include <set>
#include "OSDWindow.h"
#include "ConfigLoader.h"

static std::vector<std::vector<int>> hotkeyCombos;
static std::set<int> pressedKeys;

// Check if the hotkeys are pressed
BOOL IsHotkeyPressed(const std::vector<int>& combo, const std::set<int>& pressed) {
    for (int key : combo) {
        if (pressed.find(key) == pressed.end()) {
            return false;
        }
    }
    return true;
}

// Toggle mute/unmute
BOOL ToggleMicMute() {
    HRESULT hr;
    BOOL bMute;

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioEndpointVolume* pVolume = nullptr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);

    if (SUCCEEDED(hr)) {
        hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, &pDevice);
        if (SUCCEEDED(hr)) {
            hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&pVolume);
            if (SUCCEEDED(hr)) {
                pVolume->GetMute(&bMute);
                pVolume->SetMute(!bMute, nullptr);
                pVolume->Release();
            }
            pDevice->Release();
        }
        pEnumerator->Release();
    }
    return !bMute;
}

// Callback function
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            pressedKeys.insert(p->vkCode);
            for (const auto& combo : hotkeyCombos) {
                if (IsHotkeyPressed(combo, pressedKeys)) {
                    static DWORD triggerTime = 0;
                    DWORD now = GetTickCount();
                    if (now - triggerTime > 200) {
                        triggerTime = now;
                        BOOL isMuted = ToggleMicMute();
                        ShowOSDText(isMuted ? L"Mic Muted" : L"Mic Unmuted");
                    }
                    break;
                }
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            pressedKeys.erase(p->vkCode);
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    // Load config
    wchar_t exePath[MAX_PATH];
    GetModuleFileName(nullptr, exePath, MAX_PATH);
    PathRemoveFileSpec(exePath);
    std::wstring configPath = std::wstring(exePath) + L"\\config.ini";
    hotkeyCombos = LoadHotkeysFromIni(configPath);
    // Initialize Core Audio COM interface
    CoInitialize(nullptr);
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