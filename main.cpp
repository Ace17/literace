// "Terminal" side.
// Depends on game logic.
#include <cstdio>
#include <cassert>
#include <vector>
#include "SDL.h"
#include "input.h"
#include "game.h"

Game g_game;

Uint32 mkColor(int r, int g, int b)
{
  Uint32 color = 0;
  color |= 0xff;
  color <<= 8;
  color |= r;
  color <<= 8;
  color |= g;
  color <<= 8;
  color |= b;
  return color;
}

void drawScreen(SDL_Renderer* renderer, SDL_Texture* texture)
{
  static const Uint32 colors[] =
  {
    mkColor(40, 40, 40),
    mkColor(255, 255, 0),
    mkColor(0, 0, 255),
    mkColor(255, 0, 0),
    mkColor(0, 255, 0),
  };

  static Uint32 pixels[BOARD_WIDTH * BOARD_HEIGHT];

  for(int row = 0; row < BOARD_HEIGHT; ++row)
  {
    for(int col = 0; col < BOARD_WIDTH; ++col)
    {
      int c = g_game.board[row * BOARD_WIDTH + col];
      pixels[row * BOARD_WIDTH + col] = colors[c % 5];
    }
  }

  SDL_UpdateTexture(texture, NULL, pixels, BOARD_WIDTH * sizeof(Uint32));
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  auto window = SDL_CreateWindow("Literace", 0, 0, BOARD_WIDTH, BOARD_HEIGHT, 0);
  assert(window);

  auto renderer = SDL_CreateRenderer(window, -1, 0);
  assert(renderer);

  auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, BOARD_WIDTH, BOARD_HEIGHT);
  assert(texture);

  initGame(g_game);

  GameInput input {};

  while(1)
  {
    processInput(input);

    if(input.quit)
      break;

    updateGame(g_game, input);
    drawScreen(renderer, texture);
  }

  destroyInput();

  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}

