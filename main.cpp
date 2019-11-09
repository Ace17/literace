// "Terminal" side.
#include <cstdio>
#include <vector>
#include <algorithm>
#include "SDL.h"
#include "display.h"
#include "input.h"
#include "game.h"

using namespace std;

struct Terminal : ITerminal
{
  void drawHead(Vec2 pos, int colorIndex) override
  {
    for(int j = -2; j <= 2; ++j)
      for(int k = -2; k <= 2; ++k)
        putPixel((int*)pixels, pos.x - k, pos.y - j, darken(getColor(colorIndex)));
  }

  static int darken(int color)
  {
    int r = (color >> 16) & 0xff;
    int g = (color >> 8) & 0xff;
    int b = (color >> 0) & 0xff;
    return mkColor(r / 2, g / 2, b / 2);
  }

  static void putPixel(int* pixels, int x, int y, int color)
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

void drawScreen(IDisplay* display, Game& game)
{
  static Terminal terminal;
  game.terminal = &terminal;
  drawGame(game, (int*)terminal.pixels);
  display->refresh(terminal.pixels);
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

