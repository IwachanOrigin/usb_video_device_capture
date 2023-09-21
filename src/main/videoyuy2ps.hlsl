
#include "common.fxh"

// Assuming you have declared texture and sampler as globals:
Texture2D    txYUV        : register(t0);
SamplerState samLinear    : register(s0);

cbuffer Constants : register(b0)
{
  int width;
  int height;
};


float4 rec709YCbCr2rgba(float Y, float Cb, float Cr, float a)
{
  float r, g, b; 

  // Y: Undo 1/256 texture value scaling and scale [16..235] to [0..1] range
  // C: Undo 1/256 texture value scaling and scale [16..240] to [-0.5 .. + 0.5] range
  Y = (Y * 256.0f - 16.0f) / 219.0f; 
  Cb = (Cb * 256.0f - 16.0f) / 224.0f - 0.5f; 
  Cr = (Cr * 256.0f - 16.0f) / 224.0f - 0.5f; 

  // Convert to RGB using Rec.709 conversion matrix (see eq 26.7 in Poynton 2003)
  r = Y + 1.5748f * Cr; 
  g = Y - 0.1873f * Cb - 0.4681f * Cr; 
  b = Y + 1.8556f * Cb; 

  return float4(b, g, r, a); 
}

// Perform bilinear interpolation between the provided components.
// The samples are expected as shown:
// ---------
// | X | Y |
// |---+---|
// | W | Z |
// ---------

float4 bilinear(float4 W, float4 X, float4 Y, float4 Z, float2 weight)
{
  float4 m0 = lerp(W, Z, weight.x);
  float4 m1 = lerp(X, Y, weight.x);
  return lerp(m0, m1, weight.y);
}

// Gather neighboring YUV macropixels from the given texture coordinate
void textureGatherYUV(
  Texture2D UYVYsampler,
  SamplerState samplerState, // You'd need to use a sampler state to sample in HLSL
  float2 tc,
  out float4 W,
  out float4 X,
  out float4 Y,
  out float4 Z)
{
  uint2 textureSize;
  UYVYsampler.GetDimensions(textureSize.x, textureSize.y);
    
  int2 tx = int2(tc * float2(textureSize.x, textureSize.y));
  int2 tmin = int2(0, 0);
  int2 tmax = int2(textureSize.x - 1, textureSize.y - 1);
    
  W = UYVYsampler.Sample(samplerState, tc); 
  X = UYVYsampler.Load(int3(clamp(tx + int2(0, 1), tmin, tmax), 0)); 
  Y = UYVYsampler.Load(int3(clamp(tx + int2(1, 1), tmin, tmax), 0)); 
  Z = UYVYsampler.Load(int3(clamp(tx + int2(1, 0), tmin, tmax), 0)); 
}

float4 PS(v2f input) : SV_Target
{
  /* The shader uses texelFetch to obtain the YUV macropixels to avoid unwanted interpolation
   * introduced by the GPU interpreting the YUV data as RGBA pixels.
   * The YUV macropixels are converted into individual RGB pixels and bilinear interpolation is applied.
   */
  float2 tc = input.Tex;
  float alpha = 1.0f;

  float4 macro, macro_u, macro_r, macro_ur;
  textureGatherYUV(txYUV, samLinear, tc, macro, macro_u, macro_ur, macro_r);

  //   Select the components for the bilinear interpolation based on the texture coordinate
  //   location within the YUV macropixel:
  //   -----------------          ----------------------
  //   | UY/VY | UY/VY |          | macro_u | macro_ur |
  //   |-------|-------|    =>    |---------|----------|
  //   | UY/VY | UY/VY |          | macro   | macro_r  |
  //   |-------|-------|          ----------------------
  //   | RG/BA | RG/BA |
  //   -----------------
  uint _width, _height = 0;
  txYUV.GetDimensions(_width, _height);
  uint2 textureSize = uint2(_width, _height);

  float2 off = frac(tc * textureSize);

  float4 pixel, pixel_r, pixel_u, pixel_ur;
  if (off.x > 0.5f)
  {
    pixel = rec709YCbCr2rgba(macro.a, macro.b, macro.r, alpha);
    pixel_r = rec709YCbCr2rgba(macro_r.g, macro_r.b, macro_r.r, alpha);
    pixel_u = rec709YCbCr2rgba(macro_u.a, macro_u.b, macro_u.r, alpha);
    pixel_ur = rec709YCbCr2rgba(macro_ur.g, macro_ur.b, macro_ur.r, alpha);
  }
  else
  {
    pixel = rec709YCbCr2rgba(macro.g, macro.b, macro.r, alpha);
    pixel_r = rec709YCbCr2rgba(macro.a, macro.b, macro.r, alpha);
    pixel_u = rec709YCbCr2rgba(macro_u.g, macro_u.b, macro_u.r, alpha);
    pixel_ur = rec709YCbCr2rgba(macro_u.a, macro_u.b, macro_u.r, alpha);
  }

  return bilinear(pixel, pixel_u, pixel_ur, pixel_r, off);
}
