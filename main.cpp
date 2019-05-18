#include <cstdio>
#include <cassert>
#include "SDL.h"

auto const WIDTH = 1024;
auto const HEIGHT = 768;

auto const MAX_PLAYERS = 2;

struct PlayerInput
{
  bool left, right, up, down;
  bool boost;
};

struct Input
{
  bool quit, restart;
  PlayerInput players[MAX_PLAYERS];
};

void processEvent(SDL_Event const& event, Input& input)
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
      input.players[0].boost = isPressed;
      break;
    case SDL_SCANCODE_LEFT:
      input.players[0].left = isPressed;
      break;
    case SDL_SCANCODE_RIGHT:
      input.players[0].right = isPressed;
      break;
    case SDL_SCANCODE_UP:
      input.players[0].up = isPressed;
      break;
    case SDL_SCANCODE_DOWN:
      input.players[0].down = isPressed;
      break;
    }
  }
  else if(event.type == SDL_JOYAXISMOTION)
  {
    auto& info = event.jaxis;

    printf("[%d] axis index: %d\n", info.which, info.axis);

    if(info.axis == 0) // horizontal
    {
      input.players[info.which].left = info.value < -16384;
      input.players[info.which].right = info.value > 16384;
    }
    else if(info.axis == 1) // vertical
    {
      input.players[info.which].up = info.value < -16384;
      input.players[info.which].down = info.value > 16384;
    }
  }
  else if(event.type == SDL_JOYHATMOTION)
  {
    auto& info = event.jhat;

    printf("[%d] hat index: %d\n", info.which, info.hat);

    input.players[info.which].left = info.value == SDL_HAT_LEFT;
    input.players[info.which].right = info.value == SDL_HAT_RIGHT;
    input.players[info.which].up = info.value == SDL_HAT_UP;
    input.players[info.which].down = info.value == SDL_HAT_DOWN;
  }
  else if(event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP)
  {
    bool isPressed = event.type == SDL_JOYBUTTONDOWN;

    auto& info = event.jbutton;

    printf("[%d] %d\n", info.which, info.button);

    if(info.button == 0)
      input.players[info.which].boost = isPressed;
    else
      input.restart = isPressed;
  }
  else
  {
    printf("Unknown event: %d\n", event.type);
  }
}

void processInput(Input& input)
{
  SDL_Event event;

  while(SDL_PollEvent(&event))
    processEvent(event, input);
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
  { 0, 0 },
  { -1, 0 },
  { 0, 1 },
  { 1, 0 },
  { 0, -1 },
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

Bike g_bikes[2];
char g_board[HEIGHT][WIDTH];

void initGame()
{
  int k = 0;

  for(auto& bike : g_bikes)
  {
    bike = {};
    bike.x = (k + 1) * WIDTH / (MAX_PLAYERS + 1);
    bike.y = HEIGHT / 2;

    ++k;
  }

  memset(g_board, 0, sizeof g_board);
}

void updateBike(Bike& bike, PlayerInput input, int team)
{
  Direction wantedDirection = bike.direction;

  if(input.left)
    wantedDirection = Direction::Left;

  if(input.right)
    wantedDirection = Direction::Right;

  if(input.up)
    wantedDirection = Direction::Up;

  if(input.down)
    wantedDirection = Direction::Down;

  if(!isOpposed(bike.direction, wantedDirection))
    bike.direction = wantedDirection;

  int speed = 1;

  if(input.boost)
    speed = 2;

  int dx = dirs[(int)bike.direction][0];
  int dy = dirs[(int)bike.direction][1];

  bike.x += dx * speed;
  bike.y += dy * speed;

  bike.x = (bike.x + WIDTH) % WIDTH;
  bike.y = (bike.y + HEIGHT) % HEIGHT;

  if(dx || dy)
    if(g_board[bike.y][bike.x])
      bike.alive = false;

  g_board[bike.y][bike.x] = 1 + team;
}

bool isGameOver()
{
  for(auto& bike : g_bikes)
    if(!bike.alive)
      return true;

  return false;
}

void updateGame(Input input)
{
  if(isGameOver())
  {
    if(input.restart)
      initGame();

    return;
  }

  for(int i = 0; i < MAX_PLAYERS; ++i)
    updateBike(g_bikes[i], input.players[i], i);
}

void drawScreen(SDL_Renderer* renderer)
{
  SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);

  /* Clear the entire screen to our selected color. */
  SDL_RenderClear(renderer);

  static const int colors[][3] =
  {
    { 255, 255, 0 },
    { 0, 0, 255 },
    { 255, 0, 0 },
    { 0, 255, 0 },
  };

  for(int row = 0; row < HEIGHT; ++row)
  {
    for(int col = 0; col < WIDTH; ++col)
    {
      int c = g_board[row][col];

      if(c)
      {
        int idx = (c - 1) % 4;
        SDL_SetRenderDrawColor(renderer, colors[idx][0], colors[idx][1], colors[idx][2], 255);
        SDL_RenderDrawPoint(renderer, col, row);
      }
    }
  }

  SDL_RenderPresent(renderer);
}

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  auto window = SDL_CreateWindow("Literace", 0, 0, WIDTH, HEIGHT, 0);
  assert(window);

  auto renderer = SDL_CreateRenderer(window, -1, 0);
  assert(renderer);

  printf("Detected %d joysticks\n", SDL_NumJoysticks());

  SDL_Joystick* joys[MAX_PLAYERS];

  for(int i = 0; i < MAX_PLAYERS; ++i)
  {
    joys[i] = SDL_JoystickOpen(i);

    if(joys[i])
    {
      printf("Opened Joystick %d\n", i);
      printf("Name: %s\n", SDL_JoystickNameForIndex(i));
      printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joys[i]));
      printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joys[i]));
      printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joys[i]));
    }
    else
    {
      printf("Couldn't open Joystick %d\n", i);
    }
  }

  initGame();

  Input input {};

  while(1)
  {
    processInput(input);

    if(input.quit)
      break;

    updateGame(input);
    drawScreen(renderer);
  }

  for(int i = 0; i < MAX_PLAYERS; ++i)
    SDL_JoystickClose(joys[i]);

  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}

