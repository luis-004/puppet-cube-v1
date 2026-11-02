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

// module to handle and manipulate sequences of moves (also called algorithms)

class AlgMani {
    static constexpr std::string_view Moves[] = {
        "R", "R'", "R2", "U", "U'", "U2", "F", "F'", "F2"
    };
    static constexpr std::string_view LeftieMoves[] = {
        "L'", "L", "L2", "U'", "U", "U2", "F'", "F", "F2"
    };
    static constexpr uint8_t rev_dir[3] = {1, 0, 2};
    public:
        AlgMani();

        // translate from Move enum sequence to the standard notation
        std::string translate(const std::vector<Move>& alg);
        std::string translateLeftie(const std::vector<Move>& alg);

        // translate from standard notation into Move enums 
        std::vector<Move> translateBack(const std::string& alg);

        // reverse the sequence
        std::vector<Move> reverse(std::vector<Move> alg);

        // other methods for additional purposes irrelevant for the work

        // transfer the sequence into the viewing angle from which the cube is best to turn physically 
        std::vector<Move> translateYX2(std::vector<Move> alg);

        // optimizing sequences for physical turning
        std::pair<std::vector<Move>, std::array<uint8_t, 6>> optimizeRUF(std::vector<Move> alg, std::array<uint8_t, 6> turns);
        std::pair<std::vector<Move>, std::array<uint8_t, 6>> optimizeRF(std::vector<Move> alg, std::array<uint8_t, 6> turns);
        std::array<uint8_t, 6> getRUF(std::vector<Move> alg);

        // swap (mirror) faces of the sequence and twist (rotate) faces around
        std::vector<Move> swap(std::vector<Move> alg, uint8_t stay);
        std::vector<Move> twist(std::vector<Move> alg, bool dir);
};