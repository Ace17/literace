#pragma once

#include <vector>

struct GameInput;
struct IScene;

struct ISceneFactory
{
  virtual IScene* createPlayingScene() = 0;
  virtual IScene* createScoresScene(std::vector<int> scores) = 0;
};

struct IScene
{
  ISceneFactory* factory = nullptr;
  virtual IScene* update(GameInput input) = 0;
  virtual void draw(int* pixels) = 0;
};

