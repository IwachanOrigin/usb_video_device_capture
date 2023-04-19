
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

  RECT windowRect = { 0, 0, static_cast<LONG>(800), static_cast<LONG>(600) };
  AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

  m_hwnd = CreateWindowEx(
    WS_EX_OVERLAPPEDWINDOW
		, (TCHAR*)windowClass
		, nullptr
		, WS_OVERLAPPEDWINDOW | WS_VISIBLE
    , CW_USEDEFAULT
    , CW_USEDEFAULT
    , windowRect.right - windowRect.left
    , windowRect.bottom - windowRect.top
		, NULL
		, NULL
		, hInst
		, NULL);

  if (!m_hwnd)
  {
    return false;
  }

  ShowWindow(m_hwnd, SW_SHOW);

  return true;
}

void MainWindow::messageHandle()
{

  MSG msg{};
  while (msg.message != WM_QUIT)
  {
    // Process any messages in the queue.
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      this->render();
    }
  }
}

void MainWindow::render()
{
}

