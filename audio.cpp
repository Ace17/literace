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
        m_lfophase += 10.0 / 48000.0;
        if(m_lfophase > 1.0)
          m_lfophase -= 1.0;

        auto freq = m_sineFreq + mySin(m_lfophase) * 20;

        m_phase1 += freq / 48000.0;
        if(m_phase1 > 1.0)
          m_phase1 -= 1.0;

        m_phase2 += freq * 1.5 / 48000.0;
        if(m_phase2 > 1.0)
          m_phase2 -= 1.0;

        m_env *= 0.9999;

        double osc = 0.0;

        osc += mySquare(m_phase1);
        osc += mySquare(m_phase2);

        samples[i] = osc * m_env * 0.3;
      }
    }

    static double mySin(double t)
    {
      return sin(t * 2 * M_PI);
    }

    static double mySquare(double t)
    {
      return t < 0.5 ? -1 : 1;
    }

    double m_phase1 = 0;
    double m_phase2 = 0;
    double m_lfophase = 0;
    double m_sineFreq = 440.0;
    double m_env = 0;
  };
}

std::unique_ptr<IAudio> createAudio()
{
  return std::make_unique<Audio>();
}

