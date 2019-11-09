// "Terminal" side.
#include <cstdio>
#include <vector>
#include <algorithm>
#include "SDL.h"
#include "display.h"
#include "input.h"
#include "game.h"

using namespace std;

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

void drawScreen(IDisplay* display, Game& game)
{
  static Uint32 pixels[BOARD_WIDTH * BOARD_HEIGHT];

  drawGame(game, (int*)pixels);
  display->refresh(pixels);
}

static auto const TIMESTEP_MS = 1;

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  auto display = createDisplay(BOARD_WIDTH, BOARD_HEIGHT);

  Game g_game;

  Match match;
  g_game.sink = &match;
  initGame(g_game);

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

      updateGame(g_game, input);
    }

    drawScreen(display.get(), g_game);
  }

  destroyInput();

  SDL_Quit();
  return 0;
}

