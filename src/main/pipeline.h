
#ifndef PIPELINE_H_
#define PIPELINE_H_

#include "stdafx.h"

using Microsoft::WRL::ComPtr;

namespace Render
{

class Pipeline
{
public:
  explicit Pipeline();
  ~Pipeline();

  bool create(D3D11_INPUT_ELEMENT_DESC* input_elements, uint32_t ninput_elements);
  void activate() const;
  void destroy();

private:
  ComPtr<ID3D11VertexShader> m_vs;
  ComPtr<ID3D11InputLayout>  m_input_layout;
  ComPtr<ID3D11PixelShader>  m_ps;

  bool compileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut);
};

} // Render

#endif // PIPELINE_H_
