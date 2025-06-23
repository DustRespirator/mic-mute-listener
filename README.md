# Mic mute switch

A lightweight tool to set hotkeys for toggling mute/unmute mic on Windows. Only a simple popup.

## Installation

Download and compile it. Get [Mingw-w64](https://www.mingw-w64.org/) if you do not have a compilation environment.

```bash
g++ MicMuteListener.cpp OSDWindow.cpp ConfigLoader.cpp  -o MicMuteListener.exe -lole32 -luuid -lshlwapi -municode -mwindows
```
If you want to set autorun you can use Schedule Tasks. Copy and save as a .bat:
```cmd
@echo off
set "EXEPATH=%~dp0MicMuteListener.exe"
schtasks /Create /F /TN "MicMuteListener" /TR "\"%EXEPATH%\"" /SC ONLOGON /RL HIGHEST /RU "%USERNAME%"
echo Create Task MicMuteListener
pause
```
and delete the task:
```cmd
@echo off
schtasks /Delete /F /TN "MicMuteListener"
echo Delete Task MicMuteListener
pause
```
## Usage

Create a config.ini, it should looks like below:
```ini
[Hotkeys]
key1=VK_LCONTROL,VK_LMENU,VK_M
key2=VK_F13
key3=
```
It only accepts up to 3 combinations. VK_F24 is a fallback even you do not set it (for example, you run the .exe without a config.ini) because F24 is not exist for common devices so it won't hurt (in most cases). You can remove the fallback or change it to any key you want in the code.

You need the [definition of key codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes) to write your setting. For 0-9 and A-Z, you have to write the code as "VK_1", "VK_A". Not all of the key codes are supported, you may need to modify the key map in the code if needed.

And just run.