
#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>
#include "dxhelper.h"
#include "dx11base.h"
#include "mainapp.h"
#include "clock.h"

#include "devicesinfo.h"
#include "devicecommon.h"

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

  MainApp m_app;

  DevicesInfo m_devices;
  DeviceMediaInfo m_dmi;
};

#endif // MAIN_WINDOW_H_
