#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <filesystem>
#include <atomic>

void SetConfigReloadCallback(void (*callback)(const std::vector<std::vector<int>>&));
void ReloadConfig();
void StartWatchingConfig(const std::filesystem::path& path);
void StopWatchingConfig();