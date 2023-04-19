
set(main_src
  devicecommon.h
  devicesinfo.h
  devicesinfo.cpp
  main.cpp
  mainwindow.h
  mainwindow.cpp
)

add_executable(${PROJECT_NAME} ${main_src})
add_dependencies(${PROJECT_NAME} hlsl)
