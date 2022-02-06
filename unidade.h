#ifndef UNIDADE_H_
#define UNIDADE_H_

//#include "figura.h" 

#define INTERVALO (.3f)

/* Unidade é a classe genérica para elementos com os quais é possível interagir durante o jogo */
struct Unidade{
  Cartao cartao;
  int posx, posy;
  int id;
  float hp = 10;
  float dano = 0.;
  int equipe = 0;

  static int count;

  Unidade( string path_tex, int x=0, int y=0 ) :
    cartao{path_tex},
    posx{x},
    posy{y},
    id{count++}
  {};

  void draw(float time);

  virtual Unidade* action(float time) { return this;};
  virtual void colisao(Unidade *u) {};
};

/* Personagem representa a unidade referente ao personagem principal  */
/* controlado pelo usuário */
struct Personagem : Unidade {
  float cooldown = 0.1f;
  float last_shot = 0.;

  Personagem(ShaderProgram* sp)
    : Unidade("resources/sprito2.png", 2, 2)
  {
    cartao.program = sp;
    cartao.model = translate(0, -.03, 0) * scale(1, 1.6, 1.2);
    equipe = 1;
  };

  Unidade* fire(float t);
};

/* Projetil é a classe generica para ataques */
struct Projetil : Unidade {
  float last_time = 0;
  float vel = 56.f;
  vec2 dir = {1, 0};
  int last_posx = -1;
  int last_posy = -1;
  float dano = 5;
  int origem = -1;

  Projetil(string path, int x=0, int y=0)
    : Unidade(path, x, y),
      last_posx{x-1}
  {};

  virtual Unidade* action(float t);
  virtual void colisao(Unidade *u);
};

struct ShockWave : Projetil {
  int rastro = 2;

  ShockWave(string path, int x=0, int y=0)
    : Projetil(path, x, y)
  {
    dir = { -1, 0};
    dano = 5;
    vel = 20;
  };

  virtual Unidade* action(float t);
};

struct Piso : Unidade {
  /* vec3 color = {1.0, .0, .0}; */
  Piso(ShaderProgram* sp)
    : Unidade("resources/tex.jpeg")
  {
    cartao.program = sp;
  };

  void draw(int posx, int posy);
};

struct Inimigo : Unidade {
  Inimigo(ShaderProgram* sp, int x=0, int y=0)
    : Unidade("resources/cubomal.png")
  {
    posx = x;
    posy = y;
    cartao.program = sp;
    cartao.model = translate(0, -.03, 0) * scale(1, .95, .75);
    equipe = 2;
  };
};

struct Cubinho : Inimigo {
  float last_action = 0.;

  Cubinho(ShaderProgram* sp, int x=0, int y=0)
    : Inimigo(sp, x, y)
  {}

  virtual Unidade* action(float time);
};

/* Métodos */

void Unidade::draw(float time){
  float tranX = (3.8 - posy) * INTERVALO;
  /* float tranZ = (posx - MAPA_WIDTH/2. - .5 ) * INTERVALO; */
  float tranZ = (3.5 - posx) * INTERVALO;

  cartao.position = translate(tranX, 0., tranZ) * scale(1.5, 1.5, 1.5);

  cartao.draw(time);
};

int Unidade::count = 1; /* Inicializando contador de unidades */


Unidade* Personagem::fire(float t){
  if ((t - last_shot) > cooldown) {
    last_shot = t;

    // Personagem e seu projétil estão usando o mesmo shaderProgram
    Projetil *u = new Projetil{"resources/1_0.png", posx, posy};
    u->cartao.program = this->cartao.program;
    u->origem = id;
    return u;
  };
  return NULL;
};

Unidade* Projetil::action(float t){
  if (( !dir.x || last_posx == posx) && ( !dir.y || last_posy == posy ))
    return NULL;

  if ((t - last_time) > 1/vel) {
    last_posx = posx;
    last_posy = posy;
    posx += dir.x;
    posy += dir.y;
    last_time = t;
  }

  return this;
};

void Projetil::colisao(Unidade *u){
  if( equipe != u->equipe && origem != u->id ){
    u->hp -= dano;
    hp = -1;
  }
};

Unidade* ShockWave::action(float t){
  if (( !dir.x || last_posx == posx) && ( !dir.y || last_posy == posy ))
    return NULL;

  if ((t - last_time) > 1/vel) {
    last_posx = posx;
    last_posy = posy;
    posx += dir.x;
    posy += dir.y;
    last_time = t;
    if(rastro > 0){
      ShockWave *u = new ShockWave{"resources/sw.png", last_posx, last_posy};
      u->cartao.program = this->cartao.program;
      u->cartao.model = cartao.model * scale(1, .7, 1);
      u->origem = origem;
      u->last_time = last_time;
      u->rastro = --rastro;
      return u;
    }
  }

  return this;

};

void Piso::draw(int posx, int posy){
  float tranX = (3.5 - posy) * INTERVALO;
  float tranZ = (3 - posx) * INTERVALO;

  float sc = (INTERVALO / 2.);

  cartao.model = translate(tranX, 0., tranZ)* scale(sc, sc, sc) * rotate_x(M_PI/2.) ;

  cartao.draw(0.);
};

Unidade* Cubinho::action(float t){
  if ((t - last_action) > .05) { //menor = mais rapido
    last_action = t;

    float attack = rand() % 11;
    if( !attack ){
      ShockWave *u = new ShockWave{"resources/sw.png", posx - 1, posy};
      u->cartao.program = this->cartao.program;
      u->cartao.model = translate(-0.05,0,0) * scale(1, 1.2, .42);
      u->origem = id;
      return u;
    }

    float moveX =  rand() % 15;
    float moveY =  rand() % 15;
    posx += moveX < 2 ? -1 : ( moveX >= 10 ? 1 : 0);
    posy += moveY < 5 ? -1 : ( moveY >= 10 ? 1 : 0);
  }

  return this;
};

#endif // UNIDADE_H_
