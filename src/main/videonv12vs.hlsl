
#include "common.fxh"

v2f VS(VS_INPUT input)
{
  v2f output;
  output.Pos = input.Pos;
#if 1
  output.Tex = input.Tex;
#else
  output.Tex.x = input.Tex.x * ((textureWidth - 400.0f) / textureWidth);
  output.Tex.y = input.Tex.y * ((textureHeight - 400.0f) / textureHeight);
  output.Tex.x += 200.0f / textureWidth;
  output.Tex.y += 200.0f / textureHeight;
#endif
  return output;
}

