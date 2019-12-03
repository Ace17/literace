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
  PlayingScene(Terminal* terminal_, Match* match_) : m_match(match_)
  {
    m_game = createGame(terminal_, match_);
  }

  IScene* update(GameInput input) override
  {
    int ret = m_game->update(input);
    std::vector<int> scores;

    for(auto& score : m_match->kills)
      scores.push_back(score);

    return ret ? factory->createScoresScene(scores) : nullptr;
  }

  void draw(int* pixels) override
  {
    m_game->draw(pixels);
  }

  Match* const m_match;
  std::unique_ptr<IGame> m_game;
};

struct ScoreScene : IScene
{
  int timer = 1000;

  ScoreScene(ITerminal* terminal_, std::vector<int> scores_) : terminal(terminal_), scores(scores_)
  {
  }

  IScene* update(GameInput input) override
  {
    timer--;

    if(timer > 0)
      return nullptr;

    return factory->createPlayingScene();
  }

  void draw(int* pixels) override
  {
    memset(pixels, 0, BOARD_WIDTH * BOARD_HEIGHT * sizeof(int));

    for(int i = 0; i < scores.size(); ++i)
    {
      for(int k = 0; k < scores[i]; ++k)
        terminal->drawHead(Vec2{ 50 + k * 25, 50 + i * 25 }, i + 1);
    }
  }

  ITerminal* const terminal;
  const std::vector<int> scores;
};

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  auto display = createDisplay(BOARD_WIDTH, BOARD_HEIGHT);

  struct App : ISceneFactory
  {
    Terminal terminal;
    Match match;

    IScene* createPlayingScene() override
    {
      return withFactory(new PlayingScene(&terminal, &match));
    }

    IScene* createScoresScene(std::vector<int> scores)
    {
      return withFactory(new ScoreScene(&terminal, scores));
    }

    IScene* withFactory(IScene* s)
    {
      s->factory = this;
      return s;
    }
  };

  App app;

  std::unique_ptr<IScene> scene(app.createPlayingScene());

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

      auto newScene = scene->update(input);

      if(newScene)
      {
        scene.reset(newScene);
        printf("New scene\n");
      }
    }

    scene->draw((int*)app.terminal.pixels);

    // drawScreen
    display->refresh(app.terminal.pixels);
  }

  destroyInput();

  SDL_Quit();
  return 0;
}

