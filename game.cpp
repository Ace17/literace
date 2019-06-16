///////////////////////////////////////////////////////////////////////////////
// Game logic.
// No SDL or I/O should appear here.
#include "game.h"

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

void updateBike(Game& game, Bike& bike, PlayerInput input, int team)
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

  bike.x = (bike.x + BOARD_WIDTH) % BOARD_WIDTH;
  bike.y = (bike.y + BOARD_HEIGHT) % BOARD_HEIGHT;

  if(dx || dy)
    if(game.board[bike.y * BOARD_WIDTH + bike.x])
      bike.alive = false;

  game.board[bike.y * BOARD_WIDTH + bike.x] = 1 + team;
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
    bike.x = (k + 1) * BOARD_WIDTH / (MAX_PLAYERS + 1);
    bike.y = BOARD_HEIGHT / 2;

    ++k;
  }

  for(auto& cell : game.board)
    cell = 0;
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
    updateBike(game, game.bikes[i], input.players[i], i);
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
  };

  for(int row = 0; row < BOARD_HEIGHT; ++row)
  {
    for(int col = 0; col < BOARD_WIDTH; ++col)
    {
      int c = game.board[row * BOARD_WIDTH + col];
      putPixel(pixels, col, row, colors[c % 5]);
    }
  }

  // Draw player status
  for(int i = 0; i < MAX_PLAYERS; ++i)
  {
    auto color = colors[(1 + i) % 5];

    for(int col = 10; col < 20; ++col)
      putPixel(pixels, col, 10 + i * 10 + 0, color);

    auto& bike = game.bikes[i];

    for(int j = -2; j <= 2; ++j)
      for(int k = -2; k <= 2; ++k)
        putPixel(pixels, bike.x - k, bike.y - j, darken(color));
  }
}

