#include "BomberMan.h"
#include "Menu.h"

#include <chrono>
#include <thread>
#include <cstring>
#include <cstdlib>

// =============================================
// Trabalho M1 - Bomberman
// Algoritmos e Programação II
// Desenvolvido por:  Joao Felix, Derick Kunz, Joao Guilherme, Eduardo Loyola
// =============================================

// main
int main() 
{
    // esconde cursor 
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO c;
    GetConsoleCursorInfo(h, &c);
    c.bVisible = false;
    SetConsoleCursorInfo(h, &c);

    srand(time(0));

    while (currentRoute != EXIT) 
    {
        auto start = high_resolution_clock::now();

        currentWidget->Run();

        auto end = high_resolution_clock::now();

        duration<double> elapsed = end - start;

        if (elapsed.count() < SPEED)
        {
            std::this_thread::sleep_for(duration<double>(SPEED - elapsed.count()));
        }
    }

    limparTela();
    return 0;
}   


