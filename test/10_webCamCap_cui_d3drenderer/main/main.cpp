
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
#pragma comment(lib, "evr")
#pragma comment(lib, "strmiids")

// for dx9
#pragma comment(lib, "d3d9")
#pragma comment(lib, "dxva2")

int main(int argc, char* argv[])
{
  // Initialize
  HRESULT hr;
  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    return hr;
  }

  MainWindow::getInstance().setup();
  MainWindow::getInstance().init((HINSTANCE)0);
  MainWindow::getInstance().messageHandle();

  // Release
  hr = MFShutdown();
  if (FAILED(hr))
  {
    return hr;
  }

  return 0;
}
