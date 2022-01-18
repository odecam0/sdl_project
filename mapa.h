#ifndef MAPA_H_
#define MAPA_H_

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include "unidade.h"

using namespace std;

// Position representa cada posição do mapa
struct Position {
  vector <Unidade*> unidades;
  int equipe;
  int estado;
  bool bloqueado = false;

  void draw(int x, int y);
  void removeUnidade(Unidade *u);
  void addUnidade(Unidade *u);
};

// Contém o estado de uma batalha no que diz
// respeito às posições
struct Mapa {
  // Dimensões da matriz de posições
  const int dim1, dim2;  

  // Contém uma matriz de Positions que
  // correspondem às células do tabuleiro
  Position **mat;            

  // Ponteiros para as unidades que estão
  // sendo utilizadas na batalha
  //
  //    *  todas executam (ou não) uma ação
  //       que caso executada retorna uma nova
  //       unidade que é adcionada ao mapa em
  //       determinada posição.
  vector<Unidade*> unidades;

  // Ponteiro para personagem do jogador
  // ,que é uma unidade especial.
  //           especial pois se move e é
  //       controlada de forma especial.
  Personagem *personagem;

  // ...
  Piso *piso;

  Mapa(const int n, const int m,string path_tex="sprito.png");
  void moverPersonagem(int x, int y);
  void moverUnidade(Unidade *u, int x, int y);
  void initPersonagem(ShaderProgram *sp);
  void initPiso(ShaderProgram *sp);
  void actions(float time);
  void addUnidade(Unidade *u);
};

void Position::removeUnidade(Unidade *u)
{
  std::remove_if(
    unidades.begin(),
    unidades.end(),
    [u](Unidade *x){return (x->id == u->id);}
 );
};

void Position::addUnidade(Unidade *u)
{
  unidades.push_back(u);
}

void Mapa::moverUnidade(Unidade *u, int x, int y)
{
  int posx = max(0, min(dim1-1, x + u->posx));
  int posy = max(0, min(dim2-1, y + u->posy));

  if( mat[posx][posy].bloqueado )
    return;

  mat[u->posx][u->posy].removeUnidade(u);

  u->posx = posx;
  u->posy = posy;

  mat[posx][posy].addUnidade(u);
};

void Mapa::moverPersonagem(int x, int y)
{
  moverUnidade(personagem, x, y);
};

void Mapa::initPiso(ShaderProgram *sp)
{
  piso = new Piso(sp);
}

void Mapa::initPersonagem(ShaderProgram *sp)
{
  personagem = new Personagem(sp);
  mat[personagem->posx][personagem->posy].addUnidade(personagem);
}

Mapa::Mapa(const int n, const int m, string path_tex)
  : mat{new Position*[n]},
    dim1{n},
    dim2{m}
{
  for(int i=0; i<n; i++)
    mat[i] = new Position[m];
};

void Mapa::actions(float time){
  Unidade *u = NULL;
  for(int i = 0; i < unidades.size(); i++){
    u = unidades[i]->action(time);
    if(u)
      addUnidade(u);
  }
}

void Mapa::addUnidade(Unidade *u){
  unidades.push_back(u);
  mat[u->posx][u->posy].addUnidade(u);
}

#endif // MAPA_H_
