
#include <string>
#include <windows.h>
#include "audiodevicemanager.h"

AudioDeviceManager::AudioDeviceManager()
  : m_deviceID(0)
  , m_status(false)
{
}

AudioDeviceManager::~AudioDeviceManager()
{
	if (m_deviceID > 0)
	{
		SDL_CloseAudioDevice(m_deviceID);
	}
	SDL_Quit();
}

AudioDeviceManager& AudioDeviceManager::getInstance()
{
  static AudioDeviceManager inst;
  return inst;
}

int AudioDeviceManager::init(const int audio_device_index)
{
  if (audio_device_index < 0)
  {
    return -1;
  }
	SDL_Init(SDL_INIT_AUDIO);

  SDL_AudioSpec wants{};
  SDL_AudioSpec spec{};

  SDL_memset(&wants, '\0', sizeof(wants));

  wants.freq = 48000;
  wants.format = AUDIO_S16SYS;
  wants.channels = 2;
  wants.samples = 4096;

  const char* audioDeviceName = SDL_GetAudioDeviceName(audio_device_index, 0);
  m_deviceID = SDL_OpenAudioDevice(audioDeviceName, false, &wants, &spec, 0);

  if (m_deviceID <= 0)
  {
    m_status = false;
    delete audioDeviceName;
    audioDeviceName = nullptr;
    return -1;
  }
  m_status = true;
  return 0;
}

int AudioDeviceManager::start()
{
  if (m_deviceID <= 0)
  {
    return -1;
  }

  auto sizeAudioQueue = SDL_GetQueuedAudioSize(m_deviceID);
  if (sizeAudioQueue > 0)
  {
    SDL_ClearQueuedAudio(m_deviceID);
  }
  SDL_PauseAudioDevice(m_deviceID, 0);

  return 0;
}

bool AudioDeviceManager::render(const uint8_t* new_data, size_t data_size)
{
  // Returns 0 on success or a negative error code on failure.
  int result = SDL_QueueAudio(m_deviceID, new_data, (uint32_t)data_size);
  return result == 0 ? true : false;
}

int AudioDeviceManager::stop()
{
  if (m_deviceID <= 0)
  {
    return -1;
  }

  auto sizeAudioQueue = SDL_GetQueuedAudioSize(m_deviceID);
  if (sizeAudioQueue > 0)
  {
    SDL_ClearQueuedAudio(m_deviceID);
  }
  SDL_PauseAudioDevice(m_deviceID, 1);

  return 0;
}
