#pragma once

#include "BomberMan.h"

char sortearTipoItem() {
    char tipos[] = {'F', 'B', 'V', 'R', 'E'};
    return tipos[rand() % 5];
}

void spawnarItens() {
    int quantidade = (rand() % 6) + 3;

    for (int i = 0; i < quantidade; i++) {
        int x, y;
        int tentativas = 0;
        do {
            x = rand() % wMax;
            y = rand() % hMax;
            tentativas++;
        } while (tentativas < 500 && (
            state->screenBuffer[y][x] != WHITE ||
            (x == state->p1.pos.x && y == state->p1.pos.y) ||
            (state->p2Ativo && x == state->p2.pos.x && y == state->p2.pos.y)
        ));

        if (tentativas >= 500) continue;

        state->itens.push_back(Item(x, y, sortearTipoItem()));
    }
}

// Aplica item coletado ao jogador indicado
void aplicarItem(char simbolo, bool p2 = false) {
    Player& jogador = p2 ? state->p2 : state->p1;
    int& alcance  = p2 ? state->alcanceBombaP2  : state->alcanceBomba;
    int& limite   = p2 ? state->limiteBombasP2  : state->limiteBombas;
    int& velBomba = p2 ? state->velocidadeBombaP2 : state->velocidadeBomba;

    state->hud.itensPegos++;
    state->hud.pontuacao += 10;

    if (simbolo == 'F') {
        state->hud.itemFogo++;
        alcance++;
    } else if (simbolo == 'B') {
        state->hud.itemBombas++;
        limite++;
    } else if (simbolo == 'V') {
        state->hud.itemVidaExtra++;
        jogador.vidas++;
    } else if (simbolo == 'R') {
        state->hud.itemBombaRelogio++;
        if (velBomba > 10) velBomba -= 5;
    } else if (simbolo == 'E') {
        state->hud.itemEscudo++;
        jogador.escudo = true;
    }
}

void verificarColetaItem() {
    for (Item& item : state->itens) {
        if (!item.ativo) continue;

        if (item.pos.x == state->p1.pos.x && item.pos.y == state->p1.pos.y) {
            item.ativo = false;
            aplicarItem(item.simbolo, false);
        } else if (state->p2Ativo && state->p2Vivo &&
                   item.pos.x == state->p2.pos.x && item.pos.y == state->p2.pos.y) {
            item.ativo = false;
            aplicarItem(item.simbolo, true);
        }
    }
}

// Reseta os bônus de itens ao trocar de fase (conforme requisito do PDF)
void resetarItensFase() {
    state->alcanceBomba     = 1;
    state->limiteBombas     = 1;
    state->velocidadeBomba  = TICKS_UNTIL_NEXT_STATE_BOMB;
    state->alcanceBombaP2   = 1;
    state->limiteBombasP2   = 1;
    state->velocidadeBombaP2= TICKS_UNTIL_NEXT_STATE_BOMB;

    state->hud.itemFogo = 0;
    state->hud.itemEscudo = 0;
    state->hud.itemBombas = 0;
    state->hud.itemVidaExtra = 0;
    state->hud.itemBombaRelogio = 0;
    state->hud.itemSobreviveBomba = 0;

    // Não reseta vidas — mantemos as vidas acumuladas
    state->p1.escudo = false;
    state->p2.escudo = false;
    state->itens.clear();
    state->bombas.clear();
}
