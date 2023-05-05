
cmake_minimum_required(VERSION 3.10)

# set the project name
project(usbVideoDeviceCapture CXX)

# output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# output compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -DUNICODE -D_UNICODE)

# Visual StudioのfilteringをON
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Keep the auto-generated files together in the Visual Studio source tree.
# Because by default it it added to Source Files in the Visual Studio source tree.
# This is very hard to see.
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER AutoGenFiles)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP AutoGenFiles)

# add external lib/headers
set(SDL2_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets/lib/sdl2)
set(SDL2_PATH_LIB ${SDL2_PATH}/lib/x64)
set(SDL2_PATH_HEADERS ${SDL2_PATH}/include)
set(libsdl2 ${SDL2_PATH_LIB}/SDL2.lib)
include_directories(${SDL2_PATH_HEADERS})
link_directories(${SDL2_PATH_LIB})

# Set up the copy dll list
set(EXTERNAL_DLLS ${SDL2_PATH_LIB}/SDL2.dll)

add_subdirectory(main)
