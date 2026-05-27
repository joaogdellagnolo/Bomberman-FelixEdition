#pragma once

#include "BomberMan.h"

#define TICKS_UNTIL_NEXT_POS_ENEMY 30

// Sobrecarga simples: cria inimigo numa posição fixa
void criarInimigo(int x, int y) {
    state->enemies.push_back(Enemy(x, y));
    state->screenBuffer[y][x] = WHITE;
}

// Sobrecarga: Spawna 'qtd' inimigos em posições aleatórias livres
void criarInimigo(int mapa[hMax][wMax], int qtd) {
    int inimigosCriados = 0;
    int tentativas = 0;
    const int MAX_TENT = 1000;

    while (inimigosCriados < qtd && tentativas < MAX_TENT) {
        tentativas++;
        int x = rand() % wMax;
        int y = rand() % hMax;

        if (mapa[y][x] != WHITE) continue;

        // Não spawnar perto do P1
        if (abs(x - 1) <= 2 && abs(y - 1) <= 2) continue;

        // Não spawnar perto do P2 (nasce em 1,hMax-2)
        if (state->p2Ativo && abs(x - 1) <= 2 && abs(y - (hMax-2)) <= 2) continue;

        bool jaTemInimigo = false;
        for (const Enemy& e : state->enemies) {
            if (e.inimigoVivo && e.pos.x == x && e.pos.y == y) {
                jaTemInimigo = true;
                break;
            }
        }
        if (jaTemInimigo) continue;

        criarInimigo(x, y);
        inimigosCriados++;
    }
}

// Sobrecarga: Spawna inimigos marcados com 99 no mapa
void criarInimigo(int mapa[hMax][wMax]) {
    for (int i = 0; i < hMax; i++) {
        for (int j = 0; j < wMax; j++) {
            if (mapa[i][j] == 99) {
                criarInimigo(j, i);
                mapa[i][j] = WHITE;
                state->screenBuffer[i][j] = WHITE;
            }
        }
    }
}

// Spawna boss marcado com 98 no mapa
void spawnarBoss(int mapa[hMax][wMax]) {
    for (int i = 0; i < hMax; i++) {
        for (int j = 0; j < wMax; j++) {
            if (mapa[i][j] == BOSS_ID) {
                Enemy boss(j, i);
                boss.boss = true;
                boss.vida = 3;
                state->enemies.push_back(boss);
                mapa[i][j] = WHITE;
                state->screenBuffer[i][j] = WHITE;
            }
        }
    }
}

// Move inimigo em direção ao jogador mais próximo
void moverEmDirecaoJogador(Enemy& inimigo) {
    // Calcula alvo: o jogador vivo mais próximo
    int px = state->p1.pos.x;
    int py = state->p1.pos.y;
    bool p1vivo = state->p1.alive;

    if (state->p2Ativo && state->p2Vivo) {
        int d1 = abs(inimigo.pos.x - px) + abs(inimigo.pos.y - py);
        int d2 = abs(inimigo.pos.x - state->p2.pos.x) + abs(inimigo.pos.y - state->p2.pos.y);
        if (!p1vivo || d2 < d1) {
            px = state->p2.pos.x;
            py = state->p2.pos.y;
        }
    }

    int tentsDx[] = { (px > inimigo.pos.x ? 1 : -1), 0 };
    int tentsDy[] = { 0, (py > inimigo.pos.y ? 1 : -1) };

    for (int t = 0; t < 2; t++) {
        int ndx = tentsDx[t];
        int ndy = tentsDy[t];
        if (ndx == 0 && ndy == 0) continue;

        int nx = inimigo.pos.x + ndx;
        int ny = inimigo.pos.y + ndy;
        if (nx < 0 || nx >= wMax || ny < 0 || ny >= hMax) continue;

        bool bloqueado =
            state->screenBuffer[ny][nx] == BLOCO_SOLIDO ||
            state->screenBuffer[ny][nx] == PAREDE_DESTRUTIVEL ||
            state->screenBuffer[ny][nx] == BOMBA;

        if (!bloqueado) {
            inimigo.dx = ndx;
            inimigo.dy = ndy;
            inimigo.passosRestantes = 1;
            return;
        }
    }
    inimigo.passosRestantes = 0;
}

// Atualiza inimigo (movimento + morte)
void updateInimigo(Enemy& inimigo) {
    if (!inimigo.inimigoVivo) return;

    // Morreu na explosão
    if (state->screenBuffer[inimigo.pos.y][inimigo.pos.x] == BOMBA_EXPLOSAO) {
        if (!inimigo.tomouDano) {
            inimigo.tomouDano = true;
            inimigo.vida--;
            if (inimigo.vida <= 0) {
                inimigo.inimigoVivo = false;
                state->hud.pontuacao += inimigo.boss ? 200 : 50;
                return;
            }
        }
    } else {
        inimigo.tomouDano = false;
    }

    inimigo.tempoInimigo++;
    if (inimigo.tempoInimigo < TICKS_UNTIL_NEXT_POS_ENEMY) return;
    inimigo.tempoInimigo = 0;

    bool moveu = false;
    for (int tentativa = 0; tentativa < 8; tentativa++) {
        if (inimigo.passosRestantes <= 0) {
            int chancePersequir = 0;
            if (state->difficulty == 2) chancePersequir = 50;
            if (state->difficulty == 3) chancePersequir = 75;
            if (inimigo.boss)           chancePersequir = 100;

            if (chancePersequir > 0 && (rand() % 100) < chancePersequir) {
                moverEmDirecaoJogador(inimigo);
            }

            if (inimigo.passosRestantes <= 0) {
                int dir = rand() % 4;
                inimigo.dx = (dir == 2 ? -1 : (dir == 3 ? 1 : 0));
                inimigo.dy = (dir == 0 ? -1 : (dir == 1 ? 1 : 0));
                inimigo.passosRestantes = (rand() % 3) + 1;
            }
        }

        int proxX = inimigo.pos.x + inimigo.dx;
        int proxY = inimigo.pos.y + inimigo.dy;

        if (proxX < 0 || proxX >= wMax || proxY < 0 || proxY >= hMax) {
            inimigo.passosRestantes = 0;
            continue;
        }

        bool bloqueado =
            state->screenBuffer[proxY][proxX] == BLOCO_SOLIDO ||
            state->screenBuffer[proxY][proxX] == PAREDE_DESTRUTIVEL ||
            state->screenBuffer[proxY][proxX] == BOMBA;

        if (!bloqueado) {
            for (const Enemy& outro : state->enemies) {
                if (&outro == &inimigo) continue;
                if (outro.inimigoVivo && outro.pos.x == proxX && outro.pos.y == proxY) {
                    bloqueado = true;
                    break;
                }
            }
        }

        if (!bloqueado) {
            inimigo.pos.x = proxX;
            inimigo.pos.y = proxY;
            inimigo.passosRestantes--;
            moveu = true;
            break;
        } else {
            inimigo.passosRestantes = 0;
        }
    }

    if (!moveu) {
        inimigo.dx = 0;
        inimigo.dy = 0;
        inimigo.passosRestantes = 0;
    }
}

bool todosInimigosMortos() {
    for (const Enemy& e : state->enemies)
        if (e.inimigoVivo) return false;
    return true;
}
