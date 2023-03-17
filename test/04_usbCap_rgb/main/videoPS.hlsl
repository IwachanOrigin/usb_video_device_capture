
#include "common.fxh"

Texture2D    txYUV        : register(t0);
SamplerState samLinear    : register(s0);

float4 PS(v2f input) : SV_Target
{
  return txYUV.Sample(samLinear, input.Tex);
}
