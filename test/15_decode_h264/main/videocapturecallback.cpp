
#include "stdafx.h"
#include "dx11manager.h"
#include "timer.h"
#include "capturemanager.h"
#include "utils.h"
#include "videocapturecallback.h"

using namespace Microsoft::WRL;
using namespace helper;

VideoCaptureCB::VideoCaptureCB()
  : m_ref(1)
  , m_sourceReader(nullptr)
  , m_colorConvTransform(nullptr)
  , m_h264ToNv12Transform(nullptr)
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

  ComPtr<IUnknown> h264ToNv12TransformUnk = nullptr;
  // https://learn.microsoft.com/ja-jp/windows/win32/medfound/h-264-video-decoder
  hr = CoCreateInstance(CLSID_CMSH264DecoderMFT, nullptr, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)h264ToNv12TransformUnk.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create h264 to nv12 transform unknown.", L"Error", MB_OK);
    return hr;
  }

  hr = h264ToNv12TransformUnk->QueryInterface(IID_PPV_ARGS(m_h264ToNv12Transform.GetAddressOf()));
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get IMFTransform interface from h264 to nv12 MFT object..", L"Error", MB_OK);
    return hr;
  }

  // NV12 to RGB32
  hr = MFCreateMediaType(m_DecoderOutputMediaType.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create media type.", L"Error", MB_OK);
    return hr;
  }

  ComPtr<IMFMediaType> webCamMediaType = nullptr;
  UINT32 index = this->getOptimizedFormatIndex();
  hr = m_sourceReader->GetNativeMediaType(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , (DWORD)index
    , webCamMediaType.GetAddressOf()
    );
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get the camera current media type.", L"Error", MB_OK);
    return hr;
  }

  hr = m_sourceReader->SetCurrentMediaType(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , NULL
    , webCamMediaType.Get()
  );
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set the camera media type.", L"Error", MB_OK);
    return hr;
  }

  // default : 640, 480, 30, yuy2
  // chenged : 1920, 1080, 30, h264
  hr = m_h264ToNv12Transform->SetInputType(0, webCamMediaType.Get(), 0);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set input media type on h264 to nv12 MFT.", L"Error", MB_OK);
    return hr;
  }

  hr = utilCloneVideomediaType(webCamMediaType.Get(), MFVideoFormat_NV12, m_h264ToNv12MediaType.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to copy the camera current media type to the h264 to nv12  media type.", L"Error", MB_OK);
    return hr;
  }

  hr = m_h264ToNv12Transform->SetOutputType(0, m_h264ToNv12MediaType.Get(), 0);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set output media type on h264 to nv12 MFT.", L"Error", MB_OK);
    return hr;
  }

  DWORD mftStatus = 0;
  hr = m_h264ToNv12Transform->GetInputStatus(0, &mftStatus);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get input media status from h264 to nv12  MFT.", L"Error", MB_OK);
    return hr;
  }
  if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus)
  {
    MessageBoxW(nullptr, L"h264 to nv12 MFT is not accepting data.", L"Error", MB_OK);
    return hr;
  }

  hr = m_h264ToNv12Transform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to process FLUSH command on h264 to nv12 MFT.", L"Error", MB_OK);
    return hr;
  }

  hr = m_h264ToNv12Transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to process BEGIN_STREAMING command on h264 to nv12 MFT.", L"Error", MB_OK);
    return hr;
  }

  hr = m_h264ToNv12Transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to process START_OF_STREAM command on h264 to nv12 MFT.", L"Error", MB_OK);
    return hr;
  }


  // nv12 to rgb32
  hr = m_colorConvTransform->SetInputType(0, m_h264ToNv12MediaType.Get(), 0);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set input media type on color conversion MFT.", L"Error", MB_OK);
    return hr;
  }

  hr = utilCloneVideomediaType(webCamMediaType.Get(), MFVideoFormat_RGB32, m_DecoderOutputMediaType.GetAddressOf());
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

  mftStatus = 0;
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

      std::wcout << "Processing sample : " << m_sampleCount << std::endl;
      LONGLONG llSampleDuration = 0;
      DWORD sampleFlags = 0;
      hr = sample->SetSampleTime(llTimeStamp);
      hr = sample->GetSampleDuration(&llSampleDuration);
      hr = sample->GetSampleFlags(&sampleFlags);
      printf("Sample flags %d, sample duration %I64d, sample time %I64d\n", sampleFlags, llSampleDuration, llTimeStamp);

      // h264 to nv12
      hr = m_h264ToNv12Transform->ProcessInput(0, sample, NULL);
      if (FAILED(hr))
      {
        if (hr == MF_E_NOTACCEPTING)
        {
          std::wcout << "The h264 to nv12 decoder MF_E_NOTACCEPTING. Because the buffer is ready to be retrieved from the processOutput method." << std::endl;
        }
        else
        {
          this->outputError(L"The h264 to nv12 decoder ProcessInput call failed.", sample);
          return hr;
        }
      }

      MFT_OUTPUT_STREAM_INFO h264ToNv12StreamInfo{};
      DWORD h264ToNv12ProcessOutputStatus = 0;

      hr = m_colorConvTransform->GetOutputStreamInfo(0, &h264ToNv12StreamInfo);
      if (FAILED(hr))
      {
        this->outputError(L"Failed to get output stream info from h264 to nv12 MFT.", sample);
        return hr;
      }

      ComPtr<IMFSample> mftOutH264ToNv12Sample = nullptr;
      hr = MFCreateSample(mftOutH264ToNv12Sample.GetAddressOf());
      if (FAILED(hr))
      {
        this->outputError(L"Failed to create H264 to nv12 MF sample.", sample);
        return hr;
      }

      ComPtr<IMFMediaBuffer> mftOutH264ToNv12Buffer = nullptr;
      hr = MFCreateMemoryBuffer(h264ToNv12StreamInfo.cbSize, mftOutH264ToNv12Buffer.GetAddressOf());
      if (FAILED(hr))
      {
        this->outputError(L"Failed to create h264 t nv12 memory buffer.", sample);
        return hr;
      }

      hr = mftOutH264ToNv12Sample->AddBuffer(mftOutH264ToNv12Buffer.Get());
      if (FAILED(hr))
      {
        this->outputError(L"Failed to add sample to h264 to nv12 buffer.", sample);
        return hr;
      }

      MFT_OUTPUT_DATA_BUFFER outputH264ToNv12DataBuffer{};
      outputH264ToNv12DataBuffer.dwStreamID = 0;
      outputH264ToNv12DataBuffer.dwStatus = 0;
      outputH264ToNv12DataBuffer.pEvents = NULL;
      outputH264ToNv12DataBuffer.pSample = mftOutH264ToNv12Sample.Get();
      auto mftH264ToNv12ProcessOutput = m_h264ToNv12Transform->ProcessOutput(0, 1, &outputH264ToNv12DataBuffer, &h264ToNv12ProcessOutputStatus);
      if (SUCCEEDED(mftH264ToNv12ProcessOutput))
      {
        std::wcout << "h264 to nv12 result : " << mftH264ToNv12ProcessOutput << ", MFT status : " << h264ToNv12ProcessOutputStatus << std::endl;
        // nv12 to rgb32
        hr = m_colorConvTransform->ProcessInput(0, mftOutH264ToNv12Sample.Get(), NULL);
        if (FAILED(hr))
        {
          std::cerr << "The color conversion decoder ProcessInput call failed." << std::endl;
        }

        MFT_OUTPUT_STREAM_INFO streamInfo{};
        DWORD processOutputStatus = 0;

        hr = m_colorConvTransform->GetOutputStreamInfo(0, &streamInfo);
        if (FAILED(hr))
        {
          std::cerr << "Failed to get output stream info from color conversion MFT." << std::endl;
        }

        ComPtr<IMFSample> mftOutSample = nullptr;
        hr = MFCreateSample(mftOutSample.GetAddressOf());
        if (FAILED(hr))
        {
          std::cerr << "Failed to create MF sample." << std::endl;
        }

        ComPtr<IMFMediaBuffer> mftOutBuffer = nullptr;
        hr = MFCreateMemoryBuffer(streamInfo.cbSize, mftOutBuffer.GetAddressOf());
        if (FAILED(hr))
        {
          std::cerr << "Failed to create memory buffer." << std::endl;
        }

        hr = mftOutSample->AddBuffer(mftOutBuffer.Get());
        if (FAILED(hr))
        {
          std::cerr << "Failed to add sample to buffer." << std::endl;
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
#if 0
          // Update texture
          bool result = manager::DX11Manager::getInstance().updateTexture(byteBuffer, buffCurrLen);
          if (!result)
          {
            buf->Unlock();
            MessageBoxW(nullptr, L"Failed to update texture.", L"Error", MB_OK);
          }

          // Rendering
          result = manager::DX11Manager::getInstance().render();
          if (!result)
          {
            buf->Unlock();
            MessageBoxW(nullptr, L"Failed to rendering.", L"Error", MB_OK);
          }
#endif
          std::wcout << "current size : " << buffCurrLen << std::endl;
          buf->Unlock();
        }
      }
      else if (mftH264ToNv12ProcessOutput == MF_E_TRANSFORM_NEED_MORE_INPUT)
      {
        std::wcout << "h264 to nv12 result : MF_E_TRANSFORM_NEED_MORE_INPUT" << std::endl;
      }

      if (sample)
      {
        sample->Release();
      }
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
      if (subtype != MFVideoFormat_H264)
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
      }
    }
  }
  return index;
}

void VideoCaptureCB::outputError(const std::wstring& errMsg, IMFSample* sample)
{
  std::wcerr << errMsg << std::endl;
  if (sample)
  {
    sample->Release();
  }

  // Request next frame
  HRESULT hr = m_sourceReader->ReadSample(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , 0
    , nullptr
    , nullptr
    , nullptr
    , nullptr
  );

  LeaveCriticalSection(&m_criticalSection);
}