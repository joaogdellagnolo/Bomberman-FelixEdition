#pragma once

#include "BomberMan.h"

// conta bombas ativas de um jogador específico
int contarBombasAtivas(bool p2 = false) {
    int total = 0;
    for (Bomba& b : state->bombas)
        if ((b.temBomba || b.explodindo) && b.doP2 == p2)
            total++;
    return total;
}

// coloca bomba para um jogador
void colocarBomba(bool p2 = false) {
    Player& jogador = p2 ? state->p2 : state->p1;
    int limite  = p2 ? state->limiteBombasP2  : state->limiteBombas;
    int alcance = p2 ? state->alcanceBombaP2  : state->alcanceBomba;

    if (!jogador.alive) return;
    if (contarBombasAtivas(p2) >= limite) return;

    for (Bomba& b : state->bombas) {
        if ((b.temBomba || b.explodindo) &&
            b.pos.x == jogador.pos.x &&
            b.pos.y == jogador.pos.y &&
            b.doP2 == p2) return;
    }

    Bomba novaBomba;
    novaBomba.temBomba   = true;
    novaBomba.explodindo = false;
    novaBomba.pos        = jogador.pos;
    novaBomba.tempoBomba = 0;
    novaBomba.cooldownBomba = 0;
    novaBomba.doP2       = p2;

    int velBomba = p2 ? state->velocidadeBombaP2 : state->velocidadeBomba;
    novaBomba.tempoBomba = 0;

    state->bombas.push_back(novaBomba);
    state->screenBuffer[novaBomba.pos.y][novaBomba.pos.x] = BOMBA;
    state->hud.bombasUsadas++;
}

void gerenciarExplosao(Bomba& bomba, int tipoTile) {
    int bx = bomba.pos.x;
    int by = bomba.pos.y;

    // alcance da bomba: usa o do dono
    int alcance = bomba.doP2 ? state->alcanceBombaP2 : state->alcanceBomba;

    if (state->screenBuffer[by][bx] != BLOCO_SOLIDO)
        state->screenBuffer[by][bx] = tipoTile;

    int direcoes[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};

    for (int d = 0; d < 4; d++) {
        for (int a = 1; a <= alcance; a++) {
            int nx = bx + direcoes[d][0] * a;
            int ny = by + direcoes[d][1] * a;

            if (nx < 0 || nx >= wMax || ny < 0 || ny >= hMax) break;
            if (state->screenBuffer[ny][nx] == BLOCO_SOLIDO) break;

            int tileAntes = state->screenBuffer[ny][nx];
            state->screenBuffer[ny][nx] = tipoTile;
            if (tileAntes == PAREDE_DESTRUTIVEL) break;
        }
    }
}

void updateBomba() {
    for (Bomba& bomba : state->bombas) {
        int vel = bomba.doP2 ? state->velocidadeBombaP2 : state->velocidadeBomba;

        if (bomba.temBomba && !bomba.explodindo) {
            bomba.tempoBomba++;
            if (bomba.tempoBomba >= vel) {
                gerenciarExplosao(bomba, BOMBA_EXPLOSAO);
                bomba.temBomba   = false;
                bomba.explodindo = true;
                bomba.cooldownBomba = 0;
            }
        }

        if (bomba.explodindo) {
            bomba.cooldownBomba++;
            if (bomba.cooldownBomba > 20) {
                gerenciarExplosao(bomba, WHITE);
                bomba.explodindo = false;
            }
        }
    }

    for (int i = (int)state->bombas.size() - 1; i >= 0; i--) {
        if (!state->bombas[i].temBomba && !state->bombas[i].explodindo)
            state->bombas.erase(state->bombas.begin() + i);
    }
}
