cmake_minimum_required(VERSION 3.16.0)

project(inputactions_touchpad_visualizer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DQT_NO_DEBUG_OUTPUT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Bsymbolic-functions -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-missing-field-initializers -Wno-switch")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt6 CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(PkgConfig REQUIRED)
pkg_search_module(LIBEVDEV REQUIRED libevdev)

add_executable(inputactions-touchpad-visualizer
    src/main.cpp
    src/MainWindow.cpp
    src/Touchpad.h
    src/TouchpadHandler.cpp
)
target_link_libraries(inputactions-touchpad-visualizer PRIVATE
    Qt6::Core
    Qt6::Widgets
    ${LIBEVDEV_LIBRARIES}
)
target_include_directories(inputactions-touchpad-visualizer PUBLIC
    ${LIBEVDEV_INCLUDE_DIRS}
)
install(TARGETS inputactions-touchpad-visualizer)