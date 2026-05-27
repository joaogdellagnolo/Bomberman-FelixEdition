#pragma once

// =============================================
// Trabalho M2 - Bomberman
// Algoritmos e Programação II
// Desenvolvido por:  Joao Felix, Derick Kunz, Joao Guilherme, Eduardo Loyola
// =============================================

#include <iostream>
#include <windows.h>
#include <vector>
#include <ctime>

// CONSTANTES
#define FRAME_SPEED 0.05
#define TICKS_UNTIL_NEXT_STATE_BOMB 45
#define wMax 22
#define hMax 12
#define PORTAL 6
#define BOSS_ID 98
#define TOTAL_FASES 5

// =============================================
// STRUCTS
// =============================================

struct Coordenadas {
    int x, y;
};

struct Player {
    Coordenadas pos;
    bool alive;
    int vidas = 1;
    int tempoImune = 0;
    bool escudo = false;
    bool sobreviveBomba = false;

    Player() { pos = {1, 1}; alive = true; }
};

struct Enemy {
    Coordenadas pos;
    bool inimigoVivo;
    int tempoInimigo;
    int dx = 0;
    int dy = 0;
    int passosRestantes = 0;
    bool boss = false;
    int vida = 1;
    bool tomouDano = false;

    Enemy() : pos({0, 0}), inimigoVivo(false), tempoInimigo(0), boss(false), vida(1), tomouDano(false) {}
    Enemy(int x, int y) : pos({x, y}), inimigoVivo(true), tempoInimigo(0), boss(false), vida(1), tomouDano(false) {}
};

struct Bomba {
    Coordenadas pos;
    bool temBomba   = false;
    bool explodindo = false;
    int tempoBomba  = 0;
    int cooldownBomba = 0;
    bool doP2 = false; // identifica se a bomba pertence ao jogador 2
};

struct Item {
    Coordenadas pos;
    char simbolo;
    bool ativo;

    Item() : pos({0, 0}), simbolo(0), ativo(false) {}
    Item(int x, int y, char s) {
        pos = {x, y};
        simbolo = s;
        ativo = true;
    }
};

struct HudInfo {
    int pontuacao = 0;
    int movimentos = 0;
    int bombasUsadas = 0;
    int itensPegos = 0;

    int itemFogo = 0;
    int itemEscudo = 0;
    int itemBombas = 0;
    int itemVidaExtra = 0;
    int itemBombaRelogio = 0;
    int itemSobreviveBomba = 0;
    time_t inicioJogo;
};

// =============================================
// TEMPLATE: Usado para serialização genérica de vetores
// (salvar/carregar qualquer vetor de struct trivial em arquivo binário)
// =============================================
template<typename T>
static bool writeVec(FILE* f, const std::vector<T>& v) {
    size_t n = v.size();
    if (fwrite(&n, sizeof(n), 1, f) != 1) return false;
    if (n > 0 && fwrite(v.data(), sizeof(T), n, f) != n) return false;
    return true;
}

template<typename T>
static bool readVec(FILE* f, std::vector<T>& v) {
    size_t n = 0;
    if (fread(&n, sizeof(n), 1, f) != 1) return false;
    if (n > 100000) return false; // sanity check anti-crash
    v.resize(n);
    if (n > 0 && fread(v.data(), sizeof(T), n, f) != n) return false;
    return true;
}

struct GameState {
    bool session = false;
    Player p1;
    Player p2;
    bool p2Ativo = false;       // multiplayer ativo?
    bool p2Vivo  = false;       // p2 esta alive nesta sessao?

    std::vector<Bomba> bombas;
    std::vector<Enemy> enemies;
    std::vector<Item> itens;

    HudInfo hud;

    // Stats do P1
    int alcanceBomba  = 1;
    int limiteBombas  = 1;
    int velocidadeBomba = TICKS_UNTIL_NEXT_STATE_BOMB;

    // Stats do P2
    int alcanceBombaP2  = 1;
    int limiteBombasP2  = 1;
    int velocidadeBombaP2 = TICKS_UNTIL_NEXT_STATE_BOMB;

    int screenBuffer[hMax][wMax];
    time_t timestamp;

    int difficulty = 1;
    int faseAtual  = 1;         // fase atual (1..TOTAL_FASES)
    int pontuacaoAcumulada = 0; // carrega entre fases
};

static struct GameState* state;

void saveGame(int slot);
bool loadGame(int slot);
bool checkColisao(int target, int posX, int posY, int offX, int offY);
