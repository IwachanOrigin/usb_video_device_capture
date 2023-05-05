
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
		SDL_CloseAudioDeviceManager(m_deviceID);
	}
	SDL_Quit();
}

int AudioDeviceManager::init(const int audio_device_index)
{
  if (audio_device_index < 0)
  {
    return -1;
  }
	SDL_Init(SDL_INIT_AUDIO);

  SDL_AudioSpec wants;
  SDL_AudioSpec spec;

  SDL_memset(&wants, '\0', sizeof(wants));

  wants.freq = 48000;
  wants.format = AUDIO_S16SYS;
  wants.channels = 2;
  wants.samples = 4096;

  const char* audioDeviceName = SDL_GetAudioDeviceManagerName(audio_device_index, 0);
  m_deviceID = SDL_OpenAudioDeviceManager(audioDeviceName, false, &wants, &spec, 0);

  if (m_deviceID <= 0)
  {
    m_status = false;
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
  SDL_PauseAudioDeviceManager(m_deviceID, 0);

  return 0;
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
  SDL_PauseAudioDeviceManager(m_deviceID, 1);

  return 0;
}
