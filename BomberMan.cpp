// =============================================
// Trabalho M2 - Bomberman
// Algoritmos e Programação II
// Desenvolvido por:  Joao Felix, Derick Kunz, Joao Guilherme, Eduardo Loyola
// =============================================

#include "BomberMan.h"
#include "Mapas.h"
#include "inimigo.h"
#include "Bomba.h"
#include "item.h"
#include "Menu.h"
#include "Ranking.h"

#include <signal.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// =============================================
// COLISÃO
// =============================================
bool checkColisao(int target, int posX, int posY, int offX, int offY) {
    int destX = posX + offX;
    int destY = posY + offY;
    if (destX < 0 || destX >= wMax || destY < 0 || destY >= hMax) return true;
    return state->screenBuffer[destY][destX] == target;
}

bool checkColisaoJogadorInimigo(bool p2 = false) {
    Player& jogador = p2 ? state->p2 : state->p1;
    if (!jogador.alive) return false;
    for (const Enemy& e : state->enemies) {
        if (e.inimigoVivo && e.pos.x == jogador.pos.x && e.pos.y == jogador.pos.y)
            return true;
    }
    return false;
}

// =============================================
// SAVE / LOAD  (usa template writeVec/readVec de BomberMan.h)
// =============================================
static void ensureSavesDir() { CreateDirectoryA("saves", NULL); }

static std::string getSavePath(int slot) {
    return std::string("saves/savegame_") + std::to_string(slot) + ".dk";
}

void saveGame(int slot = 1) {
    ensureSavesDir();
    FILE* f = fopen(getSavePath(slot).c_str(), "wb");
    if (!f) return;

    fwrite(&state->session,          sizeof(state->session),          1, f);
    fwrite(&state->p1,               sizeof(state->p1),               1, f);
    fwrite(&state->p2,               sizeof(state->p2),               1, f);
    fwrite(&state->p2Ativo,          sizeof(state->p2Ativo),          1, f);
    fwrite(&state->p2Vivo,           sizeof(state->p2Vivo),           1, f);
    fwrite(&state->hud,              sizeof(state->hud),              1, f);
    fwrite(&state->alcanceBomba,     sizeof(state->alcanceBomba),     1, f);
    fwrite(&state->limiteBombas,     sizeof(state->limiteBombas),     1, f);
    fwrite(&state->velocidadeBomba,  sizeof(state->velocidadeBomba),  1, f);
    fwrite(&state->alcanceBombaP2,   sizeof(state->alcanceBombaP2),   1, f);
    fwrite(&state->limiteBombasP2,   sizeof(state->limiteBombasP2),   1, f);
    fwrite(&state->velocidadeBombaP2,sizeof(state->velocidadeBombaP2),1, f);
    fwrite(&state->timestamp,        sizeof(state->timestamp),        1, f);
    fwrite(&state->difficulty,       sizeof(state->difficulty),       1, f);
    fwrite(&state->faseAtual,        sizeof(state->faseAtual),        1, f);
    fwrite(&state->pontuacaoAcumulada,sizeof(state->pontuacaoAcumulada),1,f);
    fwrite( state->screenBuffer,     sizeof(state->screenBuffer),     1, f);

    // Template writeVec<T> usado aqui para os 3 vetores
    writeVec(f, state->bombas);
    writeVec(f, state->enemies);
    writeVec(f, state->itens);

    fclose(f);
}

bool loadGame(int slot = 1) {
    FILE* f = fopen(getSavePath(slot).c_str(), "rb");
    if (!f) return false;

    GameState* loaded = new GameState();
    bool ok =
        fread(&loaded->session,          sizeof(loaded->session),          1, f) == 1 &&
        fread(&loaded->p1,               sizeof(loaded->p1),               1, f) == 1 &&
        fread(&loaded->p2,               sizeof(loaded->p2),               1, f) == 1 &&
        fread(&loaded->p2Ativo,          sizeof(loaded->p2Ativo),          1, f) == 1 &&
        fread(&loaded->p2Vivo,           sizeof(loaded->p2Vivo),           1, f) == 1 &&
        fread(&loaded->hud,              sizeof(loaded->hud),              1, f) == 1 &&
        fread(&loaded->alcanceBomba,     sizeof(loaded->alcanceBomba),     1, f) == 1 &&
        fread(&loaded->limiteBombas,     sizeof(loaded->limiteBombas),     1, f) == 1 &&
        fread(&loaded->velocidadeBomba,  sizeof(loaded->velocidadeBomba),  1, f) == 1 &&
        fread(&loaded->alcanceBombaP2,   sizeof(loaded->alcanceBombaP2),   1, f) == 1 &&
        fread(&loaded->limiteBombasP2,   sizeof(loaded->limiteBombasP2),   1, f) == 1 &&
        fread(&loaded->velocidadeBombaP2,sizeof(loaded->velocidadeBombaP2),1, f) == 1 &&
        fread(&loaded->timestamp,        sizeof(loaded->timestamp),        1, f) == 1 &&
        fread(&loaded->difficulty,       sizeof(loaded->difficulty),       1, f) == 1 &&
        fread(&loaded->faseAtual,        sizeof(loaded->faseAtual),        1, f) == 1 &&
        fread(&loaded->pontuacaoAcumulada,sizeof(loaded->pontuacaoAcumulada),1,f) == 1 &&
        fread( loaded->screenBuffer,     sizeof(loaded->screenBuffer),     1, f) == 1 &&
        readVec(f, loaded->bombas)   &&
        readVec(f, loaded->enemies)  &&
        readVec(f, loaded->itens);

    fclose(f);

    if (ok) {
        delete state;
        state = loaded;
        state->session = true;
    } else {
        delete loaded;
    }
    return ok;
}

// =============================================
// DANO AO JOGADOR
// =============================================
void aplicarDano(Player& jogador) {
    if (jogador.escudo) {
        jogador.escudo = false;
        if (state->hud.itemEscudo > 0) state->hud.itemEscudo--;
        jogador.tempoImune = 20;
    } else if (jogador.vidas > 1) {
        jogador.vidas--;
        if (state->hud.itemVidaExtra > 0) state->hud.itemVidaExtra--;
        jogador.tempoImune = 20;
    } else {
        jogador.alive = false;
    }
}

// =============================================
// INPUT
// =============================================
void inputHandler() {
    while (_kbhit()) {
        char tecla = _getch();

        int dx = 0, dy = 0;

        switch (tecla) {
            case 'w': case 'W': dy = -1; break;
            case 's': case 'S': dy =  1; break;
            case 'a': case 'A': dx = -1; break;
            case 'd': case 'D': dx =  1; break;
            case 'e': case 'E': colocarBomba(false); break;
            case '\r':          // Enter normal = bomba P2
                if (state->p2Ativo && state->p2Vivo) colocarBomba(true);
                break;
            case 't': case 'T': state->session = false; break;
            case 'p': case 'P': saveGame(); break;
            case 0: case (char)224: {
                // Setas: controle do P2
                if (!state->p2Ativo || !state->p2Vivo) {
                    _getch(); // consome o segundo byte sem fazer nada
                    break;
                }
                char ext = _getch();
                int dx2 = 0, dy2 = 0;
                if      (ext == 72) dy2 = -1; // seta cima
                else if (ext == 80) dy2 =  1; // seta baixo
                else if (ext == 75) dx2 = -1; // seta esquerda
                else if (ext == 77) dx2 =  1; // seta direita

                if (dx2 != 0 || dy2 != 0) {
                    bool p1 = checkColisao(BLOCO_SOLIDO, state->p2.pos.x, state->p2.pos.y, dx2, dy2);
                    bool p2 = checkColisao(PAREDE_DESTRUTIVEL, state->p2.pos.x, state->p2.pos.y, dx2, dy2);
                    if (!p1 && !p2) {
                        state->p2.pos.x += dx2;
                        state->p2.pos.y += dy2;
                        state->hud.movimentos++;
                        if (state->hud.pontuacao > 0) state->hud.pontuacao--;
                    }
                }
                break;
            }
        }

        if (dx != 0 || dy != 0) {
            bool w1 = checkColisao(BLOCO_SOLIDO, state->p1.pos.x, state->p1.pos.y, dx, dy);
            bool w2 = checkColisao(PAREDE_DESTRUTIVEL, state->p1.pos.x, state->p1.pos.y, dx, dy);
            if (!w1 && !w2) {
                state->p1.pos.x += dx;
                state->p1.pos.y += dy;
                state->hud.movimentos++;
                if (state->hud.pontuacao > 0) state->hud.pontuacao--;
            }
        }
    }
}

// =============================================
// INICIALIZA UMA FASE
// =============================================
void inicializarFase(int fase, int diff, bool p2Ativo, int pontosPrevios) {
    // Cria estado do zero
    if (state) delete state;
    state = new GameState();

    state->session = true;
    state->faseAtual = fase;
    state->difficulty = diff;
    state->p2Ativo = p2Ativo;
    state->p2Vivo  = p2Ativo;
    state->pontuacaoAcumulada = pontosPrevios;
    state->hud.pontuacao = pontosPrevios;
    state->hud.inicioJogo = time(nullptr);

    // Posição inicial P2 na parte inferior esquerda
    state->p2.pos = {1, hMax - 2};
    state->p2.alive = true;

    // Copia o mapa da fase
    int (*mapa)[wMax];
    switch(fase) {
        case 1: mapa = map_1; break;
        case 2: mapa = map_2; break;
        case 3: mapa = map_3; break;
        case 4: mapa = map_4; break;
        case 5: default: mapa = map_5; break;
    }

    for (int i = 0; i < hMax; i++)
        for (int j = 0; j < wMax; j++)
            state->screenBuffer[i][j] = mapa[i][j];

    int qtdInimigos = 3;
    if (diff == 2) qtdInimigos = 5;
    if (diff == 3) qtdInimigos = 7;

    // Inimigos aleatórios
    criarInimigo(mapa, qtdInimigos);
    // Inimigos fixos (marcados com 99)
    criarInimigo(mapa);

    // Boss nas fases 3 e 5 (dificil) ou na fase 3 (qualquer dificuldade)
    bool temBoss = (fase == 3) || (fase == 5 && diff == 3);
    if (temBoss) spawnarBoss(mapa);

    spawnarItens();
}

// =============================================
// LOOP DE UMA FASE
// Retorna: true = venceu fase, false = game over
// =============================================
bool rodarFase(bool fromLoad = false) {
    bool venceu = false;
    bool bonusVitoriaAplicado = false;

    struct timespec req = {};

    while (state->session) {
        clock_t start = clock();

        inputHandler();
        verificarColetaItem();
        updateBomba();

        // Imunidade
        if (state->p1.tempoImune > 0) state->p1.tempoImune--;
        if (state->p2Ativo && state->p2.tempoImune > 0) state->p2.tempoImune--;

        // Atualiza inimigos
        for (Enemy& e : state->enemies) {
            bool estavaVivo = e.inimigoVivo;
            updateInimigo(e);
            if (estavaVivo && !e.inimigoVivo)
                state->hud.pontuacao += 50;
        }

        // Dano P1
        if (state->p1.alive && state->p1.tempoImune == 0) {
            bool napExplosao = (state->screenBuffer[state->p1.pos.y][state->p1.pos.x] == BOMBA_EXPLOSAO);
            bool napInimigo  = checkColisaoJogadorInimigo(false);
            if (napExplosao || napInimigo)
                aplicarDano(state->p1);
        }

        // Dano P2
        if (state->p2Ativo && state->p2Vivo && state->p2.alive && state->p2.tempoImune == 0) {
            bool napExplosao = (state->screenBuffer[state->p2.pos.y][state->p2.pos.x] == BOMBA_EXPLOSAO);
            bool napInimigo  = checkColisaoJogadorInimigo(true);
            if (napExplosao || napInimigo)
                aplicarDano(state->p2);
            if (!state->p2.alive) state->p2Vivo = false;
        }

        renderDraw();

        // Portal: P1 ou P2 chegou no portal
        bool p1NoPortal = state->p1.alive &&
            state->screenBuffer[state->p1.pos.y][state->p1.pos.x] == PORTAL_TILE;
        bool p2NoPortal = state->p2Ativo && state->p2Vivo &&
            state->screenBuffer[state->p2.pos.y][state->p2.pos.x] == PORTAL_TILE;

        if (p1NoPortal || p2NoPortal) {
            if (!bonusVitoriaAplicado) {
                state->hud.pontuacao += 200;
                bonusVitoriaAplicado = true;
            }
            venceu = true;
            state->session = false;
        }

        // Game over: AMBOS os jogadores mortos (ou só P1 em single player)
        bool p1Morto = !state->p1.alive;
        bool todosJogadoresMortos = p1Morto && (!state->p2Ativo || !state->p2Vivo);

        if (todosJogadoresMortos) {
            venceu = false;
            state->session = false;
        }

        // Vitória por eliminar todos
        if (!venceu && todosInimigosMortos()) {
            if (!bonusVitoriaAplicado) {
                state->hud.pontuacao += 200;
                bonusVitoriaAplicado = true;
            }
            venceu = true;
            state->session = false;
        }

        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        if (elapsed < FRAME_SPEED) {
            req.tv_sec  = 0;
            req.tv_nsec = (long)((FRAME_SPEED - elapsed) * 1e9);
            nanosleep(&req, nullptr);
        }
    }

    return venceu;
}

// =============================================
// LOOP COMPLETO DO JOGO (todas as fases)
// =============================================
void rodarJogo(int diff, bool p2Ativo, bool fromLoad = false) {
    int faseInicial = fromLoad ? state->faseAtual : 1;
    int pontos      = fromLoad ? state->hud.pontuacao : 0;
    bool p2AtivoLocal = fromLoad ? state->p2Ativo : p2Ativo;

    for (int fase = faseInicial; fase <= TOTAL_FASES; fase++) {
        if (!fromLoad) {
            inicializarFase(fase, diff, p2AtivoLocal, pontos);
        }
        fromLoad = false; // só a primeira iteração pode ser de load

        bool venceuFase = rodarFase();
        pontos = state->hud.pontuacao;

        // Salva stats antes de deletar
        int statPontos   = state->hud.pontuacao;
        int statMovs     = state->hud.movimentos;
        int statBombas   = state->hud.bombasUsadas;
        int statItens    = state->hud.itensPegos;

        if (!venceuFase) {
            // Game over
            delete state; state = nullptr;
            renderResult(false, false, statPontos, statMovs, statBombas, statItens);

            // Salva no ranking
            char nome[32] = "JOGADOR";
            RankEntry entry;
            strncpy(entry.nome, nome, 32);
            entry.pontos     = statPontos;
            entry.movimentos = statMovs;
            entry.bombas     = statBombas;
            entry.dificuldade = diff;
            salvarRanking(entry);
            return;
        }

        if (fase == TOTAL_FASES) {
            // Completou tudo!
            delete state; state = nullptr;
            renderResult(true, true, statPontos, statMovs, statBombas, statItens);

            char nome[32] = "VENCEDOR";
            RankEntry entry;
            strncpy(entry.nome, nome, 32);
            entry.pontos     = statPontos;
            entry.movimentos = statMovs;
            entry.bombas     = statBombas;
            entry.dificuldade = diff;
            salvarRanking(entry);
            return;
        }

        // Transição de fase: P2 ressuscita na próxima fase
        int nextFase = fase + 1;
        delete state; state = nullptr;

        renderTransicaoFase(nextFase);

        // Reinicia P2 na próxima fase (se estava ativo e morreu)
        inicializarFase(nextFase, diff, p2AtivoLocal, pontos);
        // Itens já foram resetados dentro de inicializarFase
    }
}

// =============================================
// MAIN
// =============================================
int main() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO c;
    GetConsoleCursorInfo(h, &c);
    c.bVisible = false;
    SetConsoleCursorInfo(h, &c);

    srand((unsigned)time(0));

    PlaySound(TEXT("musica.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

    bool executando  = true;
    int  gameDiff    = 1;
    bool p2Ativo     = false;
    bool iaAtiva     = false;

    while (executando) {
        int opcao = renderMenu(p2Ativo);

        switch (opcao) {
            case 1: // Novo jogo
                limparTela();
                rodarJogo(gameDiff, p2Ativo, false);
                break;

            case 2: // Carregar jogo
                if (loadGame(1)) {
                    rodarJogo(state->difficulty, state->p2Ativo, true);
                } else {
                    limparTela();
                    std::cout << "\n  Nenhum jogo salvo encontrado!\n";
                    Sleep(2000);
                }
                break;

            case 3: telaRanking();       break;
            case 4: telaDificuldade(gameDiff); break;
            case 5: telaComoJogar();     break;
            case 6: telaPontuacao();     break;
            case 7: executando = false;  break;
        }
    }

    PlaySound(NULL, NULL, 0);
    return 0;
}

// Handler de sinal para salvar ao fechar
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            if (state) { Beep(750, 300); saveGame(); }
            return FALSE;
        default:
            return FALSE;
    }
}
