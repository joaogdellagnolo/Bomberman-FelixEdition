#pragma once

// =============================================
// Trabalho M1 - Bomberman
// Algoritmos e Programação II
// Desenvolvido por:  Joao Felix, Derick Kunz, Joao Guilherme, Eduardo Loyola
// =============================================


#include <iostream>
#include <windows.h>
#include <vector>
#include <ctime>

//variaveis globais
extern struct GameState* state; 

// CONSTANTES
#define wMax 20
#define hMax 10
#define TICKS_UNTIL_NEXT_POS_ENEMY 30 //Modificar esses valores para + desacelera e para - acelera a bomba/inimigo.
#define TICKS_UNTIL_NEXT_STATE_BOMB 45


// STRUCTS (Dados do Jogo)

//facilita o acesso a cordanada de entidades 
struct Coordenadas {
    int x, y;
};

struct Player {
    Coordenadas pos;
    bool alive;
    //jogador começa em (1,1) para nao nascer dentro da parede
    Player() { pos = {1, 1}; alive = true; }
};

struct Enemy {
    Coordenadas pos;
    bool inimigoVivo;
    int tempoInimigo;
    int dx = 0;
    int dy = 0;
    int passosRestantes = 0;

    Enemy(int x, int y) : pos({x, y}), inimigoVivo(true), tempoInimigo(0) {}
};

struct Bomba {
    Coordenadas pos;
    bool temBomba   = false;
    bool explodindo = false;
    int tempoBomba  = 0;
    int cooldownBomba = 0;
};


struct HudInfo {
    int pontuacao = 0;
    int movimentos = 0;
    int bombasUsadas = 0;
    int itensPegos = 0;

    int itemFogo = 0;
    int itemBombas = 0;
    int itemVidaExtra = 0;
    int itemBombaRelogio = 0;
    int itemSobreviveBomba = 0;
    int itemPassaBlocos = 0;
    time_t inicioJogo;
};


struct GameState {
    bool session = false;
    Player p1;
    Bomba bomba;
    std::vector<Enemy> enemies;
    HudInfo hud;
    int screenBuffer[hMax + 2][wMax + 2];
    time_t timestamp;
};
