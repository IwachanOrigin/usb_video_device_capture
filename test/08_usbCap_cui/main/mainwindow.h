
#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

class MainWindow
{
public:
  static MainWindow& getInstance();

  void init(HINSTANCE hInst);
  void render();
  void messageHandle();

  static LRESULT WINAPI MessageProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
  explicit MainWindow();
  ~MainWindow();

  HWND m_hwnd;
};

#endif // MAIN_WINDOW_H_
