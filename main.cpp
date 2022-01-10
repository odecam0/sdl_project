#include <string>
#include <iostream>

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

#else
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#endif

#include "scene.h"

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;
SDL_Event e;

Scene scene;

float rot = 0.f;
float dist = 0.f;
float rot_x = 0.f;
// int sprite = 1;

ShaderProgram *sp{nullptr};

// Inicialização de informações da cena desenhada
void setupScene(){
  scene.setProjection(
    scale(1., 1., -1.)*perspective(
      45.0f,                                    // fov
      (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, // aspect ratio
      0.1f,                                     // near
      100.f                                     // far
    ));

  scene.setView(
    lookAt(
      {5.5f, 2.f, 0.f},  // eye
      {-2.f, .0f, 0.f},  // center
      {0.f, 1.f, 0.f}    // up
    ));

  sp = new ShaderProgram{
    Shader{"shaders/cartao.vert", GL_VERTEX_SHADER},
    Shader{ "shaders/cartao.frag", GL_FRAGMENT_SHADER}
  };

  scene.addFigura("resources/cenario4.obj", "resources/cenario4.png");

  scene.figuras.back().program = new  ShaderProgram{
    Shader{"shaders/vertex_shader", GL_VERTEX_SHADER},
    Shader{"shaders/fragment_shader", GL_FRAGMENT_SHADER}
  };

  scene.figuras.back().model = scale(2., 2., 2.);

  scene.mapa.initPersonagem(sp);
  scene.mapa.initPiso( new ShaderProgram{
    Shader{"shaders/piso.vert", GL_VERTEX_SHADER},
    Shader{ "shaders/piso.frag", GL_FRAGMENT_SHADER}
  });

}

// Inicialização do sdl e opengl
bool init()
{
  //Initialize SDL
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
  {
    printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
    return false;
  }

  // Setando versão do opengl para 3.3
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );

  //Criando janela
  gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if( gWindow == NULL )
  {
    printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
    return false;
  }

  //Criando contexto opengl
  gContext = SDL_GL_CreateContext( gWindow );
  if( gContext == NULL )
  {
    printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
    return false;
  }

  if( SDL_GL_SetSwapInterval( 1 ) < 0 )
  {
    printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
  }

  //Inicializando GLEW
  glewExperimental = GL_FALSE;
  if( glewInit() != GLEW_OK )
    throw std::runtime_error("glewInit failed");

  //Inicializando o DEPTH buffer
  glEnable(GL_DEPTH_TEST);

  //Inicializando OpenGL
  glClearColor(.3f, 0.6f, 1.f, 1.f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glCullFace(GL_FRONT);

  return true;
}

static bool quit = false;
void eventHandler( SDL_Event &e ) {
  const Uint8 *state = SDL_GetKeyboardState(NULL);
  
  if( e.type == SDL_QUIT || state[SDL_SCANCODE_ESCAPE] )
  {
    quit = true;
  }

  if( e.type == SDL_KEYDOWN )
  {
    // key down
    if( e.key.keysym.sym == SDLK_RIGHT)
      rot += .02;
    else if ( e.key.keysym.sym == SDLK_LEFT)
      rot -= .02;
    else if ( e.key.keysym.sym == SDLK_DOWN )
      dist += .08;
    else if ( e.key.keysym.sym == SDLK_UP )
      dist -= .08;
    else if ( e.key.keysym.sym == SDLK_r )
      rot_x += .02;
    else if ( e.key.keysym.sym == SDLK_f )
      rot_x -= .02;

    // movimentação
    else if ( e.key.keysym.sym == SDLK_w )
      scene.mapa.moverPersonagem(0, 1);
    else if ( e.key.keysym.sym == SDLK_s )
      scene.mapa.moverPersonagem(0, -1);
    else if ( e.key.keysym.sym == SDLK_a )
      scene.mapa.moverPersonagem(1, 0);
    else if ( e.key.keysym.sym == SDLK_d )
      scene.mapa.moverPersonagem(-1, 0);
  }

  if (e.type == SDL_WINDOWEVENT)
  {
    if (e.window.event == SDL_WINDOWEVENT_RESIZED)
    {
      SDL_SetWindowSize(gWindow , e.window.data1, e.window.data2);
      glViewport(0, 0, e.window.data1, e.window.data2);
    }
  }

  if( e.type == SDL_QUIT || state[SDL_SCANCODE_ESCAPE] )
  {
    quit = true;
  }
}

// Função de renderização
void main_loop(){
  while( SDL_PollEvent( &e ) != 0 )
  {
    eventHandler(e);
    if(quit == true)
      break;
  }

  scene.setView(
    lookAt(
      toVec3( rotate_y(rot) * rotate_z(rot_x) * translate(dist, .0, .0) * vec4{5.8f, 3.f, 0.f, 1.f} ),// eye
      toVec3( rotate_y(rot) * rotate_z(rot_x) * translate(dist, .0, .0) * vec4{-.5f, 0.f, 0.f, 1.f} ),// center
      {0.f, 1.f, 0.f}   // up
    )
  );

  // limpa tela
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // Desenha cena
  scene.draw();

  // Atualização da janela
  SDL_GL_SwapWindow( gWindow );
};

// Libera recursos
void close()
{
  SDL_DestroyWindow( gWindow );
  gWindow = NULL;
  SDL_Quit();
}

int main(int argc, char *argv[]) {

  if( !init() )
  {
    printf( "Failed to initialize!\n" );
  }
  else
  {
    quit = false;

    SDL_StartTextInput();

    setupScene();

#ifdef __EMSCRIPTEN__

    emscripten_set_main_loop(main_loop, 0, 1);

#else

    while(!quit) {
      main_loop();
      SDL_Delay(20);
    }

#endif
  }

  close();

  return 0;
}
