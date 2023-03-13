
#include <d3dcompiler.h>

#include "dx11base.h"
#include "pipeline.h"
#include "dxhelper.h"

// Header files automatically generated when compiling HLSL.
// See set_source_files_properties on CMakeLists.txt.
#include "videoPS.h"
#include "videoVS.h"

using namespace Render;

Pipeline::Pipeline()
  : m_vs(nullptr)
  , m_ps(nullptr)
  , m_input_layout(nullptr)
{
}

Pipeline::~Pipeline()
{
}

bool Pipeline::create(D3D11_INPUT_ELEMENT_DESC* input_elements, uint32_t ninput_elements)
{
  HRESULT hr = S_OK;
  // Create the vertex shader
  hr = DX11Base::getInstance().getDevice()->CreateVertexShader(g_videoVS, sizeof(g_videoVS), nullptr, m_vs.GetAddressOf());
  if (FAILED(hr))
  {
    return hr;
  }

  // Create the input layout
  hr = DX11Base::getInstance().getDevice()->CreateInputLayout(input_elements, ninput_elements, g_videoVS, sizeof(g_videoVS), m_input_layout.GetAddressOf());
  if (FAILED(hr))
  {
    return hr;
  }

  // Create the pixel shader
  hr = DX11Base::getInstance().getDevice()->CreatePixelShader(g_videoPS, sizeof(g_videoPS), nullptr, m_ps.GetAddressOf());
  if (FAILED(hr))
  {
    return hr;
  }

  return true;
}

void Pipeline::activate() const
{
  DX11Base::getInstance().getDeviceContext()->IASetInputLayout(m_input_layout.Get());
  DX11Base::getInstance().getDeviceContext()->VSSetShader(m_vs.Get(), nullptr, 0);
  DX11Base::getInstance().getDeviceContext()->PSSetShader(m_ps.Get(), nullptr, 0);
}

void Pipeline::destroy()
{
  SAFE_RELEASE(m_vs);
  SAFE_RELEASE(m_ps);
  SAFE_RELEASE(m_input_layout);
}

bool Pipeline::compileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut)
{
  HRESULT hr = S_OK;
  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
  dwShaderFlags |= D3DCOMPILE_DEBUG;
  dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ID3DBlob* pErrorBlob = nullptr;

  wchar_t wFilename[MAX_PATH]{};
  size_t convertedCount = 0;
  mbstowcs_s(&convertedCount, wFilename, szFileName, MAX_PATH);

  hr = D3DCompileFromFile(
    wFilename
    , nullptr
    , D3D_COMPILE_STANDARD_FILE_INCLUDE
    , szEntryPoint
    , szShaderModel
    , dwShaderFlags
    , 0
    , ppBlobOut
    , &pErrorBlob
    );
  if (FAILED(hr))
  {
    if (pErrorBlob)
    {
      OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
    }
    return false;
  }
  if (pErrorBlob)
  {
    pErrorBlob->Release();
  }
  return true;
}


