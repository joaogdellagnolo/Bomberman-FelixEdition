// =============================================
// Trabalho M1 - Bomberman
// Algoritmos e Programação II
// Desenvolvido por:  Joao Felix, Derick Kunz, Joao Guilherme, Eduardo Loyola
// =============================================

#include "BomberMan.h"
#include "Mapas.h"
#include "inimigo.h"
#include "Bomba.h"

#include <signal.h>
#include <iostream>
#include <conio.h>
#include <cstdlib>
#include <ctime>



// verifica se tem o target na posicao dejesada(poderia ser apenas a posicao dejesada mais pode ser util ter a posição atual de quem pediu o check)
bool checkColisao(int target, int posX, int posY, int offX, int offY) {
    int destX = posX + offX;
    int destY = posY + offY;

    // evita acessar fora do mapa
    if (destX < 0 || destX >= wMax || destY < 0 || destY >= hMax)
        return true;

    return state->screenBuffer[destY][destX] == target;
}


// coloca bomba onde o jogador ta
void colocarBomba() {

    // apenas uma bomba
    if (state->bomba.temBomba || state->bomba.explodindo)
        return;

    state->bomba.temBomba = true;
    state->bomba.pos = state->p1.pos;
    state->bomba.tempoBomba = 0;

    state->screenBuffer[state->bomba.pos.y][state->bomba.pos.x] = BOMBA;
    state->hud.bombasUsadas++;
}


// cuida da explosao (criar e apagar)
void gerenciarExplosao(int tipoTile) {
    int bx = state->bomba.pos.x;
    int by = state->bomba.pos.y;

    // centro
    if (state->screenBuffer[by][bx] != BLOCO_SOLIDO)
        state->screenBuffer[by][bx] = tipoTile;

    // cima
    if (by > 0 && state->screenBuffer[by - 1][bx] != BLOCO_SOLIDO)
        state->screenBuffer[by - 1][bx] = tipoTile;

    // baixo
    if (by < hMax && state->screenBuffer[by + 1][bx] != BLOCO_SOLIDO)
        state->screenBuffer[by + 1][bx] = tipoTile;

    // esquerda
    if (bx > 0 && state->screenBuffer[by][bx - 1] != BLOCO_SOLIDO)
        state->screenBuffer[by][bx - 1] = tipoTile;

    // direita
    if (bx < wMax && state->screenBuffer[by][bx + 1] != BLOCO_SOLIDO)
        state->screenBuffer[by][bx + 1] = tipoTile;
}


// controla o tempo da bomba
void updateBomba() {

    // esperando explodir
    if (state->bomba.temBomba && !state->bomba.explodindo) {
        state->bomba.tempoBomba++;

        if (state->bomba.tempoBomba >= TICKS_UNTIL_NEXT_STATE_BOMB) {
            gerenciarExplosao(BOMBA_EXPLOSAO);

            state->bomba.temBomba = false;
            state->bomba.explodindo = true;
            state->bomba.cooldownBomba = 0;
        }
    }

    // explosao ativa
    if (state->bomba.explodindo) {
        state->bomba.cooldownBomba++;

        if (state->bomba.cooldownBomba > 20) {
            gerenciarExplosao(WHITE); // limpa fogo
            state->bomba.explodindo = false;
        }
    }
}


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


// ve se jogador bateu em inimigo
bool checkColisaoJogadorInimigo() {
    for (const Enemy& e : state->enemies) {
        if (e.inimigoVivo &&
            e.pos.x == state->p1.pos.x &&
            e.pos.y == state->p1.pos.y) {
            return true;
        }
    }
    return false;
}


// ganhou o jogo?
bool todosInimigosMortos() {
    for (const Enemy& e : state->enemies) {
        if (e.inimigoVivo)
            return false;
    }
    return true;
}


// entrada do teclado
void inputHandler() {

    if (!_kbhit())
        return;

    char tecla = _getch();

    int dx = 0;
    int dy = 0;

    switch (tecla) {
        case 'w': case 'W': dy = -1; break;
        case 's': case 'S': dy =  1; break;
        case 'a': case 'A': dx = -1; break;
        case 'd': case 'D': dx =  1; break;
        case 'e': case 'E': colocarBomba(); break;
        case 't': case 'T': state->session = false; break;
    }

    if (dx != 0 || dy != 0) {

        bool parede1 = checkColisao(BLOCO_SOLIDO, state->p1.pos.x, state->p1.pos.y, dx, dy);
        bool parede2 = checkColisao(PAREDE_DESTRUTIVEL, state->p1.pos.x, state->p1.pos.y, dx, dy);

        if (!parede1 && !parede2) {
            state->p1.pos.x += dx;
            state->p1.pos.y += dy;
            state->hud.movimentos++;
        }
    }
}

char sortearTipoItem()
{
    char tipos[] = {'F', 'B', 'V', 'R', 'E', 'P'};
    return tipos[rand() % 6];
}

void spawnarItens()
{
    int quantidade = (rand() % 6) + 2; // 2 até 7 itens

    for (int i = 0; i < quantidade; i++)
    {
        int x, y;

        do {
            x = rand() % wMax;
            y = rand() % hMax;

        } while (
            screenBuffer[y][x] != WHITE ||
            (x == state.p1.pos.x && y == state.p1.pos.y)
        );

        char simbolo = sortearTipoItem();

        state.itens.push_back(Item(x, y, simbolo));
    }
}

void renderizarItens()
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    for (Item& item : state.itens)
    {
        if (!item.ativo)
            continue;

        COORD coord;
        coord.X = item.pos.x;
        coord.Y = item.pos.y;
        SetConsoleCursorPosition(h, coord);

        SetConsoleTextAttribute(h, COLOR_BOMB);
        std::cout << item.simbolo;
    }

    SetConsoleTextAttribute(h, COLOR_DEFAULT);
}

void verificarColetaItem()
{
    for (Item& item : state.itens)
    {
        if (!item.ativo)
            continue;

        if (item.pos.x == state.p1.pos.x && item.pos.y == state.p1.pos.y)
        {
            item.ativo = false;

            if (item.simbolo == 'F')
                state.hud.itemFogo++;

            else if (item.simbolo == 'B')
                state.hud.itemBombas++;

            else if (item.simbolo == 'V')
                state.hud.itemVidaExtra++;

            else if (item.simbolo == 'R')
                state.hud.itemBombaRelogio++;

            else if (item.simbolo == 'E')
                state.hud.itemEscudo++;

            else if (item.simbolo == 'P')
                state.hud.itemPassaBlocos++;
        }
    }
}


// menu
int exibirMenu() {

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    limparTela();
    SetConsoleTextAttribute(h, COLOR_BOMB);
    std::cout << "======================\n";
    std::cout << "\nBOMBERMAN\n";
    std::cout << "\n======================\n\n";
    SetConsoleTextAttribute(h, COLOR_DEFAULT);
    std::cout << "WASD = mover\n";
    std::cout << "E = bomba\n";
    std::cout << "T = sair\n\n";

    std::cout << "1 jogar\n";
    std::cout << "0 sair\n\n";

    int op;
    std::cin >> op;

    return op;
}

static const char* savePath = "saves/savegame.dk";
void saveGame() 
{
    FILE* file = fopen(savePath, "wb");
    if (file) {
        fwrite(&state, sizeof(GameState), 1, file);
    }
    fclose(file);   
}

GameState loadGame() 
{
    GameState loadedState;
    FILE* file = fopen(savePath, "rb");
    if (file)
        fread(&loadedState, sizeof(GameState), 1, file);
    else
        loadedState = GameState();
    
    fclose(file);
}

// loop principal
void rodarJogo(int mapa[][wMax]) {
    state->session = true;
    state->hud.inicioJogo = time(nullptr);

    // copia mapa
    for (int i = 0; i < hMax; i++)
        for (int j = 0; j < wMax; j++)
            state->screenBuffer[i][j] = mapa[i][j];

    criarInimigo(1, 7);
    criarInimigo(15, 5);
    criarInimigo(3, 3);
    criarInimigo(20, 10);
    criarInimigo(9, 6);

    spawnarItens();

    bool venceu = false;

    while (state->session) 
    {
        state->timestamp = time(nullptr);

        inputHandler();
        verificarColetaItem();
        updateBomba();

        for (Enemy& e : state->enemies)
            updateInimigo(e);

        // morreu na explosao
        if (state->screenBuffer[state->p1.pos.y][state->p1.pos.x] == BOMBA_EXPLOSAO)
            state->p1.alive = false;

        // morreu no inimigo
        if (checkColisaoJogadorInimigo())
            state->p1.alive = false;

        std::vector<std::pair<int,int>> vivos;

        for (const Enemy& e : state->enemies) {
            if (e.inimigoVivo)
                vivos.push_back({e.pos.x, e.pos.y});
        }

        renderDraw();
	renderizarItens();

        if (!state->p1.alive) 
        {
            venceu = false;
            state->session = false;
        }
        else if (todosInimigosMortos()) 
        {
            venceu = true;
            state->session = false;
        }
    }
    renderResult(venceu);
}


// main
int main() {

    // esconde cursor 
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO c;
    GetConsoleCursorInfo(h, &c);
    c.bVisible = false;
    SetConsoleCursorInfo(h, &c);

    srand(time(0));

    int op = -1;

    while (op != 0) {
        op = renderMenu(); // Corrigido de exibirMenu para RenderMenu conforme definido em Render.h

        if (op == 1)
        {
            limparTela();
            rodarJogo(map_0);
        }
    }

    limparTela();
    std::cout << "\n Fechando...\n";

    return 0;
}   

//controle de sinais de interrupt para poder salvar o jogo ao fechar
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal.
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        {    
            Beep(750, 500);
            saveGame();
            limparTela();
            return FALSE;
        }
        default:
            return FALSE;
    }
}



