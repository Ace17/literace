#pragma once

static auto const MAX_PLAYERS = 2;
static auto const WIDTH = 1024;
static auto const HEIGHT = 768;

enum class Direction
{
  Idle,
  Left,
  Down,
  Right,
  Up,
};

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

struct Bike
{
  bool alive = true;
  int x, y;
  Direction direction;
};

struct Game
{
  Bike bikes[MAX_PLAYERS];
  char board[WIDTH * HEIGHT];
};

void initGame(Game& game);
void updateGame(Game& game, Input input);

