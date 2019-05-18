#include <cstdio>
#include <cassert>
#include "SDL.h"

int x = 128;
int y = 128;

bool left, right, up, down;

bool processInput()
{
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    if(event.type == SDL_QUIT)
      return false;

    if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
      bool isPressed = event.type == SDL_KEYDOWN;
      switch(event.key.keysym.scancode)
      {
      case SDL_SCANCODE_ESCAPE:
        return false;
      case SDL_SCANCODE_LEFT:
        left = isPressed;
        break;
      case SDL_SCANCODE_RIGHT:
        right = isPressed;
        break;
      case SDL_SCANCODE_UP:
        up = isPressed;
        break;
      case SDL_SCANCODE_DOWN:
        down = isPressed;
        break;
      }
    }
  }

  return true;
}

void updateGame()
{
  if(left)
    --x;
  if(right)
    ++x;
  if(up)
    --y;
  if(down)
    ++y;
}

void drawScreen(SDL_Renderer* renderer)
{
  /* Select the color for drawing. It is set to red here. */
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  /* Clear the entire screen to our selected color. */
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
  SDL_RenderDrawPoint(renderer, x, y);

  /* Up until now everything was drawn behind the scenes.
     This will show the new, red contents of the window. */
  SDL_RenderPresent(renderer);
}

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  auto window = SDL_CreateWindow("Literace", 0, 0, 800, 600, 0);
  assert(window);

  /* We must call SDL_CreateRenderer in order for draw calls to affect this window. */
  auto renderer = SDL_CreateRenderer(window, -1, 0);
  assert(renderer);

  while(processInput())
  {
    updateGame();
    drawScreen(renderer);
    SDL_Delay(10);
  }

  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}

