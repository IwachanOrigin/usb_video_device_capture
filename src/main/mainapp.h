
#ifndef MAIN_APP_H_
#define MAIN_APP_H_

#include "stdafx.h"
#include "pipeline.h"
#include "mesh.h"
#include "capture_texture.h"

using namespace Render;

class MainApp
{
public:
  MainApp();
  ~MainApp();

  bool create(HWND hWnd);
  void destroy();
  void update(float dt);
  void render();

private:
  Pipeline m_pipeVideo;
  Mesh m_quad;
  CaptureTexture m_captureTexture;
};

#endif // MAIN_APP_H_
