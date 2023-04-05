
#ifndef CAPTURE_TEXTURE_H_
#define CAPTURE_TEXTURE_H_

#include "mfutility.h"
#include "dxhelper.h"
#include "texture.h"
#include "dx11base.h"

class CaptureTexture
{
public:
  explicit CaptureTexture();
  ~CaptureTexture();

  static bool createAPI();
  static void destroyAPI();

  bool create(const uint32_t videoDeviceIndex, const uint32_t audioDeviceIndex, const int width, const int height, const int fps);
  void destroy();
  bool update(float dt);

  Render::Texture* getTexture();

private:
  struct InternalData;
  InternalData* m_internalData;
};

#endif // CAPTURE_TEXTURE_H_
