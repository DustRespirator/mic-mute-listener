cmake_minimum_required(VERSION 3.14)
project(MicMuteSwitch)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_definitions(-DUNICODE -D_UNICODE)

add_executable(MicMuteSwitch
    src/main.cpp
    src/ConfigManager.cpp
    src/ConfigLoader.cpp
    src/MicMuteListener.cpp
    src/MicVolumeCallback.cpp
    src/OSDWindow.cpp
    src/TrayIcon.cpp
    resources.rc
)

target_link_libraries(MicMuteSwitch
    gdiplus
    gdi32
    ole32
    uuid
    shlwapi
    shell32
    user32
)

install(TARGETS MicMuteSwitch DESTINATION .)
install(FILES config.ini DESTINATION .)
install(DIRECTORY assets/icon/ DESTINATION icon)

target_link_options(MicMuteSwitch PRIVATE -municode -mwindows)