
set(hlsl_srcs
  common.fxh
  videonv12vs.hlsl
  videonv12ps.hlsl
  videoyuy2vs.hlsl
  videoyuy2ps.hlsl
  videorgb32ps.hlsl
  videorgb32vs.hlsl
)

# options for shader debug mode:  /Zi /Zss /Od /FdColorConvPS.pdb
set_source_files_properties(videonv12vs.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vn g_videoNV12VS /Fh \"${CMAKE_CURRENT_SOURCE_DIR}/videoNV12VS.h\""
  VS_SHADER_TYPE Vertex
  VS_SHADER_MODEL 4.0
  VS_SHADER_ENTRYPOINT VS )
set_source_files_properties(videonv12ps.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vn g_videoNV12PS /Fh \"${CMAKE_CURRENT_SOURCE_DIR}/videoNV12PS.h\""
  VS_SHADER_TYPE Pixel
  VS_SHADER_MODEL 4.0
  VS_SHADER_ENTRYPOINT PS )

set_source_files_properties(videoyuy2vs.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vn g_videoYUY2VS /Fh \"${CMAKE_CURRENT_SOURCE_DIR}/videoYUY2VS.h\""
  VS_SHADER_TYPE Vertex
  VS_SHADER_MODEL 4.0
  VS_SHADER_ENTRYPOINT VS )
set_source_files_properties(videoyuy2ps.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vn g_videoYUY2PS /Fh \"${CMAKE_CURRENT_SOURCE_DIR}/videoYUY2PS.h\""
  VS_SHADER_TYPE Pixel
  VS_SHADER_MODEL 4.0
  VS_SHADER_ENTRYPOINT PS )

set_source_files_properties(videorgb32vs.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vn g_videoRGB32VS /Fh \"${CMAKE_CURRENT_SOURCE_DIR}/videoRGB32VS.h\""
  VS_SHADER_TYPE Vertex
  VS_SHADER_MODEL 4.0
  VS_SHADER_ENTRYPOINT VS )
set_source_files_properties(videorgb32ps.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vng_videoRGB32PS /Fh \"${CMAKE_CURRENT_SOURCE_DIR}/videoRGB32PS.h\""
  VS_SHADER_TYPE Pixel
  VS_SHADER_MODEL 4.0
  VS_SHADER_ENTRYPOINT PS )

add_custom_target(hlsl SOURCES ${hlsl_srcs})

set(main_src
  audiocapturecallback.cpp
  audiocapturecallback.h
  audiocapturemanager.cpp
  audiocapturemanager.h
  audiooutputdevicemanager.cpp
  audiooutputdevicemanager.h
  dx11baserenderer.cpp
  dx11baserenderer.h
  dx11nv12renderer.cpp
  dx11nv12renderer.h
  dx11yuy2renderer.cpp
  dx11yuy2renderer.h
  dx11rgb32renderer.cpp
  dx11rgb32renderer.h
  dxhelper.h
  main.cpp
  mesh.cpp
  mesh.h
  pipeline.cpp
  pipeline.h
  SimpleMath.cpp
  SimpleMath.h
  SimpleMath.inl
  stdafx.cpp
  stdafx.h
  timer.h
  utils.h
  videocapturecallback.cpp
  videocapturecallback.h
  videocaptureformat.h
  videocapturemanager.cpp
  videocapturemanager.h
  win32messagehandler.cpp
  win32messagehandler.h
)

add_executable(${PROJECT_NAME} ${main_src})
add_dependencies(${PROJECT_NAME} hlsl)

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

