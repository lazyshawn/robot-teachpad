cmake_minimum_required(VERSION 3.12)

project(QtTest VERSION 1.0.0)

find_package(ZauxRobot PATHS "C:/Users/15874/CMakeBuilds/417df290-9100-f63e-9ee8-36c4ff5e9300/install")

set(QT5_DIR "D:/Qt/Qt5.12.8/5.12.8/msvc2017_64")
find_package(Qt5 PATHS "${QT5_DIR}/lib/cmake/Qt5" REQUIRED COMPONENTS Core Widgets LinguistTools Concurrent)
#find_package(Qt5 PATHS "${QT5_DIR}/lib/cmake/Qt5" REQUIRED COMPONENTS Core Widgets LinguistTools Concurrent)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms/resource)

# Ui
add_library(qt_ui
  forms/src/fsai_mainWindow.cpp
  forms/src/fsai_procedure.cpp
  forms/src/fsai_advanceConfigWindow.cpp
)
target_link_libraries(qt_ui PUBLIC
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::Concurrent
)
target_include_directories(qt_ui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/forms/src
  ${CMAKE_CURRENT_BINARY_DIR}/qt_ui_autogen/include
)

# FSAI app
add_executable(qt_demo WIN32
	forms/resource/logo.rc
	forms/src/test.cpp

	forms/src/fsai_app.cpp
)
target_link_libraries(qt_demo PUBLIC
  qt_ui
  ZauxRobot::zmotion_interface
)
if (EXISTS "${QT5_DIR}/bin/windeployqt.exe")
    message("windeployqt exists")
    add_custom_command(TARGET qt_demo POST_BUILD
        COMMAND "${QT5_DIR}/bin/windeployqt.exe" "--compiler-runtime"
        "${CMAKE_CURRENT_BINARY_DIR}/qt_demo.exe"
    )
endif()
