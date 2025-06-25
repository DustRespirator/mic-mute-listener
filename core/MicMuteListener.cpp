#include <windows.h>
#include "MicMuteListener.h"
#include "OSDWindow.h"
#include "TrayIcon.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <initguid.h>
#include <shlwapi.h>
#include <iostream>

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
                        ShowOSDText(isMuted ? L"Mic is muted" : L"Mic is unmuted");
                        UpdateTrayIcon(isMuted);
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

void SetHotkeyCombos(const std::vector<std::vector<int>>& combos) {
    hotkeyCombos = combos;
}

const std::vector<std::vector<int>>& GetHotkeyCombos() {
    return hotkeyCombos;
}