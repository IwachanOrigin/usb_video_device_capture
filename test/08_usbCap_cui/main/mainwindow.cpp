
#include "mainwindow.h"
#include <iostream>

LRESULT WINAPI MainWindow::MessageProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
  case WM_DESTROY:
    //MainWindow::getInstance().
    PostQuitMessage(0);
    break;
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}

MainWindow::MainWindow()
  : m_width(0)
  , m_height(0)
  , m_fps(0)
  , m_currentDeviceIndex(0)
{
}

MainWindow::~MainWindow()
{
  m_app.destroy();
}

MainWindow& MainWindow::getInstance()
{
  static MainWindow inst;
  return inst;
}

bool MainWindow::setup()
{
  // step1, input device no.
  m_devices.writeDeviceNameList();
  std::wcout << "Please Input Your USB Camera Device Index Number : ";
  std::string input = "";
  std::getline(std::cin, input);
  m_currentDeviceIndex = std::stoi(input);
  m_devices.setCurrentVideoDeviceIndex(m_currentDeviceIndex);

  // step2, input format no.
  m_devices.writeDeviceMediaInfoList();
  std::wcout << "Please Input Your USB Camera Device Index Number : ";
  input = "";
  std::getline(std::cin, input);
  int fmtindex = std::stoi(input);
  m_devices.setCurrentVideoFormatIndex(fmtindex);
  m_devices.getVideoDeviceMediaInfo(fmtindex, m_dmi);
  m_width = m_dmi.width;
  m_height = m_dmi.height;
  m_fps = m_dmi.frameRateNumerator;

  return true;
}

bool MainWindow::init(HINSTANCE hInst)
{
  static ATOM windowClass = 0;
  if (!windowClass)
  {
    WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = MainWindow::MessageProcedure;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = L"testClass";
		wcex.hIconSm = NULL;

		windowClass = RegisterClassEx(&wcex);
  }

  m_hwnd = CreateWindowEx(
    WS_EX_OVERLAPPEDWINDOW
		, (TCHAR*)windowClass
		, nullptr
		, WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE
		, CW_USEDEFAULT
		, 0
		, CW_USEDEFAULT
		, 0
		, NULL
		, NULL
		, hInst
		, NULL);

  if (!m_hwnd)
  {
    return false;
  }

  if (!m_app.create(m_hwnd, m_currentDeviceIndex, m_width, m_height, m_fps))
  {
    return false;
  }

  ShowWindow(m_hwnd, SW_SHOW);

  return true;
}

void MainWindow::messageHandle()
{
  MSG message{};
  {
    if (!GetMessage(&message, nullptr, 0, 0))
    {
      return;
    }
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
}

void MainWindow::render()
{
  Clock clock;
  float elapsed = clock.elapsed();
  m_app.update(elapsed);
  m_app.render();
}

