#include "game.h"

Game g_game;

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

void updateBike(Bike& bike, PlayerInput input, int team)
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

  bike.x = (bike.x + WIDTH) % WIDTH;
  bike.y = (bike.y + HEIGHT) % HEIGHT;

  if(dx || dy)
    if(g_game.board[bike.y * WIDTH + bike.x])
      bike.alive = false;

  g_game.board[bike.y * WIDTH + bike.x] = 1 + team;
}

bool isGameOver()
{
  for(auto& bike : g_game.bikes)
    if(!bike.alive)
      return true;

  return false;
}
}

void initGame()
{
  int k = 0;

  for(auto& bike : g_game.bikes)
  {
    bike = {};
    bike.x = (k + 1) * WIDTH / (MAX_PLAYERS + 1);
    bike.y = HEIGHT / 2;

    ++k;
  }

  for(auto& cell : g_game.board)
    cell = 0;
}

void updateGame(Input input)
{
  if(isGameOver())
  {
    if(input.restart)
      initGame();

    return;
  }

  for(int i = 0; i < MAX_PLAYERS; ++i)
    updateBike(g_game.bikes[i], input.players[i], i);
}

