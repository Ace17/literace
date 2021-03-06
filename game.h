#pragma once

#include <vector>
#include <memory>
using std::vector;
using std::unique_ptr;

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

static int mkColor(int r, int g, int b)
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

static const int getColor(int index)
{
  static const int c[] =
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

  return c[index % 8];
}

struct ITerminal
{
  virtual void drawHead(Vec2 pos, int colorIndex) = 0;
  virtual void drawObstacle(Vec2 pos, Vec2 size) = 0;
};

struct NullTerminal : ITerminal
{
  void drawHead(Vec2 pos, int colorIndex) override {};
  void drawObstacle(Vec2 pos, Vec2 size) override {};
};

static NullTerminal nullTerminal;

struct NullEventSink : IEventSink
{
  void onRoundFinished() override {};
  void onKilled(int, int, int) override {};
  void onCrash(int, vector<int>) override {};
  void onTurn(int, int) override {};
};

static NullEventSink nullSink;

struct Obstacle
{
  Vec2 pos;
  Vec2 vel;
  Vec2 size;
  bool solid = false;
};

struct IGame
{
  virtual ~IGame() = default;
  virtual int update(GameInput input) = 0;
  virtual void draw(int* pixels) = 0;
};

unique_ptr<IGame> createGame(ITerminal* terminal, IEventSink* sink);

