
#include "mainwindow.h"

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

void MainWindow::init(HINSTANCE hInst)
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
		wcex.lpszClassName = "testClass";
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

  ShowWindow(m_hwnd, SW_SHOW);
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
}


