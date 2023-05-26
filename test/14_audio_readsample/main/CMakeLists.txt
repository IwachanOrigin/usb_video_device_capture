
set(main_src
  capturemanager.cpp
  capturemanager.h
  main.cpp
  stdafx.cpp
  stdafx.h
  timer.h
  utils.h
  audiocapturecallback.cpp
  audiocapturecallback.h
  win32messagehandler.cpp
  win32messagehandler.h
)

add_executable(${PROJECT_NAME} ${main_src})
