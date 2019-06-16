// "Terminal" side.
// Depends on game logic.
#include <cstdio>
#include <cassert>
#include <vector>
#include "SDL.h"
#include "input.h"
#include "game.h"

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
  void onKilled(int victim, int killer) override
  {
    printf("Bike %d was killed by %d\n", victim, killer);
  }
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

