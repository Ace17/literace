// "Terminal" side.
#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>
#include "SDL.h"
#include "display.h"
#include "input.h"
#include "game.h"
#include "scene.h"

using namespace std;

struct Terminal : ITerminal
{
  void drawHead(Vec2 pos, int colorIndex) override
  {
    for(int j = -2; j <= 2; ++j)
      for(int k = -2; k <= 2; ++k)
        putPixel(pixels, pos.x - k, pos.y - j, darken(getColor(colorIndex)));
  }

  void drawObstacle(Vec2 pos, Vec2 size) override
  {
    for(int y = 0; y < size.y; ++y)
      for(int x = 0; x < size.x; ++x)
        putPixel(pixels, pos.x + x, pos.y + y, -1);
  }

  static int darken(int color)
  {
    int r = (color >> 16) & 0xff;
    int g = (color >> 8) & 0xff;
    int b = (color >> 0) & 0xff;
    return mkColor(r / 2, g / 2, b / 2);
  }

  static void putPixel(Uint32* pixels, int x, int y, int color)
  {
    x = (x + BOARD_WIDTH) % BOARD_WIDTH;
    y = (y + BOARD_HEIGHT) % BOARD_HEIGHT;

    pixels[y * BOARD_WIDTH + x] = color;
  }

  Uint32 pixels[BOARD_WIDTH * BOARD_HEIGHT];
};

struct Match : IEventSink
{
  void onRoundFinished() override
  {
    printf("Round finished: ");

    for(auto killCount : kills)
      printf(" %d", killCount);

    printf("\n");
  }

  void onKilled(int frameCount, int victim, int killer) override
  {
    if(victim == killer)
    {
      printf("Bike %d committed suicide (lifetime=%d)\n", victim, frameCount);
      kills[victim - 1] = max(kills[victim - 1] - 1, 0);
    }
    else
    {
      printf("Bike %d was killed by %d (lifetime=%d)\n", victim, killer, frameCount);
      kills[killer - 1]++;
    }
  }

  void onTurn(int frameCount, int bike) override
  {
    if(0)
      printf("bike %d turned\n", bike);
  }

  void onCrash(int frameCount, vector<int> victims) override
  {
    printf("crash! victims:");

    for(auto victim : victims)
      printf(" %d", victim);

    printf("\n");
  }

  int kills[MAX_PLAYERS] {};
};

static auto const TIMESTEP_MS = 1;

struct PlayingScene : IScene
{
  PlayingScene(Terminal* terminal_, Match* match_)
  {
    m_game.terminal = terminal_;
    m_game.sink = match_;
    initGame(m_game);
  }

  SceneId update(GameInput input) override
  {
    int ret = updateGame(m_game, input);
    return ret ? SceneId::Scores : SceneId::Playing;
  }

  void draw(int* pixels) override
  {
    drawGame(m_game, pixels);
  }

  Game m_game {};
};

struct ScoreScene : IScene
{
  int timer = 1000;

  SceneId update(GameInput input) override
  {
    timer --;
    if(timer < 0)
      return SceneId::Playing;
    else
      return SceneId::Scores;
  }

  void draw(int* pixels) override 
  {
    memset(pixels, 0x77, BOARD_WIDTH * BOARD_HEIGHT * sizeof(int));
  }
};

std::unique_ptr<IScene> createScene(SceneId id, Terminal* terminal, Match* match)
{
  switch(id)
  {
  case SceneId::Playing:
    return std::make_unique<PlayingScene>(terminal, match);
  case SceneId::Scores:
    return std::make_unique<ScoreScene>();
  }
  assert(0);
  return nullptr;
}

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  auto display = createDisplay(BOARD_WIDTH, BOARD_HEIGHT);

  Terminal terminal;
  Match match;

  SceneId sceneId = SceneId::Playing;
  std::unique_ptr<IScene> scene = createScene(sceneId, &terminal, &match);

  int64_t prev = SDL_GetTicks();
  int64_t timeAccumulator = 0;

  bool keepGoing = true;

  while(keepGoing)
  {
    auto now = SDL_GetTicks();
    timeAccumulator += now - prev;
    prev = now;

    while(timeAccumulator > 0)
    {
      timeAccumulator -= TIMESTEP_MS;
      auto input = processInput();

      if(input.quit)
      {
        keepGoing = false;
        break;
      }

      auto nextSceneId = scene->update(input);
      if(sceneId != nextSceneId)
      {
        printf("New scene: %d -> %d\n", (int)sceneId, (int)nextSceneId);
        scene = createScene(nextSceneId, &terminal, &match);
        sceneId = nextSceneId;
      }
    }

    scene->draw((int*)terminal.pixels);

    // drawScreen
    display->refresh(terminal.pixels);
  }

  destroyInput();

  SDL_Quit();
  return 0;
}

