#pragma once
#include <windows.h>
#include <filesystem>
#include <string>

bool InitTrayIcon(HINSTANCE hInstance, const std::filesystem::path& path);
void UpdateTrayIcon(bool isMuted);
void RemoveTrayIcon();