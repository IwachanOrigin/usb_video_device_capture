
#ifndef WIN32_MESSAGE_HANDLER_H_
#define WIN32_MESSAGE_HANDLER_H_

namespace message_handler
{

class Win32MessageHandler
{
public:
  static Win32MessageHandler &getInstance();

  int run(HINSTANCE hinst, int nCmdShow);
  HWND hwnd() { return m_hwnd; }

protected:
  static LRESULT CALLBACK MessageProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
  Win32MessageHandler();
  ~Win32MessageHandler();
  explicit Win32MessageHandler(const Win32MessageHandler &);
  Win32MessageHandler &operator=(const Win32MessageHandler &);

  HWND m_hwnd;
};

} // message_handler

#endif // WIN32_MESSAGE_HANDLER_H_
