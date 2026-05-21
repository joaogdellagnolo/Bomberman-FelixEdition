#include "Menu.h"
#include "Render.h"
// menu
int Menu::RenderDraw()
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    Render::limparTela();
    Render::SetConsoleTextAttribute(h, COLOR_BOMB);
    std::cout << "======================\n";
    std::cout << "\nBOMBERMAN\n";
    std::cout << "\n======================\n\n";
    Render::SetConsoleTextAttribute(h, COLOR_DEFAULT);
    std::cout << "WASD = mover\n";
    std::cout << "E = bomba\n";
    std::cout << "T = sair\n\n";

    std::cout << "1 jogar\n";
    std::cout << "0 sair\n\n";
    return 0;
}

void Menu::Run() 
{
    int op;
    std::cin >> op;

    return op;
}