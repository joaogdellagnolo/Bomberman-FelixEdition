#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>

struct RankEntry {
    char nome[32];
    int pontos;
    int movimentos;
    int bombas;
    int dificuldade;
};

inline void salvarRanking(const RankEntry& entry)
{
    std::ofstream file("ranking.dat", std::ios::binary | std::ios::app);

    if (!file.is_open())
        return;

    file.write(reinterpret_cast<const char*>(&entry), sizeof(RankEntry));
    file.close();
}

inline std::vector<RankEntry> carregarRanking()
{
    std::vector<RankEntry> ranking;

    std::ifstream file("ranking.dat", std::ios::binary);

    if (!file.is_open())
        return ranking;

    RankEntry entry;

    while (file.read(reinterpret_cast<char*>(&entry), sizeof(RankEntry))) {
        ranking.push_back(entry);
    }

    std::sort(ranking.begin(), ranking.end(),
    [](const RankEntry& a, const RankEntry& b)
    {
        return a.pontos > b.pontos;
    });

    return ranking;
}

inline void desenharRankingRecursivo(const std::vector<RankEntry>& ranking, int index = 0)
{
    if (index >= ranking.size() || index >= 10)
        return;

    const RankEntry& r = ranking[index];

    std::cout
        << " " << index + 1 << ". "
        << r.nome
        << " | " << r.pontos << " pts"
        << " | Dificuldade: " << r.dificuldade
        << std::endl;

    desenharRankingRecursivo(ranking, index + 1);
}
