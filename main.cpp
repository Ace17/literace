// "Terminal" side.
// Depends on game logic.
#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>
#include "SDL.h"
#include "input.h"
#include "game.h"

using namespace std;

Game g_game;

void drawScreen(SDL_Renderer* renderer, SDL_Texture* texture)
{
  static Uint32 pixels[BOARD_WIDTH * BOARD_HEIGHT];

  drawGame(g_game, (int*)pixels);

  SDL_UpdateTexture(texture, NULL, pixels, BOARD_WIDTH * sizeof(Uint32));
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

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

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  auto window = SDL_CreateWindow("Literace", 0, 0, BOARD_WIDTH, BOARD_HEIGHT, 0);
  assert(window);

  auto renderer = SDL_CreateRenderer(window, -1, 0);
  assert(renderer);

  auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, BOARD_WIDTH, BOARD_HEIGHT);
  assert(texture);

  Match match;
  g_game.sink = &match;
  initGame(g_game);

  while(1)
  {
    auto input = processInput();

    if(input.quit)
      break;

    updateGame(g_game, input);
    drawScreen(renderer, texture);

    SDL_Delay(1);
  }

  destroyInput();

  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}

