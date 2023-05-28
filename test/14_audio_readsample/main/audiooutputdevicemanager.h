
#ifndef AUDIO_OUTPUT_DEVICE_MANAGER_H_
#define AUDIO_OUTPUT_DEVICE_MANAGER_H_

extern "C"
{
#include <SDL.h>
}

class AudioOutputDeviceManager
{
public:
  static AudioOutputDeviceManager& getInstance();

  int init(const int audio_device_index);
  int start();
  bool render(const uint8_t* new_data, size_t data_size);
  int stop();

  SDL_AudioDeviceID getAudioID() { return m_deviceID; }
  bool getStatus() { return m_status; }

private:
  explicit AudioOutputDeviceManager();
  ~AudioOutputDeviceManager();
  explicit AudioOutputDeviceManager(const AudioOutputDeviceManager &);
  AudioOutputDeviceManager &operator=(const AudioOutputDeviceManager &);

  SDL_AudioDeviceID m_deviceID;
  bool m_status;
};

#endif // AUDIO_OUTPUT_DEVICE_MANAGER_H_
