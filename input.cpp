// Input side of the terminal.
// Depends on game logic.
#include <vector>
#include "SDL.h"
#include "input.h"

namespace
{
const int DEBUG_JOYSTICK = 0;

struct HumanWithAJoystick
{
  SDL_Joystick* joy;
  SDL_JoystickID id;
  int bikeId; // associated bike in the game
};

static std::vector<HumanWithAJoystick> g_humans;
static GameInput g_input {};

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

  if(!human.joy)
  {
    printf("Couldn't open Joystick %d\n", whichJoystick);
    return;
  }

  g_humans.push_back(human);

  printf("Player #%d enters! (joystick: %d)\n", human.bikeId, human.id);
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
  printf("Player %d has left (had joystick: %d).\n", g_humans[idx].bikeId, g_humans[idx].id);
  std::swap(g_humans[idx], g_humans.back());
  g_humans.pop_back();
}

void processEvent(SDL_Event const& event, GameInput& input)
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

    if(DEBUG_JOYSTICK)
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

    if(DEBUG_JOYSTICK)
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

    if(DEBUG_JOYSTICK)
      printf("[%d] %d\n", info.which, info.button);

    if(info.button == 0)
      player.boost = isPressed;
    else
      input.restart = isPressed;
  }
  else if(event.type == SDL_WINDOWEVENT)
  {
    // ignore
  }
  else if(event.type == SDL_AUDIODEVICEADDED)
  {
    // ignore
  }
  else
  {
    printf("Unknown event: %d\n", event.type);
  }
}
}

GameInput processInput()
{
  SDL_Event event;

  while(SDL_PollEvent(&event))
    processEvent(event, g_input);

  return g_input;
}

void destroyInput()
{
  while(!g_humans.empty())
    removeHuman(g_humans.back().id);
}

