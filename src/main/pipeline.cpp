
#include <d3dcompiler.h>

#include "pipeline.h"

// Header files automatically generated when compiling HLSL.
// See set_source_files_properties on CMakeLists.txt.
#include "videoNV12PS.h"
#include "videoNV12VS.h"
#include "videoRGB32PS.h"
#include "videoRGB32VS.h"
#include "videoYUY2PS.h"
#include "videoYUY2VS.h"

using namespace renderer;

Pipeline::Pipeline()
  : m_vs(nullptr)
  , m_ps(nullptr)
  , m_inputLayout(nullptr)
  , m_d3dDevice(nullptr)
  , m_immediateContext(nullptr)
{
}

Pipeline::~Pipeline()
{
}

bool Pipeline::create(D3D11_INPUT_ELEMENT_DESC* input_elements, uint32_t ninput_elements, ComPtr<ID3D11Device> d3dDevice, ComPtr<ID3D11DeviceContext> immediateContext, const VideoCaptureFormat& fmt)
{
  HRESULT hr = S_OK;
  m_d3dDevice = d3dDevice;
  m_immediateContext = immediateContext;
  switch (fmt)
  {
    case VideoCaptureFormat::VideoCapFmt_NV12:
    {
      // Create the vertex shader
      hr = m_d3dDevice->CreateVertexShader(g_videoNV12VS, sizeof(g_videoNV12VS), nullptr, m_vs.GetAddressOf());
      if (FAILED(hr))
      {
        return false;
      }
      // Create the input layout
      hr = m_d3dDevice->CreateInputLayout(input_elements, ninput_elements, g_videoNV12VS, sizeof(g_videoNV12VS), m_inputLayout.GetAddressOf());
      if (FAILED(hr))
      {
        return false;
      }
      // Create the pixel shader
      hr = m_d3dDevice->CreatePixelShader(g_videoNV12PS, sizeof(g_videoNV12PS), nullptr, m_ps.GetAddressOf());
      if (FAILED(hr))
      {
        return false;
      }
    }
    break;

    case VideoCaptureFormat::VideoCapFmt_YUY2:
    {
      // Create the vertex shader
      hr = m_d3dDevice->CreateVertexShader(g_videoYUY2VS, sizeof(g_videoYUY2VS), nullptr, m_vs.GetAddressOf());
      if (FAILED(hr))
      {
        return false;
      }
      // Create the input layout
      hr = m_d3dDevice->CreateInputLayout(input_elements, ninput_elements, g_videoYUY2VS, sizeof(g_videoYUY2VS), m_inputLayout.GetAddressOf());
      if (FAILED(hr))
      {
        return false;
      }
      // Create the pixel shader
      hr = m_d3dDevice->CreatePixelShader(g_videoYUY2PS, sizeof(g_videoYUY2PS), nullptr, m_ps.GetAddressOf());
      if (FAILED(hr))
      {
        return false;
      }
    }
    break;

    case VideoCaptureFormat::VideoCapFmt_DMO:
    case VideoCaptureFormat::VideoCapFmt_RGB32:
    {
      // Create the vertex shader
      hr = m_d3dDevice->CreateVertexShader(g_videoRGB32VS, sizeof(g_videoRGB32VS), nullptr, m_vs.GetAddressOf());
      if (FAILED(hr))
      {
        return false;
      }
      // Create the input layout
      hr = m_d3dDevice->CreateInputLayout(input_elements, ninput_elements, g_videoRGB32VS, sizeof(g_videoRGB32VS), m_inputLayout.GetAddressOf());
      if (FAILED(hr))
      {
        return false;
      }
      // Create the pixel shader
      hr = m_d3dDevice->CreatePixelShader(g_videoRGB32PS, sizeof(g_videoRGB32PS), nullptr, m_ps.GetAddressOf());
      if (FAILED(hr))
      {
        return false;
      }
    }
    break;

    default:
    {
      return false;
    }
  }

  return true;
}

void Pipeline::activate() const
{
  m_immediateContext->IASetInputLayout(m_inputLayout.Get());
  m_immediateContext->VSSetShader(m_vs.Get(), nullptr, 0);
  m_immediateContext->PSSetShader(m_ps.Get(), nullptr, 0);
}

void Pipeline::destroy()
{
  if (m_vs)
  {
    m_vs->Release();
    m_vs = nullptr;
  }
  if (m_ps)
  {
    m_ps->Release();
    m_ps = nullptr;
  }
  if (m_inputLayout)
  {
    m_inputLayout->Release();
    m_inputLayout = nullptr;
  }
}

bool Pipeline::compileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut)
{
  HRESULT hr = S_OK;
  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
  dwShaderFlags |= D3DCOMPILE_DEBUG;
  dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ComPtr<ID3DBlob> pErrorBlob = nullptr;

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

  return true;
}

