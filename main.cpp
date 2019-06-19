// "Terminal" side.
#define GL_GLEXT_PROTOTYPES
#include <cstdio>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
#include "SDL.h"
#include "SDL_opengl.h"
#include "input.h"
#include "game.h"

using namespace std;

#define SAFE_GL(a) \
  do { a; ensureGl(# a, __LINE__); } while(0)

static
void ensureGl(char const* expr, int line)
{
  auto const errorCode = glGetError();

  if(errorCode == GL_NO_ERROR)
    return;

  std::string ss;
  ss += "OpenGL error\n";
  ss += "Expr: " + string(expr) + "\n";
  ss += "Line: " + to_string(line) + "\n";
  ss += "Code: " + to_string(errorCode) + "\n";
  fprintf(stderr, "%s", ss.c_str());
  exit(1);
}

struct Vertex
{
  float x, y;
  float u, v;
};

struct Match : IEventSink
{
  void onRoundFinished() override
  {
    printf("Round finished: ");

    for(auto killCount : kills)
      printf(" %d", killCount);

    printf("\n");
  }

  void onKilled(int frameCount, int victim, int killer) override
  {
    if(victim == killer)
    {
      printf("Bike %d committed suicide (lifetime=%d)\n", victim, frameCount);
      kills[victim - 1] = max(kills[victim - 1] - 1, 0);
    }
    else
    {
      printf("Bike %d was killed by %d (lifetime=%d)\n", victim, killer, frameCount);
      kills[killer - 1]++;
    }
  }

  void onTurn(int frameCount, int bike) override
  {
    if(0)
      printf("bike %d turned\n", bike);
  }

  void onCrash(int frameCount, vector<int> victims) override
  {
    printf("crash! victims:");

    for(auto victim : victims)
      printf(" %d", victim);

    printf("\n");
  }

  int kills[MAX_PLAYERS] {};
};

auto const vertex_shader = R"(#version 130
in vec2 pos;
in vec2 vertexUV;

out vec2 UV;

void main()
{
  UV = vertexUV;
  gl_Position = vec4( pos, 0.0, 1.0 );
}
)";

auto const fragment_shader = R"(#version 130
in vec2 UV;

out vec4 color;
uniform sampler2D diffuse;

const vec2 scale = vec2(0.001, 0.001);

const vec2 myFilter[7] = vec2[7](
	vec2(-3.0, 0.015),
	vec2(-2.0, 0.053),
	vec2(-1.0, 0.434),
	vec2( 0.0, 0.912),
	vec2( 1.0, 0.434),
	vec2( 2.0, 0.053),
	vec2( 3.0, 0.015)
);

void main()
{
  color = vec4(0, 0, 0, 0);

  for( int i = 0; i < 7; i++ )
  {
    vec2 pos = vec2( UV.x+myFilter[i].x*scale.x, UV.y+myFilter[i].x*scale.y);
    color += texture2D(diffuse, pos )*myFilter[i].y;
  }
}
)";

enum { attrib_position, attrib_uv };

int createShader(int type, const char* code)
{
  auto vs = glCreateShader(type);

  glShaderSource(vs, 1, &code, nullptr);
  glCompileShader(vs);

  GLint status;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &status);

  if(!status)
    printf("%s\n", code);

  assert(status);
  return vs;
}

static const Vertex vertices[] =
{
  { -1, -1, 0, 0 },
  { -1, +1, 0, -1 },
  { +1, -1, 1, 0 },

  { -1, +1, 0, -1 },
  { +1, +1, 1, -1 },
  { +1, -1, 1, 0 },
};

struct Display
{
  Display()
  {
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE | SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    m_window = SDL_CreateWindow("Literace", 0, 0, BOARD_WIDTH, BOARD_HEIGHT, SDL_WINDOW_OPENGL);
    assert(m_window);

    m_context = SDL_GL_CreateContext(m_window);
    assert(m_context);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    auto vs = createShader(GL_VERTEX_SHADER, vertex_shader);
    auto fs = createShader(GL_FRAGMENT_SHADER, fragment_shader);

    auto program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    SAFE_GL(glBindAttribLocation(program, attrib_position, "pos"));
    SAFE_GL(glBindAttribLocation(program, attrib_uv, "vertexUV"));
    SAFE_GL(glLinkProgram(program));

    SAFE_GL(glUseProgram(program));

    GLuint vbo;
    SAFE_GL(glGenBuffers(1, &vbo));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));

    GLuint texture;
    SAFE_GL(glGenTextures(1, &texture));
    SAFE_GL(glBindTexture(GL_TEXTURE_2D, texture));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    SAFE_GL(glActiveTexture(GL_TEXTURE0));

#define OFFSET(a) \
  ((GLvoid*)(&((Vertex*)nullptr)->a))

    SAFE_GL(glEnableVertexAttribArray(attrib_position));
    SAFE_GL(glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET(x)));
    SAFE_GL(glEnableVertexAttribArray(attrib_uv));
    SAFE_GL(glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET(u)));

    SAFE_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW));
  }

  ~Display()
  {
    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);
  }

  void refresh(const Uint32* pixels)
  {
    SAFE_GL(glClearColor(0, 1, 0, 1));
    SAFE_GL(glClear(GL_COLOR_BUFFER_BIT));
    SAFE_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BOARD_WIDTH, BOARD_HEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels));
    SAFE_GL(glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(*vertices)));

    SDL_GL_SwapWindow(m_window);
  }

  SDL_GLContext m_context;
  SDL_Window* m_window;
};

void drawScreen(Display& display, Game& game)
{
  static Uint32 pixels[BOARD_WIDTH * BOARD_HEIGHT];

  drawGame(game, (int*)pixels);
  display.refresh(pixels);
}

static auto const TIMESTEP_MS = 5;

int main()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  Display display;

  Game g_game;

  Match match;
  g_game.sink = &match;
  initGame(g_game);

  int64_t prev = SDL_GetTicks();
  int64_t timeAccumulator = 0;

  bool keepGoing = true;

  while(keepGoing)
  {
    auto now = SDL_GetTicks();
    timeAccumulator += now - prev;
    prev = now;

    while(timeAccumulator > 0)
    {
      timeAccumulator -= TIMESTEP_MS;
      auto input = processInput();

      if(input.quit)
      {
        keepGoing = false;
        break;
      }

      updateGame(g_game, input);
    }

    drawScreen(display, g_game);
    SDL_Delay(1);
  }

  destroyInput();

  SDL_Quit();
  return 0;
}

