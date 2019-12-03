///////////////////////////////////////////////////////////////////////////////
// Game logic.
// No SDL or I/O should appear here.
#include "game.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>

namespace
{
struct Game : IGame
{
  Bike bikes[MAX_PLAYERS];
  vector<Obstacle> obstacles;
  char board[BOARD_WIDTH * BOARD_HEIGHT];
  IEventSink* sink = &nullSink;
  ITerminal* terminal = &nullTerminal;
  int frameCount;
  bool gameIsOver;
  int gameOverDelay;

  int update(GameInput input) override;
  void draw(int* pixels) override;
};

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

std::unique_ptr<IGame> createGame(ITerminal* terminal, IEventSink* sink)
{
  auto pGame = std::make_unique<Game>();
  auto& game = *pGame;

  game.terminal = terminal;
  game.sink = sink;

  int k = 0;

  for(auto& bike : game.bikes)
  {
    bike = {};
    bike.pos.x = (k + 1) * BOARD_WIDTH / (MAX_PLAYERS + 1);
    bike.pos.y = BOARD_HEIGHT / 2;
    bike.direction = Direction::Up;

    ++k;
  }

  for(auto& cell : game.board)
    cell = 0;

  game.obstacles.clear();

  int obCount = rand() % 3 + 1;

  for(int k = 0; k < obCount; ++k)
  {
    Vec2 pos = { rand() % BOARD_WIDTH, rand() % BOARD_HEIGHT };
    Vec2 vel = { rand() % 3 - 1, rand() % 3 - 1 };
    Vec2 size = { rand() % 200 + 20, rand() % 200 + 20 };
    game.obstacles.push_back({ pos, vel, size, true });
  }

  game.frameCount = 0;
  game.gameIsOver = false;

  return pGame;
}

bool pointInsideSegment(int p, int left, int width, int mod)
{
  p = (p + mod) % mod;
  left = (left + mod) % mod;

  while(p < left)
    p += mod;

  return p >= left && p <= left + width;
}

bool pointInsideRectangle(Vec2 pos, Vec2 rectPos, Vec2 rectSize)
{
  if(!pointInsideSegment(pos.x, rectPos.x, rectSize.x, BOARD_WIDTH))
    return false;

  if(!pointInsideSegment(pos.y, rectPos.y, rectSize.y, BOARD_HEIGHT))
    return false;

  return true;
}

void checkForCollisions(Game& game, GameInput input)
{
  for(int i = 0; i < MAX_PLAYERS; ++i)
  {
    if(!game.bikes[i].alive)
      continue;

    for(auto& ob : game.obstacles)
    {
      auto& bike = game.bikes[i];

      if(pointInsideRectangle(bike.pos, ob.pos, ob.size))
      {
        game.sink->onCrash(game.frameCount, { i });
        bike.alive = false;
        break;
      }
    }

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

void eraseRectangle(Game& game, Vec2 pos, Vec2 size)
{
  for(int y = 0; y < size.y; ++y)
    for(int x = 0; x < size.x; ++x)
    {
      int xm = (pos.x + x) % BOARD_WIDTH;
      int ym = (pos.y + y) % BOARD_HEIGHT;
      assert(xm >= 0);
      assert(ym >= 0);
      game.board[ym * BOARD_WIDTH + xm] = 0;
    }
}

void updateObstacles(Game& game)
{
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

    ob.pos.x = (ob.pos.x + BOARD_WIDTH) % BOARD_WIDTH;
    ob.pos.y = (ob.pos.y + BOARD_HEIGHT) % BOARD_HEIGHT;

    eraseRectangle(game, ob.pos, ob.size);
  }
}

void oneTurn(Game& game, GameInput input)
{
  if(isGameOver(game))
  {
    if(!game.gameIsOver)
    {
      game.gameIsOver = true;
      game.gameOverDelay = 1000;
      game.sink->onRoundFinished();
    }

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

  updateObstacles(game);
  game.frameCount++;
}

int Game::update(GameInput input)
{
  static int turnAccumulator = 0;
  turnAccumulator += 100;

  while(turnAccumulator > 0)
  {
    turnAccumulator -= 500;
    oneTurn(*this, input);
  }

  if(gameOverDelay > 0)
    gameOverDelay--;

  return gameIsOver && gameOverDelay == 0 ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////
// Display.
// No SDL or I/O should appear here.

namespace
{
void putPixel(int* pixels, int x, int y, int color)
{
  x = (x + BOARD_WIDTH) % BOARD_WIDTH;
  y = (y + BOARD_HEIGHT) % BOARD_HEIGHT;

  pixels[y * BOARD_WIDTH + x] = color;
}
}

void Game::draw(int* pixels)
{
  auto& game = *this;
  for(int row = 0; row < BOARD_HEIGHT; ++row)
  {
    for(int col = 0; col < BOARD_WIDTH; ++col)
    {
      int c = game.board[row * BOARD_WIDTH + col];
      putPixel(pixels, col, row, getColor(c));
    }
  }

  for(auto& ob : game.obstacles)
    game.terminal->drawObstacle(ob.pos, ob.size);

  // Draw player status
  for(int i = 0; i < MAX_PLAYERS; ++i)
  {
    auto colorIndex = 1 + i;
    auto color = getColor(colorIndex);

    for(int col = 10; col < 20; ++col)
      putPixel(pixels, col, 10 + i * 10 + 0, color);

    auto& bike = game.bikes[i];

    if(bike.alive)
      game.terminal->drawHead(bike.pos, colorIndex);
  }
}

