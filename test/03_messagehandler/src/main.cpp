
#include "stdafx.h"
#include "win32messagehandler.h"

using namespace message_handler;

int main(int argc, char* argv[])
{
  Win32MessageHandler::getInstance().run((HINSTANCE)0, 1);
  return 0;
}
