#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <initguid.h>
#include <iostream>
#include "OSDWindow.h"

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
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (p->vkCode == VK_F24) {
            BOOL isMuted = ToggleMicMute();
            ShowOSDText(isMuted ? L"Mic Muted" : L"Mic Unmuted");
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
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