#pragma once

#include <memory>
struct IAudio
{
  virtual ~IAudio() = default;
  virtual void beep() = 0;
};

std::unique_ptr<IAudio> createAudio();
