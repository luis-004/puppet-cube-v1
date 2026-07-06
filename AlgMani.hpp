#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <utility>
#include <cmath>
#include <array>
#include <sstream>
#include <algorithm>

#include "Move.hpp"

class AlgMani {
    static constexpr std::string_view Moves[] = {
        "R", "U", "F", "R'", "U'", "F'", "R2", "U2", "F2"
    };
    static constexpr std::string_view LeftieMoves[] = {
        "L'", "U'", "F'", "L", "U", "F", "L2", "U2", "F2"
    };
    static constexpr uint8_t rev_dir[3] = {1, 0, 2};
    public:
        AlgMani();
        std::string translate(const std::vector<Move>& alg);
        std::string translateLeftie(const std::vector<Move>& alg);
        std::vector<Move> translateBack(const std::string& alg);
        std::vector<Move> translateYX2(std::vector<Move> alg);
        std::vector<Move> reverse(std::vector<Move> alg);
        std::pair<std::vector<Move>, std::array<uint8_t, 6>> optimizeRUF(std::vector<Move> alg, std::array<uint8_t, 6> turns);
        std::pair<std::vector<Move>, std::array<uint8_t, 6>> optimizeRF(std::vector<Move> alg, std::array<uint8_t, 6> turns);
        std::array<uint8_t, 6> getRUF(std::vector<Move> alg);
        std::vector<Move> swap(std::vector<Move> alg, uint8_t stay);
        std::vector<Move> twist(std::vector<Move> alg, bool dir);
};