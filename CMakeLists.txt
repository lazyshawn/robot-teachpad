cmake_minimum_required(VERSION 3.12)

project(QtTest VERSION 1.0.0)

find_package(Qt5 PATHS "D:/Qt/Qt5.12.8/5.12.8/msvc2017_64/lib/cmake/Qt5" COMPONENTS Core Widgets LinguistTools REQUIRED)
find_package(ZauxRobot PATHS "C:/Users/15874/CMakeBuilds/417df290-9100-f63e-9ee8-36c4ff5e9300/install")

# Ui
add_library(qt_ui
  forms/src/fsai_mainWindow.cpp
  forms/temp/fsai_mainWindow_moc.cpp
)
target_link_libraries(qt_ui PUBLIC
  Qt5::Core
  Qt5::Widgets
)
target_include_directories(qt_ui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/forms/src
  ${CMAKE_CURRENT_SOURCE_DIR}/forms/temp
)

# FSAI app
add_executable(qt_demo #WIN32
	forms/resource/logo.rc
	forms/src/test.cpp

	forms/src/fsai_app.cpp
	forms/temp/fsai_app_moc.cpp
)

target_link_libraries(qt_demo PUBLIC
  qt_ui
  ZauxRobot::zmotion_interface
)

