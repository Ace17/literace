///////////////////////////////////////////////////////////////////////////////
// Game logic.
// No SDL or I/O should appear here.
#include "game.h"
#include <cstdio>
#include <map>

namespace
{
const int dirs[][2] =
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

void updateBikeDirection(Game& game, Bike& bike, PlayerInput input, int team)
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
  {
    if(bike.direction != wantedDirection)
      game.sink->onTurn(game.frameCount, team);

    bike.direction = wantedDirection;
  }
}

Vec2 computeNextBikePosition(Bike& bike, PlayerInput input)
{
  int speed = 1;

  if(input.boost)
    speed = 2;

  int dx = dirs[(int)bike.direction][0];
  int dy = dirs[(int)bike.direction][1];

  Vec2 nextPos;
  nextPos.x = bike.pos.x + dx * speed;
  nextPos.y = bike.pos.y + dy * speed;

  nextPos.x = (nextPos.x + BOARD_WIDTH) % BOARD_WIDTH;
  nextPos.y = (nextPos.y + BOARD_HEIGHT) % BOARD_HEIGHT;
  return nextPos;
}

void updateBike(Game& game, Bike& bike, PlayerInput input, int team)
{
  auto oldPos = bike.pos;
  auto nextPos = computeNextBikePosition(bike, input);

  if(oldPos != nextPos)
  {
    bike.pos = nextPos;

    if(game.board[bike.pos.y * BOARD_WIDTH + bike.pos.x])
    {
      game.sink->onKilled(game.frameCount, team, game.board[bike.pos.y * BOARD_WIDTH + bike.pos.x]);
      bike.alive = false;
    }
  }

  game.board[bike.pos.y * BOARD_WIDTH + bike.pos.x] = team;
}

bool isGameOver(Game& game)
{
  for(auto& bike : game.bikes)
    if(!bike.alive)
      return true;

  return false;
}
}

void initGame(Game& game)
{
  int k = 0;

  for(auto& bike : game.bikes)
  {
    bike = {};
    bike.pos.x = (k + 1) * BOARD_WIDTH / (MAX_PLAYERS + 1);
    bike.pos.y = BOARD_HEIGHT / 2;

    ++k;
  }

  for(auto& cell : game.board)
    cell = 0;

  game.frameCount = 0;
}

void checkForCollisions(Game& game, GameInput input)
{
  for(int i = 0; i < MAX_PLAYERS; ++i)
  {
    for(int j = i + 1; j < MAX_PLAYERS; ++j)
    {
      auto pos1 = game.bikes[i].pos;
      auto nextPos1 = computeNextBikePosition(game.bikes[i], input.players[i]);
      auto pos2 = game.bikes[j].pos;
      auto nextPos2 = computeNextBikePosition(game.bikes[j], input.players[j]);

      if(nextPos1 == nextPos2)
        game.sink->onCrash(game.frameCount, { i, j });

      if(nextPos1 == pos2 && nextPos2 == pos1)
        game.sink->onCrash(game.frameCount, { i, j });
    }
  }
}

void updateGame(Game& game, GameInput input)
{
  if(isGameOver(game))
  {
    if(input.restart)
      initGame(game);

    return;
  }

  for(int i = 0; i < MAX_PLAYERS; ++i)
    updateBikeDirection(game, game.bikes[i], input.players[i], 1 + i);

  checkForCollisions(game, input);

  for(int i = 0; i < MAX_PLAYERS; ++i)
    updateBike(game, game.bikes[i], input.players[i], 1 + i);

  game.frameCount++;
}

///////////////////////////////////////////////////////////////////////////////
// Display.
// No SDL or I/O should appear here.

namespace
{
int mkColor(int r, int g, int b)
{
  int color = 0;
  color |= 0xff;
  color <<= 8;
  color |= r;
  color <<= 8;
  color |= g;
  color <<= 8;
  color |= b;
  return color;
}

int darken(int color)
{
  int r = (color >> 16) & 0xff;
  int g = (color >> 8) & 0xff;
  int b = (color >> 0) & 0xff;
  return mkColor(r / 2, g / 2, b / 2);
}

void putPixel(int* pixels, int x, int y, int color)
{
  if(x < 0 || y < 0 || x >= BOARD_WIDTH || y >= BOARD_HEIGHT)
    return;

  pixels[y * BOARD_WIDTH + x] = color;
}
}

void drawGame(Game& game, int* pixels)
{
  static const int colors[] =
  {
    mkColor(40, 40, 40),
    mkColor(255, 255, 0),
    mkColor(64, 64, 255),
    mkColor(255, 0, 0),
    mkColor(0, 255, 0),
    mkColor(255, 255, 128),
    mkColor(128, 128, 255),
    mkColor(255, 128, 128),
    mkColor(128, 255, 128),
  };

  auto const N = (sizeof colors) / (sizeof *colors);

  for(int row = 0; row < BOARD_HEIGHT; ++row)
  {
    for(int col = 0; col < BOARD_WIDTH; ++col)
    {
      int c = game.board[row * BOARD_WIDTH + col];
      putPixel(pixels, col, row, colors[c % N]);
    }
  }

  // Draw player status
  for(int i = 0; i < MAX_PLAYERS; ++i)
  {
    auto color = colors[(1 + i) % N];

    for(int col = 10; col < 20; ++col)
      putPixel(pixels, col, 10 + i * 10 + 0, color);

    auto& bike = game.bikes[i];

    for(int j = -2; j <= 2; ++j)
      for(int k = -2; k <= 2; ++k)
        putPixel(pixels, bike.pos.x - k, bike.pos.y - j, darken(color));
  }
}

