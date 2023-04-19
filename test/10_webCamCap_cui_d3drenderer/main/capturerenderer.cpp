
#include "capturerenderer.h"

#include <d3d9.h>
#include <Dxva2api.h>
#include <evr.h>
#include <tchar.h>

#include <cstdio>
#include <iostream>

#include "MFUtility.h"

using namespace Renderer;

CaptureRenderer::CaptureRenderer()
  : m_pD3DVideoSample(nullptr)
  , m_pVideoSample(nullptr)
  , m_pDstBuffer(nullptr)
  , m_p2DBuffer(nullptr)
  , m_pVideoReader(nullptr)
  , m_evrTimeStamp(0)
{
}

bool CaptureRenderer::create(HWND hwnd, int deviceNo, int width, int height, int fps, const GUID subtype)
{
  HRESULT hr = S_OK;

  // Set up Video sink (EVR)
  ComPtr<IMFActivate> pActive = nullptr;
  hr = MFCreateVideoRendererActivate(hwnd, &pActive);
  if (hr != S_OK)
  {
    std::cerr << "Failed to created video rendered activation context." << std::endl;
    return false;
  }

  ComPtr<IMFMediaSink> pVideoSink;
  hr = pActive->ActivateObject(IID_IMFMediaSink, (void**)&pVideoSink);
  if (hr != S_OK)
  {
    std::cerr << "Failed to activate IMFMediaSink interface on video sink." << std::endl;
    return false;
  }

  // Initialize the renderer before doing anything else including querying for other interfaces,
  // see https://msdn.microsoft.com/en-us/library/windows/desktop/ms704667(v=vs.85).aspx.
  ComPtr<IMFVideoRenderer> pVideoRenderer = nullptr;
  hr = pVideoSink->QueryInterface(__uuidof(IMFVideoRenderer), (void**)&pVideoRenderer);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get video renderer interface from EVR media sink." << std::endl;
    return false;
  }

  hr = pVideoRenderer->InitializeRenderer(nullptr, nullptr);
  if (hr != S_OK)
  {
    std::cerr << "Failed to initialize the video renderer." << std::endl;
    return false;
  }

  ComPtr<IMFGetService> pService = nullptr;
  hr = pVideoSink->QueryInterface(__uuidof(IMFGetService), (void**)&pService);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get service interface from EVR media sink." << std::endl;
    return false;
  }

  ComPtr<IMFVideoDisplayControl> pVideoDisplayControl = nullptr;
  hr = pService->GetService(MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)&pVideoDisplayControl);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get video display control interface from service interface." << std::endl;
    return false;
  }

  hr = pVideoDisplayControl->SetVideoWindow(hwnd);
  if (hr != S_OK)
  {
    std::cerr << "Failed to SetVideoWindow." << std::endl;
    return false;
  }

  RECT rect = {0, 0, width, height};
  hr = pVideoDisplayControl->SetVideoPosition(nullptr, &rect);
  if (hr != S_OK)
  {
    std::cerr << "Failed to SetVideoPosition." << std::endl;
    return false;
  }

  ComPtr<IMFStreamSink> pStreamSink = nullptr;
  hr = pVideoSink->GetStreamSinkByIndex(0, &pStreamSink);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get video renderer stream by index." << std::endl;
    return false;
  }

  ComPtr<IMFMediaTypeHandler> pSinkMediaTypeHandler = nullptr;
  hr = pStreamSink->GetMediaTypeHandler(&pSinkMediaTypeHandler);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get media type handler for stream sink." << std::endl;
    return false;
  }

  DWORD sinkMediaTypeCount = 0;
  hr = pSinkMediaTypeHandler->GetMediaTypeCount(&sinkMediaTypeCount);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get sink media type count." << std::endl;
    return false;
  }
  std::cout << "Sink media type count : " << sinkMediaTypeCount << std::endl;

  // set up webcam video source.
  ComPtr<IMFMediaSource> pVideoSource = nullptr;
  hr = GetVideoSourceFromDevice(deviceNo, &pVideoSource, &m_pVideoReader);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get webcam video source." << std::endl;
    return false;
  }

  hr = m_pVideoReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, true);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get webcam video source." << std::endl;
    return false;
  }

  ComPtr<IMFPresentationDescriptor> pSourcePresentationDescriptor = nullptr;
  hr = pVideoSource->CreatePresentationDescriptor(&pSourcePresentationDescriptor);
  if (hr != S_OK)
  {
    std::cerr << "Failed to create the presentation descriptor from the meida source." << std::endl;
    return false;
  }

  BOOL fSelected = FALSE;
  ComPtr<IMFStreamDescriptor> pSourceStreamDescriptor = nullptr;
  hr = pSourcePresentationDescriptor->GetStreamDescriptorByIndex(0, &fSelected, &pSourceStreamDescriptor);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get source stream descriptor from presentation descriptor." << std::endl;
    return false;
  }

  ComPtr<IMFMediaTypeHandler> pSourceMediaTypeHandler = nullptr;
  hr = pSourceStreamDescriptor->GetMediaTypeHandler(&pSourceMediaTypeHandler);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get source media type handler." << std::endl;
    return false;
  }

  DWORD srcMediaTypeCount = 0;
  hr = pSourceMediaTypeHandler->GetMediaTypeCount(&srcMediaTypeCount);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get source media type count." << std::endl;
    return false;
  }

  // Attempt to set the desired media type on the webcam source.
  ComPtr<IMFMediaType> pWebcamSourceType = nullptr;
  hr = MFCreateMediaType(&pWebcamSourceType);
  if (hr != S_OK)
  {
    std::cerr << "Failed to create webcam output media type." << std::endl;
    return false;
  }

  hr = FindMatchingVideoType(pSourceMediaTypeHandler.Get(), subtype, width, height, fps, pWebcamSourceType.Get());
  if (hr != S_OK)
  {
    std::cerr << "No matching webcam media type was found." << std::endl;
    return false;
  }

  hr = pSourceMediaTypeHandler->SetCurrentMediaType(pWebcamSourceType.Get());
  if (hr != S_OK)
  {
    std::cerr << "Failed to set media type on source." << std::endl;
    return false;
  }

  ComPtr<IMFMediaType> pVideoSourceOutputType = nullptr;
  hr = pSourceMediaTypeHandler->GetCurrentMediaType(&pVideoSourceOutputType);
  if (hr != S_OK)
  {
    std::cerr << "Error retrieving current media type from first video stream." << std::endl;
    return false;
  }

  std::cout << "Webcam media type: " << std::endl;
  std::cout << GetMediaTypeDescription(pVideoSourceOutputType.Get()) << std::endl;

  // Set the video input type on the EVR sink.
  ComPtr<IMFMediaType> pImfEvrSinkMediaType = nullptr;
  hr = MFCreateMediaType(&pImfEvrSinkMediaType);
  if (hr != S_OK)
  {
    std::cerr << "Failed to create video output media type." << std::endl;
    return false;
  }

  hr = pImfEvrSinkMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
  if (hr != S_OK)
  {
    std::cerr << "Failed to set video output media major type." << std::endl;
    return false;
  }

  GUID rendererFormat = MFVideoFormat_RGB32;
  if (subtype == MFVideoFormat_MJPG)
  {
    rendererFormat = MFVideoFormat_YUY2;
  }
  hr = pImfEvrSinkMediaType->SetGUID(MF_MT_SUBTYPE, rendererFormat);
  if (hr != S_OK)
  {
    std::cerr << "Failed to set video sub-type attribute on meida type" << std::endl;
    return false;
  }

  hr = pImfEvrSinkMediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
  if (hr != S_OK)
  {
    std::cerr << "Failed to set interlace mode attribute on media type." << std::endl;
    return false;
  }

  hr = pImfEvrSinkMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
  if (hr != S_OK)
  {
    std::cerr << "Failed to set independent samples attribute on media type." << std::endl;
    return false;
  }

  hr = MFSetAttributeRatio(pImfEvrSinkMediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
  if (hr != S_OK)
  {
    std::cerr << "Failed to set pixel aspect ratio attribute on media type." << std::endl;
    return false;
  }

  hr = MFSetAttributeSize(pImfEvrSinkMediaType.Get(), MF_MT_FRAME_SIZE, width, height);
  if (hr != S_OK)
  {
    std::cerr << "Failed to set the frame size attribute on media type." << std::endl;
    return false;
  }

  hr = MFSetAttributeRatio(pImfEvrSinkMediaType.Get(), MF_MT_FRAME_RATE, fps, 1);
  if (hr != S_OK)
  {
    std::cerr << "Failed to set the frame rate attribute on media type." << std::endl;
    return false;
  }

  hr = pSinkMediaTypeHandler->SetCurrentMediaType(pImfEvrSinkMediaType.Get());
  if (hr != S_OK)
  {
    std::cerr << "Failed to set input media type on EVR sink." << std::endl;
    return false;
  }

  std::cout << "EVR input media:" << std::endl;
  std::cout << GetMediaTypeDescription(pImfEvrSinkMediaType.Get()) << std::endl;

  ComPtr<IMFMediaType> pSourceReaderType = nullptr;
  hr = MFCreateMediaType(&pSourceReaderType);
  if (hr != S_OK)
  {
    std::cerr << "Failed to source reader media type." << std::endl;
    return false;
  }

  hr = pImfEvrSinkMediaType->CopyAllItems(pSourceReaderType.Get());
  if (hr != S_OK)
  {
    std::cerr << "Error copying media type attributes from EVR input to source reader media type." << std::endl;
    return false;
  }

  // VERY IMPORTANT: Set the media type on the source reader to match the media type on the EVR. The
  // reader will do it's best to translate between the media type set on the webcam and the input type to the EVR.
  // If an error occurs copying the sample in the read-loop then it's usually because the reader could not translate
  // the types.
  hr = m_pVideoReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pSourceReaderType.Get());
  if (hr != S_OK)
  {
    std::cerr << "Failed to set output media type on webcam source reader." << std::endl;
    return false;
  }

  // Source and sink now configured. Set up remaining infrastracture.
  // Get Direct3D surface organised.
  ComPtr<IMFVideoSampleAllocator> pVideoSampleAllocator = nullptr;
  hr = MFGetService(pStreamSink.Get(), MR_VIDEO_ACCELERATION_SERVICE, IID_PPV_ARGS(&pVideoSampleAllocator));
  if (hr != S_OK)
  {
    std::cerr << "Failed to get IMFVideoSampleAllocator." << std::endl;
    return false;
  }

  ComPtr<IDirect3DDeviceManager9> pD3DManager = nullptr;
  hr = MFGetService(pVideoSink.Get(), MR_VIDEO_ACCELERATION_SERVICE, IID_PPV_ARGS(&pD3DManager));
  if (hr != S_OK)
  {
    std::cerr << "Failed to get Direct3D manager from EVR media sink." << std::endl;
    return false;
  }

  hr = pVideoSampleAllocator->SetDirectXManager(pD3DManager.Get());
  if (hr != S_OK)
  {
    std::cerr << "Failed to set D3DManager on video sample allocator." << std::endl;
    return false;
  }

  hr = pVideoSampleAllocator->InitializeSampleAllocator(1, pImfEvrSinkMediaType.Get());
  if (hr != S_OK)
  {
    std::cerr << "Failed to initialize video sample allocator." << std::endl;
    return false;
  }

  hr = pVideoSampleAllocator->AllocateSample(&m_pD3DVideoSample);
  if (hr != S_OK)
  {
    std::cerr << "Failed to allocate video sample." << std::endl;
    return false;
  }

  // Get clocks organized
  ComPtr<IMFPresentationClock> pClock = nullptr;
  hr = MFCreatePresentationClock(&pClock);
  if (hr != S_OK)
  {
    std::cerr << "Failed to create presentatin clock." << std::endl;
    return false;
  }

  ComPtr<IMFPresentationTimeSource> pTimeSource = nullptr;
  hr = MFCreateSystemTimeSource(&pTimeSource);
  if (hr != S_OK)
  {
    std::cerr << "Failed to create system time source." << std::endl;
    return false;
  }

  hr = pClock->SetTimeSource(pTimeSource.Get());
  if (hr != S_OK)
  {
    std::cerr << "Failed to set time source." << std::endl;
    return false;
  }

  hr = pVideoSink->SetPresentationClock(pClock.Get());
  if (hr != S_OK)
  {
    std::cerr << "Failed to set presentation clock on video sink." << std::endl;
    return false;
  }

  hr = pClock->Start(0);
  if (hr != S_OK)
  {
    std::cerr << "Error starting presentation clock." << std::endl;
    return false;
  }

  return true;
}

void CaptureRenderer::render()
{
  HRESULT hr = S_OK;
  DWORD streamIndex = 0;
  DWORD flags = 0;
  LONGLONG llTimeStamp = 0;

  hr = m_pVideoReader->ReadSample(
    MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , 0
    , &streamIndex
    , &flags
    , &llTimeStamp
    , &m_pVideoSample
    );
  if (hr != S_OK)
  {
    std::cerr << "Error reading video sample." << std::endl;
    return;
  }

  if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
  {
    std::cout << "End of stream." << std::endl;
    return;
  }

  if (flags & MF_SOURCE_READERF_STREAMTICK)
  {
    std::cout << "Stream tick." << std::endl;
    return;
  }

  if (!m_pVideoSample)
  {
    std::cerr << "Null video sample." << std::endl;
    return;
  }

  LONGLONG sampleDuration = 0;
  DWORD mfOutFlags = 0;

  // video source sample.
  hr = m_pVideoSample->SetSampleTime(llTimeStamp);
  if (hr != S_OK)
  {
    std::cerr << "Error setting the video sample time." << std::endl;
    return;
  }

  hr = m_pVideoSample->GetSampleDuration(&sampleDuration);
  if (hr != S_OK)
  {
    std::cerr << "Failed to get video sample duration." << std::endl;
    return;
  }

  std::cout << "Attempting to convert sample, sample duration " << sampleDuration
            << ", sample time " << llTimeStamp
            << ", evr timestamp " << m_evrTimeStamp
            << std::endl;

  // Make Direct3D sample.
  
}

void CaptureRenderer::destroy()
{
  
}
