#pragma once
#include <windows.h>
#include <vector>
#include <set>

BOOL IsHotkeyPressed(const std::vector<int>& combo, const std::set<int>& pressed);
BOOL ToggleMicMute();
void CheckMicMute();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void SetHotkeyCombos(const std::vector<std::vector<int>>& combos);
const std::vector<std::vector<int>>& GetHotkeyCombos();
bool GetMicMuteState();
bool InitMicVolumeCallback();
void UninitMicVolumeCallback();