
#include "common.fxh"

Texture2D    txY        : register(t0);
Texture2D    txUV        : register(t1);
SamplerState samLinear    : register(s0);


float3 YUVToRGB(float3 yuv)
{
  // BT.601 coefs
  static const float3 yuvCoef_r = { 1.164f, 0.000f, 1.596f };
  static const float3 yuvCoef_g = { 1.164f, -0.392f, -0.813f};
  static const float3 yuvCoef_b = { 1.164f, 2.017f, 0.000f };
  yuv -= float3(0.0625f, 0.5f, 0.5f);
  return saturate(float3( 
    dot( yuv, yuvCoef_r ),
    dot( yuv, yuvCoef_g ),
    dot( yuv, yuvCoef_b )
    ));
  //float r = yuv.r + 1.370705 * (yuv.b - 128);
  //float g = yuv.r - 0.698001 * (yuv.b - 128) - 0.337633 * (yuv.g - 128);
  //float b = yuv.r + 1.732446 * (yuv.g - 128);
  //return saturate(float3(r, g, b));
}

float4 PS(v2f input) : SV_Target
{
  float y = txY.Sample(samLinear, float2(input.Tex.x, input.Tex.y)).r;
  float2 uv = txUV.Sample(samLinear, float2(input.Tex.x, input.Tex.y)).rg;
  float u = uv.r;
  float v = uv.g;
  return float4(YUVToRGB(float3(y,u,v)), 1.f);
}
