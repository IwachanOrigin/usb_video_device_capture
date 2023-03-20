
#include "capture_texture.h"
#include "mfutility.h"

#define OUTPUT_FRAME_WIDTH 640				// Adjust if the webcam does not support this frame width.
#define OUTPUT_FRAME_HEIGHT 480				// Adjust if the webcam does not support this frame height.
#define OUTPUT_FRAME_RATE 30					// Adjust if the webcam does not support this frame rate.

#define MODE_YUY2 (0)

CaptureTexture::CaptureTexture()
  : m_internalData(nullptr)
{
}

CaptureTexture::~CaptureTexture()
{
}

// -------------------------------------------
struct CaptureTexture::InternalData
{
  IMFMediaSource* pVideoSource = NULL, *pAudioSource = NULL;
  IMFMediaSource* pAggSource = NULL;
  IMFCollection* pCollection = NULL;
  IMFAttributes* videoConfig = NULL;
  IMFSourceReader* pSourceReader = NULL;
  WCHAR* webcamFriendlyName;
  IMFMediaType* pVideoSrcOut = NULL, *pAudioSrcOut = NULL;
  IMFSinkWriter* pWriter = NULL;
  IMFMediaType* pVideoSinkOut = NULL, *pAudioSinkOut = NULL;
  DWORD srcVideoStreamIndex = 0, srcAudioStreamIndex = 0;
  DWORD sinkVideoStreamIndex = 0, sinkAudioStreamIndex = 0;

  DWORD stmIndex = 0;
  BOOL isSelected = false;
  IMFMediaType* pStmMediaType = NULL;

  DWORD streamIndex, flags;
  LONGLONG llSampleTimeStamp;
  IMFSample* pSample = NULL;
  LONGLONG llVideoBaseTime = 0, llAudioBaseTime = 0;
  int sampleCount = 0;

  Render::Texture* m_targetTexture = nullptr;
  uint32_t width = OUTPUT_FRAME_WIDTH;
  uint32_t height = OUTPUT_FRAME_HEIGHT;
  bool finished = false;

public:
  ~InternalData()
  {
    SAFE_RELEASE(pAggSource);
    SAFE_RELEASE(pVideoSource);
    SAFE_RELEASE(pAudioSource);
    SAFE_RELEASE(videoConfig);
    SAFE_RELEASE(pSourceReader);
    SAFE_RELEASE(pVideoSrcOut);
    SAFE_RELEASE(pAudioSrcOut);
    SAFE_RELEASE(pVideoSinkOut);
    SAFE_RELEASE(pAudioSinkOut);
    SAFE_RELEASE(pWriter);
  }

  bool open(const uint32_t videoDeviceIndex, const uint32_t audioDeviceIndex)
  {
    HRESULT hr = S_OK;
    finished = false;

    // Get the sources for the video and audio capture devices.
    CHECK_HR(GetSourceFromCaptureDevice(DeviceType::Video, videoDeviceIndex, &pVideoSource, nullptr),
      L"Failed to get video source and reader.");

    CHECK_HR(GetSourceFromCaptureDevice(DeviceType::Audio, audioDeviceIndex, &pAudioSource, nullptr),
      L"Failed to get video source and reader.");

    // Combine the two into an aggregate source and create a reader.
    CHECK_HR(MFCreateCollection(&pCollection), L"Failed to create source collection.");
    CHECK_HR(pCollection->AddElement(pVideoSource), L"Failed to add video source to collection.");
    CHECK_HR(pCollection->AddElement(pAudioSource), L"Failed to add audio source to collection.");

    CHECK_HR(MFCreateAggregateSource(pCollection, &pAggSource), L"Failed to create aggregate source.");

    CHECK_HR(MFCreateSourceReaderFromMediaSource(pAggSource, NULL, &pSourceReader), L"Error creating media source reader.");

    // Note the webcam needs to support this media type. 
    CHECK_HR(MFCreateMediaType(&pVideoSrcOut), L"Failed to create media type.");
    CHECK_HR(pVideoSrcOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), L"Failed to set major video type.");
#if MODE_YUY2
    CHECK_HR(pVideoSrcOut->SetGUID(MF_MT_SUBTYPE, WMMEDIASUBTYPE_YUY2), L"Failed to set video sub type to YUY2.");
#else
    CHECK_HR(pVideoSrcOut->SetGUID(MF_MT_SUBTYPE, WMMEDIASUBTYPE_I420), L"Failed to set video sub type to I420.");
#endif
    CHECK_HR(MFSetAttributeRatio(pVideoSrcOut, MF_MT_FRAME_RATE, OUTPUT_FRAME_RATE, 1), L"Failed to set frame rate on source reader out type.");
    CHECK_HR(MFSetAttributeSize(pVideoSrcOut, MF_MT_FRAME_SIZE, OUTPUT_FRAME_WIDTH, OUTPUT_FRAME_HEIGHT), L"Failed to set frame size.");

    CHECK_HR(pSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pVideoSrcOut), L"Failed to set video media type on source reader.");

    CHECK_HR(MFCreateMediaType(&pAudioSrcOut), L"Failed to create media type.");
    CHECK_HR(pAudioSrcOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio), L"Failed to set major audio type.");
    CHECK_HR(pAudioSrcOut->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float), L"Failed to set audio sub type.");

    CHECK_HR(pSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pAudioSrcOut), L"Failed to set audio media type on source reader.");

    while (pSourceReader->GetStreamSelection(stmIndex, &isSelected) == S_OK)
    {
      printf("Stream %d is selected %d.\n", stmIndex, isSelected);

      CHECK_HR(pSourceReader->GetCurrentMediaType(stmIndex, &pStmMediaType), L"Failed to get media type for selected stream.");
      std::cout << "Media type: " << GetMediaTypeDescription(pStmMediaType) << std::endl;

      GUID majorMediaType;
      pStmMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorMediaType);
      if (majorMediaType == MFMediaType_Audio)
      {
        std::cout << "Source audio stream index is " << stmIndex << "." << std::endl;
        srcAudioStreamIndex = stmIndex;
      }
      else if (majorMediaType == MFMediaType_Video)
      {
        std::cout << "Video stream index is " << stmIndex << "." << std::endl;
        srcVideoStreamIndex = stmIndex;
      }

      stmIndex++;
      SAFE_RELEASE(pStmMediaType);
    }

    if (m_targetTexture)
    {
      m_targetTexture->destroy();
    }

    // Just to ensure we have something as the first frame, and also ensure we have the real required size (height % 16 should be 0)
    update(0.0f);

    return true;
  }

  void update(float dt)
  {
    if (finished)
    {
      return;
    }

    assert(pSourceReader);
    flags = 0;

    HRESULT hr;
    hr = pSourceReader->ReadSample(
      MF_SOURCE_READER_ANY_STREAM,
      0,																	// Flags.
      &streamIndex,												// Receives the actual stream index. 
      &flags,															// Receives status flags.
      &llSampleTimeStamp,									// Receives the time stamp.
      &pSample												// Receives the sample or NULL.
      );

    if (hr != S_OK)
    {
      finished = true;
      return;
    }

    DWORD sinkStmIndex = (streamIndex == srcAudioStreamIndex) ? sinkAudioStreamIndex : sinkVideoStreamIndex;

    if (sinkStmIndex == sinkAudioStreamIndex )
    {
      if (llAudioBaseTime == 0)
      {
        llAudioBaseTime = llSampleTimeStamp;
      }
      else
      {
        // Re-base the time stamp.
        llSampleTimeStamp -= llAudioBaseTime;
      }
    }

    if (sinkStmIndex == sinkVideoStreamIndex)
    {
      if (llVideoBaseTime == 0)
      {
        llVideoBaseTime = llSampleTimeStamp;
      }
      else
      {
        // Re-base the time stamp.
        llSampleTimeStamp -= llVideoBaseTime;
      }
    }

    if (streamIndex == srcAudioStreamIndex)
    {
      return;
    }

    if (pSample)
    {
      hr = pSample->SetSampleTime(llSampleTimeStamp);
      assert(hr == S_OK);

      IMFMediaBuffer* buf = NULL;
      DWORD bufLength;

      hr = pSample->ConvertToContiguousBuffer(&buf);
      hr = buf->GetCurrentLength(&bufLength);

      byte* byteBuffer = NULL;
      DWORD buffMaxLen = 0, buffCurrLen = 0;
      hr = buf->Lock(&byteBuffer, &buffMaxLen, &buffCurrLen);
      assert(hr == S_OK);

      // Some videos report one resolution and after the first frame change the height to the next multiple of 16 (using the event MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
#if MODE_YUY2
      if (!m_targetTexture)
      {
        m_targetTexture = new Render::Texture();
        int texture_width = width * 2;
        if (!m_targetTexture->create(texture_width, height, DXGI_FORMAT_R8_UNORM, true))
        {
          return;
        }
      }
      m_targetTexture->updateFromYUY2(byteBuffer, buffCurrLen);
#else
      if (!m_targetTexture)
      {
        m_targetTexture = new Render::Texture();
        int texture_height = height * 2;
        if (!m_targetTexture->create(width, texture_height, DXGI_FORMAT_R8_UNORM, true))
        {
          return;
        }

      }
      m_targetTexture->updateFromIYUV(byteBuffer, buffCurrLen);
#endif

      //dbg(L"sample %d, source stream index %d, sink stream index %d, timestamp %I64d.\n", sampleCount, streamIndex, sinkStmIndex, llSampleTimeStamp);
      
      SAFE_RELEASE(buf);
      sampleCount++;
    }
    SAFE_RELEASE(pSample);
  }

};

bool CaptureTexture::createAPI()
{
  HRESULT hr = MFStartup(MF_VERSION);
  return(hr == S_OK);
}

void CaptureTexture::destroyAPI()
{
  MFShutdown();
}

bool CaptureTexture::create(const uint32_t videoDeviceIndex, const uint32_t audioDeviceIndex)
{
  assert(!m_internalData);
  m_internalData = new InternalData();
  return m_internalData->open(videoDeviceIndex, audioDeviceIndex);
}

void CaptureTexture::destroy()
{
  if (m_internalData)
  {
    delete m_internalData;
  }
  m_internalData = nullptr;
}

bool CaptureTexture::update(float dt)
{
  m_internalData->update(dt);
  return m_internalData->finished;
}

Render::Texture* CaptureTexture::getTexture()
{
  assert(m_internalData);
  return m_internalData->m_targetTexture;
}
