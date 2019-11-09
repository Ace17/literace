#pragma once

#include <vector>
using std::vector;

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

struct Vec2
{
  int x, y;

  bool operator != (Vec2 const& other) const
  {
    return !(*this == other);
  }

  bool operator == (Vec2 const& other) const
  {
    return x == other.x && y == other.y;
  }
};

struct Bike
{
  bool alive = true;
  Vec2 pos;
  Direction direction;
};

struct IEventSink
{
  virtual void onRoundFinished() = 0;
  virtual void onKilled(int frameCount, int victim, int killer) = 0;
  virtual void onCrash(int frameCount, vector<int> victims) = 0;
  virtual void onTurn(int frameCount, int bike) = 0;
};

struct NullEventSink : IEventSink
{
  void onRoundFinished() override {};
  void onKilled(int, int, int) override {};
  void onCrash(int, vector<int> ) override {};
  void onTurn(int, int) override {};
};

static NullEventSink nullSink;

struct Obstacle
{
  Vec2 pos;
  Vec2 vel;
  Vec2 size;
};

struct Game
{
  Bike bikes[MAX_PLAYERS];
  vector<Obstacle> obstacles;
  char board[BOARD_WIDTH * BOARD_HEIGHT];
  IEventSink* sink = &nullSink;
  int frameCount;
  bool gameIsOver;
};

void initGame(Game& game);
void updateGame(Game& game, GameInput input);
void drawGame(Game& game, int* pixels);

