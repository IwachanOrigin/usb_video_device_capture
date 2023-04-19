
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
  : m_pVideoSample(nullptr)
  , m_pDstBuffer(nullptr)
  , m_p2DBuffer(nullptr)
  , m_pVideoReader(nullptr)
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

  return true;
}

void CaptureRenderer::render()
{
  
}

void CaptureRenderer::destroy()
{
  
}
