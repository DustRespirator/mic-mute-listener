#include "MicMuteListener.h"
#include "MicVolumeCallback.h"
#include "OSDWindow.h"
#include "TrayIcon.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <initguid.h>
#include <shlwapi.h>
#include <iostream>

static std::vector<std::vector<int>> hotkeyCombos;
static std::set<int> pressedKeys;
static BOOL micMuteState = FALSE;
static IAudioEndpointVolume* g_pEndpointVolume = nullptr;
static MicVolumeCallback* g_pVolumeCallback = nullptr;

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
    if (!g_pEndpointVolume) return FALSE;

    BOOL bMute;
    g_pEndpointVolume->GetMute(&bMute);
    if (SUCCEEDED(g_pEndpointVolume->SetMute(!bMute, nullptr))) {
        micMuteState = !bMute;
    }
    return micMuteState;
}

// Check mute/unmute state only, for initializing icon
void CheckMicMute() {
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
                micMuteState = bMute;
                pVolume->Release();
            }
            pDevice->Release();
        }
        pEnumerator->Release();
    }
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
                        ToggleMicMute();
                        ShowOSDText(micMuteState ? L"Mic is muted" : L"Mic is unmuted");
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

bool GetMicMuteState() {
    if (g_pEndpointVolume) {
        BOOL bMute;
        if (SUCCEEDED(g_pEndpointVolume->GetMute(&bMute))) {
            micMuteState = bMute;
        }
    }
    return micMuteState;
}

bool InitMicVolumeCallback() {
    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) return false;

    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, &pDevice);
    pEnumerator->Release();
    if (FAILED(hr)) {
        return false;
    }

    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&g_pEndpointVolume);
    pDevice->Release();
    if (FAILED(hr)) {
        return false;
    }

    g_pVolumeCallback = new MicVolumeCallback();
    hr = g_pEndpointVolume->RegisterControlChangeNotify(g_pVolumeCallback);
    if (FAILED(hr)) {
        g_pVolumeCallback->Release();
        g_pVolumeCallback = nullptr;
        g_pEndpointVolume->Release();
        g_pEndpointVolume = nullptr;
        return false;
    }

    BOOL bMute = FALSE;
    g_pEndpointVolume->GetMute(&bMute);
    micMuteState = bMute;
    return true;
}

void UninitMicVolumeCallback() {
    if (g_pEndpointVolume && g_pVolumeCallback) {
        g_pEndpointVolume->UnregisterControlChangeNotify(g_pVolumeCallback);
    }
    if (g_pVolumeCallback) {
        g_pVolumeCallback->Release();
        g_pVolumeCallback = nullptr;
    }
    if (g_pEndpointVolume) {
        g_pEndpointVolume->Release();
        g_pEndpointVolume = nullptr;
    }
}