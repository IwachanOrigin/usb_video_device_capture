
set(main_src
  audiooutputdevicemanager.h
  audiooutputdevicemanager.cpp
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

# Copy dlls
foreach(DLL IN LISTS EXTERNAL_DLLS)
  # Get the file name of the DLL
  get_filename_component(DLL_FILENAME "${DLL}" NAME)

  # Command to copy the DLL file
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${DLL}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/${DLL_FILENAME}"
    COMMENT "Copying external DLL to output directory"
  )
endforeach()

