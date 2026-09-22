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
        "R", "R'", "U", "U'", "F", "F'"
    };
    static constexpr std::string_view LeftieMoves[] = {
        "L'", "L", "U'", "U", "F'", "F"
    };
    public:
        AlgMani();
        std::string translate(const std::vector<Move>& alg);
        std::string translateLeftie(const std::vector<Move>& alg);
        std::vector<Move> translateBack(const std::string& alg);
        std::vector<Move> translateYX2(std::vector<Move> alg);
        std::vector<Move> reverse(std::vector<Move> alg);
        std::pair<std::vector<Move>, std::array<uint8_t, 3>> optimizeRUF(std::vector<Move> alg, std::array<uint8_t, 3> turns);
        std::pair<std::vector<Move>, std::array<uint8_t,3>> optimizeRF(std::vector<Move> alg, std::array<uint8_t, 3> turns);
        std::array<uint8_t, 3> getRUF(std::vector<Move> alg);
        std::vector<Move> swap(std::vector<Move> alg, uint8_t stay);
        std::vector<Move> twist(std::vector<Move> alg, bool dir);
};