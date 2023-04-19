
#ifndef CAPTURE_RENDERER_H_
#define CAPTURE_RENDERER_H_

#include <windows.h>
#include <windowsx.h>

#include <wrl/client.h>

#include <mfapi.h>
#include <mferror.h>
#include <mfobjects.h>
#include <mfplay.h>
#include <mfreadwrite.h>

using namespace Microsoft::WRL;

namespace Renderer
{

class CaptureRenderer
{
public:
  explicit CaptureRenderer();
  ~CaptureRenderer() = default;

  bool create(HWND hwnd, int deviceNo, int width, int height, int fps, const GUID subtype);
  void render();
  void destroy();

private:
  ComPtr<IMFSample> m_pVideoSample;
  ComPtr<IMFMediaBuffer> m_pDstBuffer;
  ComPtr<IMF2DBuffer> m_p2DBuffer;
  ComPtr<IMFSourceReader> m_pVideoReader;
};

} // Renderer

#endif // CAPTURE_RENDERER_H_
