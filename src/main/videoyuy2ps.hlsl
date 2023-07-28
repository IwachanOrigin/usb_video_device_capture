
#include "common.fxh"

Texture2D    txYUV        : register(t0);
SamplerState samLinear    : register(s0);

cbuffer Constants : register(b0)
{
  int width;
  int height;
};

float3 YUVToRGB(float3 yuv)
{
  // BT.601 coefs
  static const float3 yuvCoef_r = { 1.164f, 0.000f, 1.596f };
  static const float3 yuvCoef_g = { 1.164f, -0.392f, -0.813f };
  static const float3 yuvCoef_b = { 1.164f, 2.017f, 0.000f };
  yuv -= float3(0.0625f, 0.5f, 0.5f);
  return saturate(float3( 
    dot( yuv, yuvCoef_r ),
    dot( yuv, yuvCoef_g ),
    dot( yuv, yuvCoef_b )
    ));
}

float4 PS(v2f input) : SV_Target
{
#if 1
  float2 pixel1 = txYUV.Sample(samLinear, input.Tex).rg;
  float2 pixel2 = txYUV.Sample(samLinear, input.Tex + float2(1.0 / width, 0)).rg;

  float y0 = pixel1.r;
  float u0 = pixel1.g;
  float y1 = pixel2.r;
  float v0 = pixel2.g;

  // left or right pixel?
  const float y = uint(input.Tex.x * width) & 1 ? y1 : y0;
  return float4(YUVToRGB(float3(y, u0, v0)), 1.f);
#else
  return txYUV.Sample(samLinear, input.Tex);
#endif
}
