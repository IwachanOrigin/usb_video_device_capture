
#include "MFUtility.h"
#include "mainwindow.h"

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfplay")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "wmcodecdspuuid")

int main(int argc, char* argv[])
{
  MainWindow::getInstance().setup();

  return 0;
}
