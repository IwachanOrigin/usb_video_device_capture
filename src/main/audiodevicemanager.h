
#ifndef AUDIO_DEVICE_MANAGER_H_
#define AUDIO_DEVICE_MANAGER_H_

extern "C"
{
#include <SDL.h>
}

class AudioDeviceManager
{
public:
  static AudioDeviceManager& getInstance();

  int init(const int audio_device_index);
  int start();
  bool render(const uint8_t* new_data, size_t data_size);
  int stop();

  SDL_AudioDeviceID getAudioID() { return m_deviceID; }
  bool getStatus() { return m_status; }

private:
  explicit AudioDeviceManager();
  ~AudioDeviceManager();
  explicit AudioDeviceManager(const AudioDeviceManager &);
  AudioDeviceManager &operator=(const AudioDeviceManager &);

  SDL_AudioDeviceID m_deviceID;
  bool m_status;
};

#endif // AUDIO_DEVICE_MANAGER_H_
