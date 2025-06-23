#pragma once
#include <windows.h>
#include <string>
#include <vector>

std::vector<std::vector<int>> LoadHotkeysFromIni(const std::wstring& path);