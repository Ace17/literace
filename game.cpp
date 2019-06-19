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
  int survivors = 0;

  for(auto& bike : game.bikes)
    if(bike.alive)
      survivors++;

  return survivors < 2;
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

  game.obstacles.clear();

  int obCount = rand() % 3 + 1;

  for(int k = 0; k < obCount; ++k)
  {
    game.obstacles.push_back({
      { rand() % BOARD_WIDTH, rand() % BOARD_HEIGHT },
      { rand() % 3 - 1, rand() % 3 - 1 }, { rand() % 200 + 20, rand() % 200 + 20 }
    });
  }

  game.frameCount = 0;
  game.gameIsOver = false;
}

void checkForCollisions(Game& game, GameInput input)
{
  for(int i = 0; i < MAX_PLAYERS; ++i)
  {
    if(!game.bikes[i].alive)
      continue;

    for(int j = i + 1; j < MAX_PLAYERS; ++j)
    {
      if(!game.bikes[j].alive)
        continue;

      auto pos1 = game.bikes[i].pos;
      auto nextPos1 = computeNextBikePosition(game.bikes[i], input.players[i]);
      auto pos2 = game.bikes[j].pos;
      auto nextPos2 = computeNextBikePosition(game.bikes[j], input.players[j]);

      if(nextPos1 == nextPos2
         || (nextPos1 == pos2 && nextPos2 == pos1))
      {
        game.sink->onCrash(game.frameCount, { i, j });
        game.bikes[i].alive = false;
        game.bikes[j].alive = false;
        return;
      }
    }
  }
}

bool allBikeReady(Game& game)
{
  for(auto& bike : game.bikes)
    if(bike.direction == Direction::Idle)
      return false;

  return true;
}

void updateGame(Game& game, GameInput input)
{
  if(isGameOver(game))
  {
    if(!game.gameIsOver)
    {
      game.gameIsOver = true;
      game.sink->onRoundFinished();
    }

    if(input.restart)
      initGame(game);

    return;
  }

  for(int i = 0; i < MAX_PLAYERS; ++i)
    updateBikeDirection(game, game.bikes[i], input.players[i], 1 + i);

  if(!allBikeReady(game) && 0)
    return;

  checkForCollisions(game, input);

  for(int i = 0; i < MAX_PLAYERS; ++i)
    if(game.bikes[i].alive)
      updateBike(game, game.bikes[i], input.players[i], 1 + i);

  for(auto& ob : game.obstacles)
  {
    ob.pos.x += rand() % 3 - 1;
    ob.pos.y += rand() % 3 - 1;
    ob.pos.x += ob.vel.x;
    ob.pos.y += ob.vel.y;
    ob.size.x += rand() % 3 - 1;
    ob.size.y += rand() % 3 - 1;

    if(ob.pos.x < 0)
      ob.vel.x = abs(ob.vel.x);

    if(ob.pos.x >= BOARD_WIDTH)
      ob.vel.x = -abs(ob.vel.x);

    if(ob.pos.y < 0)
      ob.vel.y = abs(ob.vel.y);

    if(ob.pos.y >= BOARD_HEIGHT)
      ob.vel.y = -abs(ob.vel.y);
  }

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
  x = (x + BOARD_WIDTH) % BOARD_WIDTH;
  y = (y + BOARD_HEIGHT) % BOARD_HEIGHT;

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

  for(auto& ob : game.obstacles)
  {
    for(int y = 0; y < ob.size.y; ++y)
      for(int x = 0; x < ob.size.x; ++x)
        putPixel(pixels, ob.pos.x + x, ob.pos.y + y, -1);
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

