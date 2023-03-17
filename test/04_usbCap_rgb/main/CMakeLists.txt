
set(hlsl_srcs
  common.fxh
  videoVS.hlsl
  videoPS.hlsl
)

# options for shader debug mode:  /Zi /Zss /Od /FdColorConvPS.pdb
set_source_files_properties(videoVS.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vng_videoVS /Fh\"${CMAKE_CURRENT_SOURCE_DIR}/videoVS.h\""
  VS_SHADER_TYPE Vertex
  VS_SHADER_MODEL 4.0
  VS_SHADER_ENTRYPOINT VS )
set_source_files_properties(videoPS.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vng_videoPS /Fh\"${CMAKE_CURRENT_SOURCE_DIR}/videoPS.h\""
  VS_SHADER_TYPE Pixel
  VS_SHADER_MODEL 4.0
  VS_SHADER_ENTRYPOINT PS )

add_custom_target(hlsl SOURCES ${hlsl_srcs})

set(main_src
  capture_texture.cpp
  capture_texture.h
  clock.h
  dx11base.cpp
  dx11base.h
  dxhelper.h
  main.cpp
  mainapp.cpp
  mainapp.h
  mesh.cpp
  mesh.h
  mfutility.h
  pipeline.cpp
  pipeline.h
  SimpleMath.cpp
  SimpleMath.h
  stdafx.cpp
  stdafx.h
  texture.cpp
  texture.h
  timer.h
)

add_executable(${PROJECT_NAME} WIN32 ${main_src})
add_dependencies(${PROJECT_NAME} hlsl)
