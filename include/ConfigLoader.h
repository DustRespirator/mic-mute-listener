#pragma once
#include <windows.h>
#include <filesystem>
#include <string>
#include <vector>

std::vector<std::vector<int>> LoadHotkeysFromIni(const std::filesystem::path& path);