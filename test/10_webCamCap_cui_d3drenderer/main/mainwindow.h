
#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include "capturerenderer.h"
#include "devicesinfo.h"
#include "devicecommon.h"

using namespace Renderer;

class MainWindow
{
public:
  static MainWindow& getInstance();

  bool setup();
  bool init(HINSTANCE hInst);
  void render();
  void messageHandle();
  void setDeviceIndex(const int index)
  {
    m_devices.setCurrentVideoDeviceIndex(index);
  }
  void setDeviceFormatIndex(const int index)
  {
    m_devices.setCurrentVideoFormatIndex(index);
  }

  static LRESULT WINAPI MessageProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
  explicit MainWindow();
  ~MainWindow();

  HWND m_hwnd;
  int m_currentDeviceIndex;
  int m_width;
  int m_height;
  int m_fps;

  DevicesInfo m_devices;
  DeviceMediaInfo m_dmi;
  CaptureRenderer m_captureRenderer;
};

#endif // MAIN_WINDOW_H_
