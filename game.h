#pragma once

static auto const MAX_PLAYERS = 4;
static auto const BOARD_WIDTH = 1024;
static auto const BOARD_HEIGHT = 768;

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

struct GameInput
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
  char board[BOARD_WIDTH * BOARD_HEIGHT];
};

void initGame(Game& game);
void updateGame(Game& game, GameInput input);
void drawGame(Game& game, int* pixels);

