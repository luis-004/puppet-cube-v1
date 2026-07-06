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

class PuppetB {
    static constexpr uint8_t NUM_CORNERS = 7;
    static constexpr uint8_t NUM_EDGES = 9;
    static constexpr uint8_t NUM_MOVES = 9;
    static constexpr uint8_t FLIP_BIT = 4;
    static constexpr uint8_t NUM_CORNER_VALS = 21;
    static constexpr uint8_t NUM_EDGE_VALS = 18;

    static constexpr std::array<uint8_t, NUM_CORNERS> solved_corners = {0, 1, 2, 3, 4, 5, 6};
    static constexpr std::array<uint8_t, NUM_EDGES> solved_edges = {0, 1, 2, 3, 4, 5, 6, 7, 8};

    static constexpr uint8_t corners[NUM_MOVES][NUM_CORNER_VALS] = { // TODO
        {1, 2, 7, 6}, // R
        {2, 0, 3, 8}, // U
        {0, 1, 5, 4}, // F
        {1, 2, 7, 6}, // R'
        {2, 0, 3, 8}, // U'
        {0, 1, 5, 4}, // F'
        {1, 2, 7, 6}, // R2
        {2, 0, 3, 8}, // U2
        {0, 1, 5, 4}  // F2
    };

    static constexpr uint8_t edges[NUM_MOVES][NUM_EDGE_VALS] = { // TODO
        {1, 2, 7, 6}, // R
        {2, 0, 3, 8}, // U
        {0, 1, 5, 4}, // F
        {1, 2, 7, 6}, // R'
        {2, 0, 3, 8}, // U'
        {0, 1, 5, 4}, // F'
        {1, 2, 7, 6}, // R2
        {2, 0, 3, 8}, // U2
        {0, 1, 5, 4}  // F2
    };

    // counter clockwise rotation
    static constexpr std::array<uint8_t, NUM_CORNER_VALS> rotation_table = {0,  3,  4,  5,  6,  1,  2,  9, 10, 11, 12,  7,  8, 15, 16, 17, 18, 13, 14};

    // mirror over R-axis
    static constexpr std::array<uint8_t, NUM_CORNER_VALS> mirror_table = {0,  1,  6,  5,  4,  3,  2, 13, 18, 17, 16, 15, 14,  7, 12, 11, 10,  9,  8};

    static constexpr std::array<uint8_t, NUM_CORNER_VALS> idx_in_type = {0, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2}; // idx problematic; different meaning than before
    static constexpr std::array<uint8_t, NUM_CORNER_VALS> orientation = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2};

    static constexpr uint8_t corner_perms[3][3] = {
        {255, 0, 1},
        {2, 255, 3},
        {4, 5, 255}
    };

    inline static const auto from7choose3 = [] {
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

    public:
        struct State{
            std::array<uint8_t, NUM_CORNERS> corners;
            std::array<uint8_t, NUM_EDGES> edges;
            bool operator==(const State& other) const{
                return (corners == other.corners) && (edges == other.edges);
            }
        };
        PuppetB();
        void getSolved(State& state);
        bool testSolved(const State& state);
        void turn(State& state, uint8_t face, uint8_t dir);
        uint32_t getShapeHash(const State&);
        uint8_t getCornerIdx(const State& state);
        std::pair<uint32_t, uint8_t> getEdgeIdxs(const State& state);
        void rotateCorners(State& state);
        void mirrorCorners(State& state);
        void solveParity(State& state);
};