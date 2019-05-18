#include <cstdio>
#include <cassert>
#include "SDL.h"

auto const WIDTH = 1024;
auto const HEIGHT = 768;

struct Input
{
  bool quit, restart;

  bool left, right, up, down;
  bool boost;
};

void processInput(Input& input)
{
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    if(event.type == SDL_QUIT)
    {
      input.quit = true;
      return;
    }

    if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
      bool isPressed = event.type == SDL_KEYDOWN;
      switch(event.key.keysym.scancode)
      {
      case SDL_SCANCODE_ESCAPE:
        {
          input.quit = true;
          return;
        }
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
    else if(event.type == SDL_JOYAXISMOTION)
    {
      if(event.jaxis.axis == 0) // horizontal
      {
        input.left = event.jaxis.value < 0;
        input.right = event.jaxis.value > 0;
      }
      else if(event.jaxis.axis == 1) // vertical
      {
        input.up = event.jaxis.value < 0;
        input.down = event.jaxis.value > 0;
      }
    }
    else if(event.type == SDL_JOYBUTTONDOWN)
    {
      input.restart = true;
    }
    else if(event.type == SDL_JOYBUTTONUP)
    {
      input.restart = false;
    }
    else
    {
      printf("Unknown event: %d\n", event.type);
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
  bool alive = true;
  int x, y;
  Direction direction;
};

Bike g_bike;
char g_board[HEIGHT][WIDTH];

void initGame()
{
  g_bike = {};
  g_bike.x = WIDTH/2;
  g_bike.y = HEIGHT/2;
  memset(g_board, 0, sizeof g_board);
}

void updateGame(Input input)
{
  if(!g_bike.alive)
  {
    if(input.restart)
      initGame();
    return;
  }

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

  int dx = dirs[(int)g_bike.direction][0];
  int dy = dirs[(int)g_bike.direction][1];

  g_bike.x += dx * speed;
  g_bike.y += dy * speed;

  g_bike.x = (g_bike.x + WIDTH)%WIDTH;
  g_bike.y = (g_bike.y + HEIGHT)%HEIGHT;

  if(dx || dy)
    if(g_board[g_bike.y][g_bike.x])
      g_bike.alive = false;

  g_board[g_bike.y][g_bike.x] = 1;
}

void drawScreen(SDL_Renderer* renderer)
{
  /* Select the color for drawing. It is set to red here. */
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  /* Clear the entire screen to our selected color. */
  SDL_RenderClear(renderer);

  for(int row=0;row < HEIGHT;++row)
  {
    for(int col=0;col < WIDTH;++col)
    {
      int c = g_board[row][col] ? 255 : 0;
      if(c)
      {
        SDL_SetRenderDrawColor(renderer, c, c, c, 255);
        SDL_RenderDrawPoint(renderer, col, row);
      }
    }
  }

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

  printf("Detected %d joysticks\n", SDL_NumJoysticks());

  auto joy = SDL_JoystickOpen(0);

  if(joy)
  {
    printf("Opened Joystick 0\n");
    printf("Name: %s\n", SDL_JoystickNameForIndex(0));
    printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joy));
    printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joy));
    printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joy));
  }
  else
  {
    printf("Couldn't open Joystick 0\n");
  }

  initGame();

  Input input{};

  while(1)
  {
    processInput(input);
    if(input.quit)
      break;

    updateGame(input);
    drawScreen(renderer);
  }

  SDL_JoystickClose(joy);
  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}

