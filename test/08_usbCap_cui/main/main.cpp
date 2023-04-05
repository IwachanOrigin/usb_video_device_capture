
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
  // Initialize
  HRESULT hr;
  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    return hr;
  }

  MainWindow::getInstance().setup();

  // Release
  hr = MFShutdown();
  if (FAILED(hr))
  {
    return hr;
  }

  return 0;
}
