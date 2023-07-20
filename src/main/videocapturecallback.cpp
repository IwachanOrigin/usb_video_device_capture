
#include "stdafx.h"
#include "dx11manager.h"
#include "utils.h"
#include "videocapturecallback.h"

using namespace Microsoft::WRL;
using namespace helper;

VideoCaptureCB::VideoCaptureCB()
  : m_ref(1)
  , m_sourceReader(nullptr)
  , m_colorConvTransform(nullptr)
  , m_DecoderOutputMediaType(nullptr)
  , m_sampleCount(0)
{
  InitializeCriticalSection(&m_criticalSection);
}

VideoCaptureCB::~VideoCaptureCB()
{
  DeleteCriticalSection(&m_criticalSection);
}

STDMETHODIMP VideoCaptureCB::QueryInterface(REFIID riid, void** ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(VideoCaptureCB, IMFSourceReaderCallback)
    , { 0 }
  };
  return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) VideoCaptureCB::AddRef()
{
  return InterlockedIncrement(&m_ref);
}

STDMETHODIMP_(ULONG) VideoCaptureCB::Release()
{
  LONG ref = InterlockedDecrement(&m_ref);
  if (ref == 0)
  {
    delete this;
  }
  return ref;
}

HRESULT VideoCaptureCB::setSourceReader(IMFSourceReader* sourceReader)
{
  HRESULT hr = E_FAIL;

  if (!sourceReader)
  {
    return hr;
  }
  m_sourceReader = sourceReader;

  ComPtr<IUnknown> colorConvTransformUnk = nullptr;
  // https://learn.microsoft.com/ja-jp/windows/win32/medfound/colorconverter
  // https://learn.microsoft.com/ja-jp/windows/win32/medfound/video-processor-mft
  hr = CoCreateInstance(CLSID_CColorConvertDMO, nullptr, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)colorConvTransformUnk.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create color conversion transform unknown.", L"Error", MB_OK);
    return hr;
  }

  hr = colorConvTransformUnk->QueryInterface(IID_PPV_ARGS(m_colorConvTransform.GetAddressOf()));
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get IMFTransform interface from color converter MFT object..", L"Error", MB_OK);
    return hr;
  }

  hr = MFCreateMediaType(m_DecoderOutputMediaType.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create media type.", L"Error", MB_OK);
    return hr;
  }

  ComPtr<IMFMediaType> deviceDefaultMediaType = nullptr;
  hr = m_sourceReader->GetCurrentMediaType(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , deviceDefaultMediaType.GetAddressOf()
    );
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get the device current media type.", L"Error", MB_OK);
    return hr;
  }
  std::wcout << "----- Default device media info -----" << std::endl;
  std::cout << GetMediaTypeDescription(deviceDefaultMediaType.Get()) << std::endl << std::endl;

  ComPtr<IMFMediaType> deviceMediaType = nullptr;
  UINT32 index = this->getOptimizedFormatIndex();
  hr = m_sourceReader->GetNativeMediaType(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , (DWORD)index
    , deviceMediaType.GetAddressOf()
    );
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get the device optimized media type.", L"Error", MB_OK);
    return hr;
  }
  std::wcout << "----- Optimized device media info -----" << std::endl;
  std::cout << GetMediaTypeDescription(deviceMediaType.Get()) << std::endl << std::endl;

  hr = m_sourceReader->SetCurrentMediaType(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , NULL
    , deviceMediaType.Get()
  );
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set the camera media type.", L"Error", MB_OK);
    return hr;
  }

  // default : 640, 480, 30, yuy2
  // chenged : 1920, 1080, 30, nv12 or yuy2 or rgb32
  hr = m_colorConvTransform->SetInputType(0, deviceMediaType.Get(), 0);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set input media type on color conversion MFT.", L"Error", MB_OK);
    return hr;
  }

  hr = utilCloneVideomediaType(deviceMediaType.Get(), MFVideoFormat_RGB32, m_DecoderOutputMediaType.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to copy the camera current media type to the decoder output media type.", L"Error", MB_OK);
    return hr;
  }

  hr = m_colorConvTransform->SetOutputType(0, m_DecoderOutputMediaType.Get(), 0);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set output media type on color conversion MFT.", L"Error", MB_OK);
    return hr;
  }

  DWORD mftStatus = 0;
  hr = m_colorConvTransform->GetInputStatus(0, &mftStatus);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get input media status from color conversion MFT.", L"Error", MB_OK);
    return hr;
  }
  if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus)
  {
    MessageBoxW(nullptr, L"Color conversion MFT is not accepting data.", L"Error", MB_OK);
    return hr;
  }

  hr = m_colorConvTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to process FLUSH command on color conversion MFT.", L"Error", MB_OK);
    return hr;
  }

  hr = m_colorConvTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to process BEGIN_STREAMING command on color conversion MFT.", L"Error", MB_OK);
    return hr;
  }

  hr = m_colorConvTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to process START_OF_STREAM command on color conversion MFT.", L"Error", MB_OK);
    return hr;
  }

  hr = this->setCaptureResolutionAndFps();
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to init capture resolution and fps to internal variables.", L"Error", MB_OK);
    return hr;
  }

  return hr;
}

STDMETHODIMP VideoCaptureCB::OnReadSample(
    HRESULT hrStatus
    , DWORD dwStreamIndex
    , DWORD dwStreamFlags
    , LONGLONG llTimeStamp
    , IMFSample* sample)
{
  HRESULT hr = S_OK;

  EnterCriticalSection(&m_criticalSection);

  if (SUCCEEDED(hrStatus))
  {
    if (sample)
    {
      sample->AddRef();

      //std::wcout << "Processing video sample : " << m_sampleCount << std::endl;
      LONGLONG llSampleDuration = 0;
      DWORD sampleFlags = 0;
      hr = sample->SetSampleTime(llTimeStamp);
      hr = sample->GetSampleDuration(&llSampleDuration);
      hr = sample->GetSampleFlags(&sampleFlags);
      //printf("video Sample flags %d, sample duration %I64d, sample time %I64d\n", (int)sampleFlags, llSampleDuration, llTimeStamp);
#if 1
      // Send to gpu to cpu
      ComPtr<IMFMediaBuffer> buf = nullptr;
      hr = sample->ConvertToContiguousBuffer(buf.GetAddressOf());
      if (FAILED(hr))
      {
        std::cerr << "Failed the ConvertToContiguousBuffer." << std::endl;
      }

      byte* byteBuffer = nullptr;
      DWORD buffCurrLen = 0;
      hr = buf->Lock(&byteBuffer, NULL, &buffCurrLen);
      if (FAILED(hr))
      {
        std::cerr << "Failed the ConvertToContiguousBuffer." << std::endl;
      }

      // Update texture
      bool result = manager::DX11Manager::getInstance().updateTexture(byteBuffer, buffCurrLen);
      if (!result)
      {
        buf->Unlock();
        std::wcerr << "Failed to update texture." << std::endl;
      }

      // Rendering
      result = manager::DX11Manager::getInstance().render();
      if (!result)
      {
        buf->Unlock();
        std::wcerr << "Failed to rendering." << std::endl;
      }
      buf->Unlock();

#else
      hr = m_colorConvTransform->ProcessInput(0, sample, NULL);
      if (FAILED(hr))
      {
        std::cerr << "The color conversion decoder ProcessInput call failed." << std::endl;
        sample->Release();
      }

      MFT_OUTPUT_STREAM_INFO streamInfo{};
      DWORD processOutputStatus = 0;

      hr = m_colorConvTransform->GetOutputStreamInfo(0, &streamInfo);
      if (FAILED(hr))
      {
        std::cerr << "Failed to get output stream info from color conversion MFT." << std::endl;
        sample->Release();
      }

      ComPtr<IMFSample> mftOutSample = nullptr;
      hr = MFCreateSample(mftOutSample.GetAddressOf());
      if (FAILED(hr))
      {
        std::cerr << "Failed to create MF sample." << std::endl;
        sample->Release();
      }

      ComPtr<IMFMediaBuffer> mftOutBuffer = nullptr;
      hr = MFCreateMemoryBuffer(streamInfo.cbSize, mftOutBuffer.GetAddressOf());
      if (FAILED(hr))
      {
        std::cerr << "Failed to create memory buffer." << std::endl;
        sample->Release();
      }

      hr = mftOutSample->AddBuffer(mftOutBuffer.Get());
      if (FAILED(hr))
      {
        std::cerr << "Failed to add sample to buffer." << std::endl;
        sample->Release();
      }

      MFT_OUTPUT_DATA_BUFFER outputDataBuffer{};
      outputDataBuffer.dwStreamID = 0;
      outputDataBuffer.dwStatus = 0;
      outputDataBuffer.pEvents = NULL;
      outputDataBuffer.pSample = mftOutSample.Get();
      auto mftProcessOutput = m_colorConvTransform->ProcessOutput(0, 1, &outputDataBuffer, &processOutputStatus);
      if (SUCCEEDED(mftProcessOutput))
      {
        std::wcout << "Color conversion result : " << mftProcessOutput << ", MFT status : " << processOutputStatus << std::endl;
        ComPtr<IMFMediaBuffer> buf = nullptr;
        hr = mftOutSample->ConvertToContiguousBuffer(buf.GetAddressOf());
        if (FAILED(hr))
        {
          std::cerr << "Failed the ConvertToContiguousBuffer." << std::endl;
        }

        byte* byteBuffer = nullptr;
        DWORD buffCurrLen = 0;
        hr = buf->Lock(&byteBuffer, NULL, &buffCurrLen);
        if (FAILED(hr))
        {
          std::cerr << "Failed the ConvertToContiguousBuffer." << std::endl;
        }

        // Update texture
        bool result = manager::DX11Manager::getInstance().updateTexture(byteBuffer, buffCurrLen);
        if (!result)
        {
          buf->Unlock();
          //MessageBoxW(nullptr, L"Failed to update texture.", L"Error", MB_OK);
          std::wcerr << "Failed to update texture." << std::endl;
        }

        // Rendering
        result = manager::DX11Manager::getInstance().render();
        if (!result)
        {
          buf->Unlock();
          //MessageBoxW(nullptr, L"Failed to rendering.", L"Error", MB_OK);
          std::wcerr << "Failed to rendering." << std::endl;
        }
        buf->Unlock();
      }
      else if (mftProcessOutput == MF_E_TRANSFORM_NEED_MORE_INPUT)
      {
        std::wcout << "Color conversion result : MF_E_TRANSFORM_NEED_MORE_INPUT" << std::endl;
      }
#endif
      sample->Release();
      m_sampleCount++;
    }
  }

  // Request next frame
  hr = m_sourceReader->ReadSample(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , 0
    , nullptr
    , nullptr
    , nullptr
    , nullptr
    );

  LeaveCriticalSection(&m_criticalSection);

  return S_OK;
}

UINT32 VideoCaptureCB::getOptimizedFormatIndex()
{
  UINT32 index = 0, wMax = 0, rMax = 0;
  for (DWORD i = 0; ; i++)
  {
    ComPtr<IMFMediaType> pType = nullptr;
    HRESULT hr = m_sourceReader->GetNativeMediaType(
      (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
      , i
      , pType.GetAddressOf()
    );

    if (FAILED(hr))
    {
      break;
    }

    if (SUCCEEDED(hr))
    {
      GUID subtype{};
      hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
      if (FAILED(hr))
      {
        continue;
      }

      // Check
      VideoCaptureFormat fmt = VideoCaptureFormat::VideoCapFmt_NONE;
      if (!this->isAcceptedFormat(subtype, fmt))
      {
        continue;
      }

      // Found an output type.
      UINT32 rate = 0, den = 0, width = 0, height = 0;
      hr = MFGetAttributeSize(pType.Get(), MF_MT_FRAME_RATE, &rate, &den);
      rate /= den;
      hr = MFGetAttributeSize(pType.Get(), MF_MT_FRAME_SIZE, &width, &height);
      if (width >= wMax && rate >= rMax)
      {
        wMax = width;
        rMax = rate;
        index = i;
        m_vcf = fmt;
      }
    }
  }
  return index;
}

bool VideoCaptureCB::isAcceptedFormat(const GUID& subtype, VideoCaptureFormat& fmt)
{
  if (subtype == MFVideoFormat_NV12)
  {
    fmt = VideoCaptureFormat::VideoCapFmt_NV12;
    return true;
  }
  else if (subtype == MFVideoFormat_YUY2)
  {
    fmt = VideoCaptureFormat::VideoCapFmt_YUY2;
    return true;
  }
  else if (subtype == MFVideoFormat_RGB32)
  {
    fmt = VideoCaptureFormat::VideoCapFmt_RGB32;
    return true;
  }

  return false;
}

HRESULT VideoCaptureCB::setCaptureResolutionAndFps()
{
  if (!m_sourceReader)
  {
    std::wcerr << "Source Reader is NULL." << std::endl;
    return E_FAIL;
  }
  ComPtr<IMFMediaType> pType = nullptr;
  HRESULT hr = m_sourceReader->GetCurrentMediaType(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , pType.GetAddressOf()
    );

  if (FAILED(hr))
  {
    std::wcerr << "Failed to get current media type." << std::endl;
    return E_FAIL;
  }

  // Found an output type.
  uint32_t rate = 0, den = 0, width = 0, height = 0;
  hr = MFGetAttributeSize(pType.Get(), MF_MT_FRAME_RATE, &rate, &den);
  if (FAILED(hr))
  {
    std::wcerr << "Failed to get frame rate from current media type." << std::endl;
    return E_FAIL;
  }
  rate /= den;
  m_capFps = rate;

  hr = MFGetAttributeSize(pType.Get(), MF_MT_FRAME_SIZE, &width, &height);
  if (FAILED(hr))
  {
    std::wcerr << "Failed to get frame size from current media type." << std::endl;
    return E_FAIL;
  }
  m_capWidth = width;
  m_capHeight = height;

  return S_OK;
}
