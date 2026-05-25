#pragma once

#include "bomberman.h"

#define TICKS_UNTIL_NEXT_POS_ENEMY 30

// cria inimigo
void criarInimigo(int x, int y) {
    state->enemies.push_back(Enemy(x, y));
}

// atualiza inimigo (movimento + morte)
void updateInimigo(Enemy& inimigo) {

    if (!inimigo.inimigoVivo)
        return;

    // morreu na explosao
    if (state->screenBuffer[inimigo.pos.y][inimigo.pos.x] == BOMBA_EXPLOSAO) {
        inimigo.inimigoVivo = false;
        return;
    }

    inimigo.tempoInimigo++;

    if (inimigo.tempoInimigo < TICKS_UNTIL_NEXT_POS_ENEMY)
        return;

    inimigo.tempoInimigo = 0;

    // se nao tem passos definidos, escolhe nova direcao
    if (inimigo.passosRestantes <= 0) {
        int dir = rand() % 4;

        inimigo.dx = (dir == 2 ? -1 : (dir == 3 ? 1 : 0));
        inimigo.dy = (dir == 0 ? -1 : (dir == 1 ? 1 : 0));

        inimigo.passosRestantes = (rand() % 3) + 1; // 1 a 3 passos
    }

    // tenta andar 1 passo
    bool bloqueado =
        checkColisao(BLOCO_SOLIDO, inimigo.pos.x, inimigo.pos.y, inimigo.dx, inimigo.dy) ||
        checkColisao(PAREDE_DESTRUTIVEL, inimigo.pos.x, inimigo.pos.y, inimigo.dx, inimigo.dy) ||
        checkColisao(BOMBA, inimigo.pos.x, inimigo.pos.y, inimigo.dx, inimigo.dy);

    if (!bloqueado) {
        inimigo.pos.x += inimigo.dx;
        inimigo.pos.y += inimigo.dy;
        inimigo.passosRestantes--; // anda 1 passo por frame
    }
    else {
        // se bateu em algo, cancela movimento atual
        inimigo.passosRestantes = 0;
    }
}

// ganhou o jogo?
bool todosInimigosMortos() {
    for (const Enemy& e : state->enemies) {
        if (e.inimigoVivo)
            return false;
    }
    return true;
}