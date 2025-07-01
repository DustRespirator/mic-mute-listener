#include "ConfigManager.h"
#include "ConfigLoader.h"
#include <chrono>
#include <iostream>

static std::filesystem::path configFilePath;
static std::thread watchThread;
static std::atomic<bool> watching = false;
static HANDLE hDir = INVALID_HANDLE_VALUE;
static void (*reloadCallback)(const std::vector<std::vector<int>>&) = nullptr;

void SetConfigReloadCallback(void (*callback)(const std::vector<std::vector<int>>&)) {
    reloadCallback = callback;
}

void ReloadConfig() {
    auto hotkeys = LoadHotkeysFromIni(configFilePath);
    if (reloadCallback) {
        reloadCallback(hotkeys);
    }
}

void StartWatchingConfig(const std::filesystem::path& path) {
    if (watching) return;

    configFilePath = path;
    watching = true;

    watchThread = std::thread([] {
        std::wstring directory = configFilePath.parent_path().wstring();
        std::wstring filename = configFilePath.filename().wstring();

        hDir = CreateFile(
            directory.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr
        );

        if (hDir == INVALID_HANDLE_VALUE) {
            return;
        }

        BYTE buffer[1024];
        DWORD bytesReturned;

        while (watching) {
            if (ReadDirectoryChangesW(
                    hDir,
                    buffer,
                    sizeof(buffer),
                    FALSE,
                    FILE_NOTIFY_CHANGE_LAST_WRITE,
                    &bytesReturned,
                    nullptr,
                    nullptr)) {
                
                FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)buffer;
                do {
                    std::wstring changedFile(fni->FileName, fni->FileNameLength / sizeof(WCHAR));
                    if (changedFile == filename && fni->Action == FILE_ACTION_MODIFIED) {
                        ReloadConfig();
                        break;
                    }
                } while (fni->NextEntryOffset != 0 &&
                         (fni = (FILE_NOTIFY_INFORMATION*)((BYTE*)fni + fni->NextEntryOffset)));
            } else {
                break;
            }
        }

        CloseHandle(hDir);
        hDir = INVALID_HANDLE_VALUE;
    });
}

void StopWatchingConfig() {
    watching = false;
    if (hDir != INVALID_HANDLE_VALUE) {
        CancelIoEx(hDir, nullptr);
    }
    if (watchThread.joinable()) {
        watchThread.join();
    }
}