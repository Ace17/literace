#include "audio.h"
#include "assert.h"

#include "SDL.h"
#include <stdexcept>
#include <string>

namespace
{
  struct Audio : IAudio
  {
    Audio()
    {
      SDL_AudioSpec spec {};
      spec.freq = 48000;
      spec.channels = 1;
      spec.format = AUDIO_F32;
      spec.samples = 1024;
      spec.callback = &Audio::staticMixAudio;
      spec.userdata = this;

      SDL_AudioSpec realSpec {};
      if(SDL_OpenAudio(&spec, &realSpec))
        throw std::runtime_error(std::string("Can't open audio: ") + SDL_GetError());

      printf("[audio] %d Hz\n", realSpec.freq);
      assert(realSpec.format == AUDIO_F32);

      SDL_PauseAudio(0);
    }

    ~Audio()
    {
      SDL_CloseAudio();
    }

    void beep() override
    {
      m_env = 1.0;
    }

    static void staticMixAudio(void* user, Uint8* samples, int len)
    {
      auto pThis = (Audio*)user;
      pThis->mixAudio((float*)samples, len/sizeof(float));
    }

    void mixAudio(float* samples, int sampleCount)
    {
      for(int i=0;i < sampleCount;++i)
      {
        samples[i] = sin(m_time * m_sineFreq * 2 * M_PI) * m_env * 0.3;
        m_time += 1.0 / 48000.0;
        m_env *= 0.9999;
      }
    }

    double m_time = 0;
    double m_sineFreq = 440.0;
    double m_env = 0;
  };
}

std::unique_ptr<IAudio> createAudio()
{
  return std::make_unique<Audio>();
}

