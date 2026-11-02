#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <utility>
#include <cmath>
#include <array>
#include <algorithm>
#include <cassert>
#include <bitset>

#include "Move.hpp"

// state representation of the Puppet Cube

class Puppet{
    static constexpr uint8_t NUM_CORNERS = 7;
    static constexpr uint8_t NUM_EDGES = 9;
    static constexpr uint8_t NUM_MOVES = 9;
    static constexpr uint8_t FLIP_BIT = 4;
    static constexpr uint8_t NUM_CORNER_VALS = 21;
    static constexpr uint8_t RANGE_EDGE_VALS = 25;

    // defiition of the goal state
    static constexpr std::array<uint8_t, NUM_CORNERS> solved_corners = {0, 1, 2, 3, 4, 5, 6};
    static constexpr std::array<uint8_t, NUM_EDGES> solved_edges = {0, 1, 2, 3, 4, 5, 6, 7, 8};

    static constexpr uint8_t move_corners[NUM_MOVES][NUM_CORNER_VALS] = {
        {17, 14, 8, 9, 4, 5, 6, 3, 0, 15, 16, 11, 12, 13, 10, 7, 1, 2, 18, 19, 20}, // R
        {8, 16, 17, 7, 4, 5, 6, 15, 2, 3, 14, 11, 12, 13, 1, 9, 10, 0, 18, 19, 20}, // R'
        {2, 10, 0, 15, 4, 5, 6, 9, 17, 7, 1, 11, 12, 13, 16, 3, 14, 8, 18, 19, 20}, // R2
        {12, 1, 2, 0, 10, 11, 6, 19, 8, 9, 7, 17, 18, 13, 5, 15, 16, 14, 3, 4, 20}, // U
        {3, 1, 2, 18, 19, 14, 6, 10, 8, 9, 4, 5, 0, 13, 17, 15, 16, 11, 12, 7, 20}, // U'
        {18, 1, 2, 12, 7, 17, 6, 4, 8, 9, 19, 14, 3, 13, 11, 15, 16, 5, 0, 10, 20}, // U2
        {1, 13, 2, 3, 4, 7, 12, 8, 20, 9, 10, 11, 14, 19, 15, 6, 16, 17, 18, 0, 5}, // F
        {19, 0, 2, 3, 4, 20, 15, 5, 7, 9, 10, 11, 6, 1, 12, 14, 16, 17, 18, 13, 8}, // F'
        {13, 19, 2, 3, 4, 8, 14, 20, 5, 9, 10, 11, 15, 0, 6, 12, 16, 17, 18, 1, 7}  // F2
    };

    // one independent bit is used to indicate the orientation of an edge, so the values aren't continuous. 
    static constexpr uint8_t move_edges[NUM_MOVES][RANGE_EDGE_VALS] = {
        {0, 18, 7, 3, 4, 5, 1, 22, 8, 255, 255, 255, 255, 255, 255, 255, 16, 2, 23, 19, 20, 21, 17, 6, 24}, // R
        {0, 6, 17, 3, 4, 5, 23, 2, 8, 255, 255, 255, 255, 255, 255, 255, 16, 22, 1, 19, 20, 21, 7, 18, 24}, // R'
        {0, 23, 22, 3, 4, 5, 18, 17, 8, 255, 255, 255, 255, 255, 255, 255, 16, 7, 6, 19, 20, 21, 2, 1, 24}, // R2
        {3, 1, 16, 24, 4, 5, 6, 7, 2, 255, 255, 255, 255, 255, 255, 255, 19, 17, 0, 8, 20, 21, 22, 23, 18}, // U
        {18, 1, 8, 0, 4, 5, 6, 7, 19, 255, 255, 255, 255, 255, 255, 255, 2, 17, 24, 16, 20, 21, 22, 23, 3}, // U'
        {24, 1, 19, 18, 4, 5, 6, 7, 16, 255, 255, 255, 255, 255, 255, 255, 8, 17, 3, 2, 20, 21, 22, 23, 0}, // U2
        {17, 5, 2, 3, 0, 20, 6, 7, 8, 255, 255, 255, 255, 255, 255, 255, 1, 21, 18, 19, 16, 4, 22, 23, 24}, // F
        {4, 16, 2, 3, 21, 1, 6, 7, 8, 255, 255, 255, 255, 255, 255, 255, 20, 0, 18, 19, 5, 17, 22, 23, 24}, // F'
        {21, 20, 2, 3, 17, 16, 6, 7, 8, 255, 255, 255, 255, 255, 255, 255, 5, 4, 18, 19, 1, 0, 22, 23, 24}  // F2
    };

    // arrays to compute the ranks
    static constexpr std::array<uint8_t, NUM_CORNER_VALS> position =    {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6};
    static constexpr std::array<uint8_t, NUM_CORNER_VALS> orientation = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2};
    static constexpr std::array<uint8_t, 8> corner_perms = {0, 1, 255, 2, 3, 255, 4, 5};
    static constexpr std::array<uint16_t, 7> pow3 = {0, 1, 3, 9, 27, 81, 243};

    // the "CornerDistributionIndex"; gets the position of the solid corner and an index representing the permutation of the hollow corners.
    // outputs a continuous index
    inline static const auto from6choose3 = [] {
        std::array<std::array<uint8_t, 128>, 7> table{};
        for (auto& row : table) {
            row.fill(255);
        }
        uint8_t idx = 0;
        for(int i = 0; i < NUM_CORNERS; i++){
            for(int j = 0; j < NUM_CORNERS; j++){
                for(int k = 0; k < NUM_CORNERS; k++){
                    for(int zeroCor = 0; zeroCor < NUM_CORNERS; zeroCor++){
                        if(zeroCor != i && zeroCor != j && zeroCor != k && i < j && j < k){
                            uint8_t permutation = 0;
                            permutation |= (1u << i);
                            permutation |= (1u << j);
                            permutation |= (1u << k);
                            table[zeroCor][permutation] = idx++;
                        }
                    }
                }
            }
        }
        return table;
    }();

    // to calculate symmetries:
    // counter clockwise rotation
    static constexpr std::array<uint8_t, NUM_CORNER_VALS> rotation_corners = {7, 3, 4, 5, 6, 1, 2, 14, 10, 11, 12, 13, 8, 9, 0, 17, 18, 19, 20, 15, 16};
    static constexpr std::array<uint8_t, RANGE_EDGE_VALS> rotation_edges = {1, 2, 0, 5, 6, 7, 8, 3, 4, 255, 255, 255, 255, 255, 255, 255, 17, 18, 16, 21, 22, 23, 24, 19, 20};

    // mirror over R-axis
    static constexpr std::array<uint8_t, NUM_CORNER_VALS> mirror_corners = {0, 3, 2, 1, 6, 5, 4, 14, 17, 16, 15, 20, 19, 18, 7, 10, 9, 8, 13, 12, 11};
    static constexpr std::array<uint8_t, RANGE_EDGE_VALS> mirror_edges = {0, 2, 1, 4, 3, 8, 7, 6, 5, 255, 255, 255, 255, 255, 255, 255, 16, 18, 17, 20, 19, 24, 23, 22, 21};

    public:
        struct State{
            std::array<uint8_t, NUM_CORNERS> corners;
            std::array<uint8_t, NUM_EDGES> edges;
            bool operator==(const State& other) const{
                return (corners == other.corners) && (edges == other.edges);
            }
        };
        Puppet();
        void getSolved(State& state);
        bool testSolved(const State& state);

        void turn(State& state, Move m);

        // extract ranks and keys
        uint32_t getShapeKey(const State&);
        uint8_t getCornerRank(const State& state);
        uint32_t getEPRank(const State& state);
        uint8_t getEORank(const State& state);
        std::pair<uint32_t, uint8_t> getEdgeRanks(const State& state);

        // methods to compute symmetrically similar states (used to exploit the 6-fold symmetry)
        void rotate(State& state);
        void mirror(State& state);

        // function to reconstruct a state out of chosen ranks. Such a state could have odd parity and be therefore
        // not normally solvable. Therefore, fixParity checks for parity and, if it's odd, 
        // swaps two edges in the state to make it solvable 
        State stateFromRanks(uint32_t shape_key, uint8_t corner_rank, uint32_t edge_rank, uint8_t flip_rank);
        bool fixParity(State& state);
};