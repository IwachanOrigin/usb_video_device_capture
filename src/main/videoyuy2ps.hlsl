
#include "common.fxh"

Texture2D    txYUV        : register(t0);
SamplerState samLinear    : register(s0);

cbuffer Constants : register(b0)
{
  int width;
  int height;
};

float4 PS(v2f input) : SV_Target
{
  // Normalize pixel coordinates to [0, 1] for texture sampling
  float2 uv = input.Pos.xy / float2(width, height);
  float4 pixdata = txYUV.Sample(samLinear, uv);

  // Extract color information from packed pixel data
  uint y0 = (asuint(pixdata.r) & 0xff000000) >> 24;
  uint u  = (asuint(pixdata.r) & 0xff0000) >> 16;
  uint y1 = (asuint(pixdata.r) & 0xff00) >> 8;
  uint v  = asuint(pixdata.r) & 0x000000ff;

  // Check if you are left/right pixel
  uint y = (input.Pos.x % 2 == 0) ? y0 : y1;

  // Convert yuyv to rgb
  float cb = u;
  float cr = v;

  float r = (y + 1.402 * (cr - 128.0));
  float g = (y - 0.344 * (cb - 128.0) - 0.714 * (cr - 128.0));
  float b = (y + 1.772 * (cb - 128.0));

  return float4(r, g, b, 1.f) / 256.0f;
}
