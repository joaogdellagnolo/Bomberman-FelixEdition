#pragma once

#include <iostream>
#include "Render.h"
#include "Ranking.h"
#include <windows.h>
#include <conio.h>
using namespace std;

inline void drawBoxTitle(const string& title) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, COLOR_BOMB);
    cout << "========================================\n";
    cout << "            " << title << "\n";
    cout << "========================================\n\n";
    SetConsoleTextAttribute(h, COLOR_DEFAULT);
}

inline void waitEsc() {
    while (true) {
        if (_kbhit()) {
            char c = _getch();
            if (c == 27) return;
        }
        Sleep(50);
    }
}

inline void telaComoJogar() {
    limparTela();
    drawBoxTitle("COMO JOGAR");
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute(h, COLOR_PLAYER);
    cout << "  JOGADOR 1:\n";
    SetConsoleTextAttribute(h, COLOR_DEFAULT);
    cout << "  WASD  -> movimentacao\n";
    cout << "  E     -> coloca bomba\n";
    cout << "  P     -> salva jogo\n\n";

    SetConsoleTextAttribute(h, COLOR_PLAYER2);
    cout << "  JOGADOR 2 (Multiplayer):\n";
    SetConsoleTextAttribute(h, COLOR_DEFAULT);
    cout << "  Setas -> movimentacao\n";
    cout << "  ENTER -> coloca bomba\n\n";

    SetConsoleTextAttribute(h, COLOR_BOMB);
    cout << "  OBJETIVO:\n";
    SetConsoleTextAttribute(h, COLOR_DEFAULT);
    cout << "  Destrua caixas (#) e elimine inimigos (E)\n";
    cout << "  Chegue ao portal (X) para avancar de fase\n";
    cout << "  Colete itens para melhorar seu personagem\n";
    cout << "  Evite explosoes e inimigos!\n\n";

    cout << "  ESC para voltar\n";
    waitEsc();
}

inline void telaPontuacao() {
    limparTela();
    drawBoxTitle("PONTUACAO");
    cout << "  +50  inimigo eliminado\n";
    cout << "  +200 boss eliminado\n";
    cout << "  +200 passar de fase\n";
    cout << "  +10  item coletado\n";
    cout << "  -1   por movimento\n\n";
    cout << "  A pontuacao acumula entre fases.\n";
    cout << "  Itens resetam ao passar de fase.\n\n";
    cout << "  ESC para voltar\n";
    waitEsc();
}

inline void telaDificuldade(int& difficulty) {
    limparTela();
    int index = difficulty - 1;

    while (true) {
        renderCursorHandler();
        drawBoxTitle("DIFICULDADE");

        string opcoes[3] = {
            "Facil   - 3 inimigos, movimento aleatorio",
            "Medio   - 5 inimigos, 50% chance de perseguir",
            "Dificil - 7 inimigos, 75% + Boss nas fases 3 e 5"
        };

        for (int i = 0; i < 3; i++) {
            if (i == index) cout << " > ";
            else             cout << "   ";
            cout << opcoes[i] << "\n";
        }
        cout << "\nESC para voltar\n";

        if (_kbhit()) {
            char tecla = _getch();
            if (tecla == 'w' || tecla == 'W') index = (index + 2) % 3;
            if (tecla == 's' || tecla == 'S') index = (index + 1) % 3;
            if (tecla == 13) { difficulty = index + 1; }
            if (tecla == 27) return;
        }
        Sleep(60);
    }
}

inline void telaRanking() {
    limparTela();
    drawBoxTitle("RANKING");
    vector<RankEntry> ranking = carregarRanking();
    if (ranking.empty())
        cout << "  Nenhuma pontuacao salva\n";
    else
        desenharRankingRecursivo(ranking);
    cout << "\nESC para voltar\n";
    waitEsc();
}

// =========================================================
// Menu principal - seta cima/baixo para ativar/desativar P2
// =========================================================
inline int renderMenu(bool& p2Ativo) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    const int TOTAL_OPCOES = 7;
    std::string opcoes[TOTAL_OPCOES] = {
        "Novo Jogo", "Carregar Jogo", "Ranking",
        "Dificuldade", "Como Jogar", "Pontuacao", "Sair"
    };

    int selecionado = 0;
    limparTela();

    while (true) {
        renderCursorHandler();
        drawBoxTitle("BOMBERMAN");

        SetConsoleTextAttribute(h, COLOR_DEFAULT);
        std::cout << "  W/S = navegar  ENTER = selecionar\n";

        // Status do multiplayer com setas (conforme requisito)
        SetConsoleTextAttribute(h, p2Ativo ? COLOR_PLAYER2 : COLOR_DEFAULT);
        std::cout << "  Seta CIMA = ativar P2  |  Seta BAIXO = desativar P2\n";
        std::cout << "  Multiplayer: " << (p2Ativo ? "[ATIVADO - 2 Jogadores]" : "[DESATIVADO - 1 Jogador]") << "\n\n";
        SetConsoleTextAttribute(h, COLOR_DEFAULT);

        for (int i = 0; i < TOTAL_OPCOES; i++) {
            if (i == selecionado) {
                SetConsoleTextAttribute(h, COLOR_PLAYER);
                std::cout << " > " << opcoes[i] << "\n";
            } else {
                SetConsoleTextAttribute(h, COLOR_DEFAULT);
                std::cout << "   " << opcoes[i] << "\n";
            }
        }
        std::cout << "\n========================================\n";
        SetConsoleTextAttribute(h, COLOR_DEFAULT);

        if (_kbhit()) {
            char tecla = _getch();

            if (tecla == 'w' || tecla == 'W') {
                selecionado--;
                if (selecionado < 0) selecionado = TOTAL_OPCOES - 1;
            } else if (tecla == 's' || tecla == 'S') {
                selecionado++;
                if (selecionado >= TOTAL_OPCOES) selecionado = 0;
            } else if (tecla == 13) {
                return selecionado + 1;
            } else if (tecla == 0 || tecla == (char)224) {
                // teclas especiais (setas)
                char ext = _getch();
                if (ext == 72) { // seta cima -> ativa P2
                    p2Ativo = true;
                    limparTela();
                } else if (ext == 80) { // seta baixo -> desativa P2
                    p2Ativo = false;
                    limparTela();
                }
            }
        }
        Sleep(50);
    }
}
