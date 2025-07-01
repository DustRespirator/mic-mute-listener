#include "MicVolumeCallback.h"
#include "TrayIcon.h"

MicVolumeCallback::MicVolumeCallback()
    : refCount(1) {}

// IUnknown
STDMETHODIMP MicVolumeCallback::QueryInterface(REFIID riid, void** ppvObject) {
    if (riid == IID_IUnknown || riid == __uuidof(IAudioEndpointVolumeCallback)) {
        *ppvObject = static_cast<IAudioEndpointVolumeCallback*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MicVolumeCallback::AddRef() {
    return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) MicVolumeCallback::Release() {
    ULONG count = InterlockedDecrement(&refCount);
    if (count == 0) {
        delete this;
    }
    return count;
}

// IAudioEndpointVolumeCallback
STDMETHODIMP MicVolumeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) {
    if (pNotify) {
        BOOL isMuted = pNotify->bMuted;
        UpdateTrayIcon(isMuted);
    }
    return S_OK;
}