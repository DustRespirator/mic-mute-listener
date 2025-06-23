#include <windows.h>
#include "ConfigLoader.h"
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

// Define VK string to virtual key mapping, also the definition of 0-9 and A-Z
static const std::unordered_map<std::wstring, int> vkMap = {
    {L"VK_BACK", VK_BACK},         {L"VK_TAB", VK_TAB},
    {L"VK_RETURN", VK_RETURN},     {L"VK_SHIFT", VK_SHIFT},
    {L"VK_CONTROL", VK_CONTROL},   {L"VK_MENU", VK_MENU},
    {L"VK_PAUSE", VK_PAUSE},       {L"VK_CAPITAL", VK_CAPITAL},
    {L"VK_ESCAPE", VK_ESCAPE},     {L"VK_SPACE", VK_SPACE},
    {L"VK_PRIOR", VK_PRIOR},       {L"VK_NEXT", VK_NEXT},
    {L"VK_END", VK_END},           {L"VK_HOME", VK_HOME},
    {L"VK_LEFT", VK_LEFT},         {L"VK_UP", VK_UP},
    {L"VK_RIGHT", VK_RIGHT},       {L"VK_DOWN", VK_DOWN},
    {L"VK_SNAPSHOT", VK_SNAPSHOT}, {L"VK_INSERT", VK_INSERT},
    {L"VK_DELETE", VK_DELETE},     {L"VK_APPS", VK_APPS},
    {L"VK_LWIN", VK_LWIN},         {L"VK_RWIN", VK_RWIN},
    {L"VK_F1", VK_F1},             {L"VK_F2", VK_F2},
    {L"VK_F3", VK_F3},             {L"VK_F4", VK_F4},
    {L"VK_F5", VK_F5},             {L"VK_F6", VK_F6},
    {L"VK_F7", VK_F7},             {L"VK_F8", VK_F8},
    {L"VK_F9", VK_F9},             {L"VK_F10", VK_F10},
    {L"VK_F11", VK_F11},           {L"VK_F12", VK_F12},
    {L"VK_F13", VK_F13},           {L"VK_F14", VK_F14},
    {L"VK_F15", VK_F15},           {L"VK_F16", VK_F16},
    {L"VK_F17", VK_F17},           {L"VK_F18", VK_F18},
    {L"VK_F19", VK_F19},           {L"VK_F20", VK_F20},
    {L"VK_F21", VK_F21},           {L"VK_F22", VK_F22},
    {L"VK_F23", VK_F23},           {L"VK_F24", VK_F24},
    {L"VK_LSHIFT", VK_LSHIFT},     {L"VK_RSHIFT", VK_RSHIFT},
    {L"VK_LCONTROL", VK_LCONTROL}, {L"VK_RCONTROL", VK_RCONTROL},
    {L"VK_LMENU", VK_LMENU},       {L"VK_RMENU", VK_RMENU},
    {L"VK_0", 0x30},               {L"VK_1", 0x31},
    {L"VK_2", 0x32},               {L"VK_3", 0x33},
    {L"VK_4", 0x34},               {L"VK_5", 0x35},
    {L"VK_6", 0x36},               {L"VK_7", 0x37},
    {L"VK_8", 0x38},               {L"VK_9", 0x39},
    {L"VK_A", 0x41},               {L"VK_B", 0x42},
    {L"VK_C", 0x43},               {L"VK_D", 0x44},
    {L"VK_E", 0x45},               {L"VK_F", 0x46},
    {L"VK_G", 0x47},               {L"VK_H", 0x48},
    {L"VK_I", 0x49},               {L"VK_J", 0x4A},
    {L"VK_K", 0x4B},               {L"VK_L", 0x4C},
    {L"VK_M", 0x4D},               {L"VK_N", 0x4E},
    {L"VK_O", 0x4F},               {L"VK_P", 0x50},
    {L"VK_Q", 0x51},               {L"VK_R", 0x52},
    {L"VK_S", 0x53},               {L"VK_T", 0x54},
    {L"VK_U", 0x55},               {L"VK_V", 0x56},
    {L"VK_W", 0x57},               {L"VK_X", 0x58},
    {L"VK_Y", 0x59},               {L"VK_Z", 0x5A}
    // Add more as needed
};

std::vector<std::vector<int>> LoadHotkeysFromIni(const std::wstring& path) {
    std::vector<std::vector<int>> hotkeys;
    bool valid;
    for (int i = 1; i <= 3; i++) {
        std::wstring keyName = L"key" + std::to_wstring(i);
        wchar_t buffer[256] = {};
        GetPrivateProfileString(L"Hotkeys", keyName.c_str(), nullptr, buffer, 256, path.c_str());
        std::vector<int> combo;
        std::wstringstream ss(buffer);
        std::wstring token;
        valid = true;
        while (std::getline(ss, token, L',')) {
            if (auto it = vkMap.find(token); it != vkMap.end()) {
                combo.push_back(it->second);
            } else {
                valid = false;
                break;
            }
        }
        if (valid && !combo.empty()) {
            hotkeys.push_back(combo);
        } else {
            hotkeys.push_back({VK_F24});
        }
    }
    return hotkeys;
}