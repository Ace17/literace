#include <cstdio>
#include <cassert>
#include "SDL.h"
#include "game.h"

void processEvent(SDL_Event const& event, Input& input)
{
  if(event.type == SDL_QUIT)
  {
    input.quit = true;
    return;
  }

  if(event.type == SDL_JOYDEVICEADDED)
  {
    printf("Joystick added: %d\n", event.jdevice.which);
  }

  if(event.type == SDL_JOYDEVICEREMOVED)
  {
    printf("Joystick removed\n");
  }

  if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
  {
    bool isPressed = event.type == SDL_KEYDOWN;
    switch(event.key.keysym.scancode)
    {
    case SDL_SCANCODE_ESCAPE:
      input.quit = true;
      return;
    case SDL_SCANCODE_SPACE:
      input.restart = isPressed;
      break;
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

Uint32 pixels[WIDTH * HEIGHT];

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

  for(int row = 0; row < HEIGHT; ++row)
  {
    for(int col = 0; col < WIDTH; ++col)
    {
      int c = g_game.board[row * WIDTH + col];
      pixels[row * WIDTH + col] = colors[c % 5];
    }
  }

  SDL_UpdateTexture(texture, NULL, pixels, WIDTH * sizeof(Uint32));
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  auto window = SDL_CreateWindow("Literace", 0, 0, WIDTH, HEIGHT, 0);
  assert(window);

  auto renderer = SDL_CreateRenderer(window, -1, 0);
  assert(renderer);

  auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
  assert(texture);

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
    drawScreen(renderer, texture);
  }

  for(int i = 0; i < MAX_PLAYERS; ++i)
    SDL_JoystickClose(joys[i]);

  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}

