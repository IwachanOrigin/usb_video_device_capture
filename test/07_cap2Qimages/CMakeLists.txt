cmake_minimum_required(VERSION 3.10)

# set the project name
project(cap2QImages CXX)

# output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# output compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -DUNICODE -D_UNICODE)

# 環境変数に以下を追加する
# QTDIR=<Qtのインストールディレクトリ>
# QTDIR=C:\software\Qt\5.13.1\5.13.1\msvc2017_64
# -DQTDIR="D:\software\Qt\5.15.2\5.15.2\msvc2019_64"
#set(QTDIR "C:\\Qt\\Qt5.15.2\\5.15.2\\msvc2019_64")
#set(QTDIR $ENV{QTDIR})
set(CMAKE_PREFIX_PATH ${QTDIR})

# Visual StudioのfilteringをON
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Keep the auto-generated files together in the Visual Studio source tree.
# Because by default it it added to Source Files in the Visual Studio source tree.
# This is very hard to see.
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER AutoGenFiles)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP AutoGenFiles)

# Auto generate moc & ui & rcc files when need.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the Qt libraries for Qt Quick/QML/Widgets/Gui
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick QuickControls2 Qml Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick QuickControls2 Qml Gui Widgets)

# Add files
file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE RC_FILES *.rc)
file(GLOB_RECURSE QRC_FILES *.qrc)

# Filter
source_group("Ui Files" FILES ${UI_FILES})
source_group("Qrc Files" FILES ${QRC_FILES})

add_subdirectory(main)
