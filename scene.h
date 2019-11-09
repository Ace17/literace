#pragma once

struct GameInput;

enum class SceneId
{
  Playing,
  Scores
};

struct IScene
{
  virtual SceneId update(GameInput input) = 0;
  virtual void draw(int* pixels) = 0;
};

