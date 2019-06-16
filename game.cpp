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

