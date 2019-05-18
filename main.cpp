#include <cstdio>
#include <cassert>
#include "SDL.h"

auto const WIDTH = 800;
auto const HEIGHT = 600;

int x = 128;
int y = 128;

struct Input
{
  bool quit;
  bool left, right, up, down;
  bool boost;
};

void processInput(Input& input)
{
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    if(event.type == SDL_QUIT)
      input.quit = true;

    if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
      bool isPressed = event.type == SDL_KEYDOWN;
      switch(event.key.keysym.scancode)
      {
      case SDL_SCANCODE_ESCAPE:
        input.quit = true;
      case SDL_SCANCODE_LSHIFT:
        input.boost = isPressed;
        break;
      case SDL_SCANCODE_LEFT:
        input.left = isPressed;
        break;
      case SDL_SCANCODE_RIGHT:
        input.right = isPressed;
        break;
      case SDL_SCANCODE_UP:
        input.up = isPressed;
        break;
      case SDL_SCANCODE_DOWN:
        input.down = isPressed;
        break;
      }
    }
  }
}

enum class Direction
{
  Idle,
  Left,
  Down,
  Right,
  Up,
};

static const int dirs[][2] =
{
  {0, 0},
  {-1, 0},
  {0, 1},
  {1, 0},
  {0, -1},
};

bool isOpposed(Direction a, Direction b)
{
  int a_dx = dirs[(int)a][0];
  int a_dy = dirs[(int)a][1];
  int b_dx = dirs[(int)b][0];
  int b_dy = dirs[(int)b][1];

  return a_dx == -b_dx && a_dy == -b_dy;
}

struct Bike
{
  int x, y;
  Direction direction;
};

Bike g_bike;

void updateGame(Input input)
{
  Direction wantedDirection = g_bike.direction;

  if(input.left)
    wantedDirection = Direction::Left;
  if(input.right)
    wantedDirection = Direction::Right;
  if(input.up)
    wantedDirection = Direction::Up;
  if(input.down)
    wantedDirection = Direction::Down;

  if(!isOpposed(g_bike.direction, wantedDirection))
    g_bike.direction = wantedDirection;

  int speed = 1;

  if(input.boost)
    speed = 2;

  x += dirs[(int)g_bike.direction][0] * speed;
  y += dirs[(int)g_bike.direction][1] * speed;

  x = (x + WIDTH)%WIDTH;
  y = (y + HEIGHT)%HEIGHT;
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

  auto window = SDL_CreateWindow("Literace", 0, 0, WIDTH, HEIGHT, 0);
  assert(window);

  /* We must call SDL_CreateRenderer in order for draw calls to affect this window. */
  auto renderer = SDL_CreateRenderer(window, -1, 0);
  assert(renderer);

  Input input{};

  while(1)
  {
    processInput(input);
    if(input.quit)
      break;

    updateGame(input);
    drawScreen(renderer);
    SDL_Delay(10);
  }

  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}

