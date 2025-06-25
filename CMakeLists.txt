cmake_minimum_required(VERSION 3.14)
project(MicMuteSwitch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(core)

add_definitions(-DUNICODE -D_UNICODE)

add_executable(MicMuteSwitch
    main.cpp
    core/ConfigLoader.cpp
    core/MicMuteListener.cpp
    core/OSDWindow.cpp
    core/TrayIcon.cpp
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
install(DIRECTORY icon/ DESTINATION icon)

target_link_options(MicMuteSwitch PRIVATE -municode -mwindows)