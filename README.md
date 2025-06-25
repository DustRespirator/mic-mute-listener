# Mic mute switch

A lightweight tool to set hotkeys for toggling mute/unmute mic on Windows. Only a simple popup.

## Installation

Download and compile it. Get [Mingw-w64](https://www.mingw-w64.org/) if you do not have a compilation environment.

```bash
mkdir build
cd build
cmake ..
cmake --build .
cmake --install . --prefix ../dist
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
key2=VK_F24
key3=
```
It only accepts up to 3 combinations. Without config.ini, only click on the tray icon is available.

You need the [definition of key codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes) to write your setting. For 0-9 and A-Z, you have to write the code as "VK_1", "VK_A". Not all of the key codes are supported, you may need to modify the key map in the code if needed.

And just run.