#include <cstdio>
#include <cassert>
#include <vector>
#include "SDL.h"
#include "game.h"

struct HumanWithAJoystick
{
  SDL_Joystick* joy;
  SDL_JoystickID id;
  int which;
  int bikeId;
};

std::vector<HumanWithAJoystick> g_humans;

template<typename T, typename Lambda>
int indexOf(std::vector<T> const& array, Lambda predicate)
{
  for(int i = 0; i < (int)array.size(); ++i)
    if(predicate(array[i]))
      return i;

  return -1;
}

void addHuman(int whichJoystick)
{
  HumanWithAJoystick human {};

  auto hasOurId = [&] (HumanWithAJoystick const& existingHuman)
    {
      return existingHuman.bikeId == human.bikeId;
    };

  // find a free bikeId
  while(indexOf(g_humans, hasOurId) != -1)
    human.bikeId++;

  if(human.bikeId >= MAX_PLAYERS)
  {
    printf("Too many players\n");
    return;
  }

  human.joy = SDL_JoystickOpen(whichJoystick);
  human.id = SDL_JoystickGetDeviceInstanceID(whichJoystick);
  human.which = whichJoystick;

  if(!human.joy)
  {
    printf("Couldn't open Joystick %d\n", whichJoystick);
    return;
  }

  g_humans.push_back(human);

  printf("Player #%d enters! (joystick: %d)\n", human.bikeId, whichJoystick);
}

void removeHuman(SDL_JoystickID joyId)
{
  auto isItTheOneToRemove = [&] (HumanWithAJoystick const& human)
    {
      return human.id == joyId;
    };

  auto idx = indexOf(g_humans, isItTheOneToRemove);

  if(idx == -1)
    return; // this joystick wasn't assigned to a bike

  SDL_JoystickClose(g_humans[idx].joy);
  printf("Player %d has left (had joystick: %d).\n", g_humans[idx].bikeId, g_humans[idx].which);
  std::swap(g_humans[idx], g_humans.back());
  g_humans.pop_back();
}

void processEvent(SDL_Event const& event, Input& input)
{
  if(event.type == SDL_QUIT)
  {
    input.quit = true;
    return;
  }

  if(event.type == SDL_JOYDEVICEADDED)
    addHuman(event.jdevice.which);
  else if(event.type == SDL_JOYDEVICEREMOVED)
    removeHuman(event.jdevice.which);

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
    auto& player = input.players[info.which];

    printf("[%d] axis index: %d\n", info.which, info.axis);

    if(info.axis == 0) // horizontal
    {
      player.left = info.value < -16384;
      player.right = info.value > 16384;
    }
    else if(info.axis == 1) // vertical
    {
      player.up = info.value < -16384;
      player.down = info.value > 16384;
    }
  }
  else if(event.type == SDL_JOYHATMOTION)
  {
    auto& info = event.jhat;
    auto& player = input.players[info.which];

    printf("[%d] hat index: %d\n", info.which, info.hat);

    player.left = info.value == SDL_HAT_LEFT;
    player.right = info.value == SDL_HAT_RIGHT;
    player.up = info.value == SDL_HAT_UP;
    player.down = info.value == SDL_HAT_DOWN;
  }
  else if(event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP)
  {
    bool isPressed = event.type == SDL_JOYBUTTONDOWN;

    auto& info = event.jbutton;
    auto& player = input.players[info.which];

    printf("[%d] %d\n", info.which, info.button);

    if(info.button == 0)
      player.boost = isPressed;
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

  while(!g_humans.empty())
    removeHuman(g_humans.back().id);

  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}

