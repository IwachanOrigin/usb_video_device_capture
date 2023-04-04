
set(main_src
  devicesinfo.cpp
  devicesinfo.h
  main.cpp
  mainwindow.cpp
  mainwindow.h
  MFUtility.h
)

add_executable(
  ${PROJECT_NAME}
  WIN32
  ${main_src}
  ${UI_FILES}
  ${RC_FILES}
  ${QRC_FILES}
)

# Link
target_link_libraries(
  ${PROJECT_NAME}
  Qt${QT_VERSION_MAJOR}::Widgets
)

##################################################################################################################
# COPY DLL
##################################################################################################################

get_target_property(_qmake_path Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_path ${_qmake_path} DIRECTORY)

find_program(DEPLOYQT NAMES windeployqt HINTS "${_qt_bin_path}")

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${DEPLOYQT} $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>
  --$<IF:$<CONFIG:Debug>,debug,release> --dir "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
install(CODE "execute_process(COMMAND ${DEPLOYQT} ${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:${PROJECT_NAME}>
                              --$<IF:$<CONFIG:Debug>,debug,release> --qmldir ${CMAKE_SOURCE_DIR})")

