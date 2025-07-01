#pragma once
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

class MicVolumeCallback : public IAudioEndpointVolumeCallback {
public:
    MicVolumeCallback();
    ~MicVolumeCallback() = default;

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IAudioEndpointVolumeCallback
    STDMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;

private:
    LONG refCount;
};