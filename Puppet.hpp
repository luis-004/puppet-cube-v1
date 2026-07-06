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

class Puppet {
    static constexpr uint8_t NUM_CORNERS = 7;
    static constexpr uint8_t NUM_EDGES = 9;
    static constexpr uint8_t FLIP_BIT = 4;
    static constexpr uint8_t NUM_CORNER_VALS = 19;
    static constexpr uint8_t RANGE_EDGE_VALS = 25;

    static constexpr std::array<uint8_t, NUM_CORNERS> solved_corners = {0, 1, 2, 3, 4, 5, 6};
    static constexpr std::array<uint8_t, NUM_EDGES> solved_edges = {0, 1, 2, 3, 4, 5, 6, 7, 8};

    static constexpr uint8_t permute_corners[3][4] = { // [face][index]
        {0, 2, 1, 6}, // R
        {0, 4, 3, 2}, // U
        {0, 6, 5, 4}  // F
    };

    static constexpr uint8_t permute_edges[3][4] = { // [face][index]
        {1, 2, 7, 6}, // R
        {2, 0, 3, 8}, // U
        {0, 1, 5, 4}  // F
    };

    static constexpr uint8_t roll[3][4] = { // [dir][position in cycle]
        {3, 0, 1, 2},
        {1, 2, 3, 0},
        {2, 3, 0, 1}
    };

    static constexpr uint8_t twist_corners[3][3][4] = { // [face][dir][twist]
        {{2, 2, 1, 1}, {1, 2, 2, 1}, {0, 1, 0, 2}},
        {{0, 1, 1, 1}, {2, 2, 2, 0}, {1, 1, 2, 2}},
        {{1, 0, 1, 1}, {0, 2, 2, 2}, {2, 1, 1, 2}}
    };

    static constexpr uint8_t edge_flips[3][3][2] = { // [face][dir][edge]
        {{1, 2}, {1, 6}, {1, 7}}, 
        {{2, 0}, {2, 8}, {2, 3}},
        {{0, 1}, {0, 4}, {0, 5}}
    };

    static constexpr uint8_t twist_table[3][NUM_CORNER_VALS] = { // [twist][corner]
        {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18}, // dont change orientation
        {0,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  1,  2,  3,  4,  5,  6}, // twist counterclockwise
        {0, 13, 14, 15, 16, 17, 18,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12}  // twist clockwise
    };

    // counter clockwise rotation
    static constexpr std::array<uint8_t, NUM_CORNER_VALS> rotation_corners = {0,  3,  4,  5,  6,  1,  2,  9, 10, 11, 12,  7,  8, 15, 16, 17, 18, 13, 14};
    static constexpr std::array<uint8_t, RANGE_EDGE_VALS> rotation_edges = {1, 2, 0, 5, 6, 7, 8, 3, 4, 0, 0, 0, 0, 0, 0, 0, 17, 18, 16, 21, 22, 23, 24, 19, 20};

    // mirror over R-axis
    static constexpr std::array<uint8_t, NUM_CORNER_VALS> mirror_corners = {0,  1,  6,  5,  4,  3,  2, 13, 18, 17, 16, 15, 14,  7, 12, 11, 10,  9,  8};
    static constexpr std::array<uint8_t, RANGE_EDGE_VALS> mirror_edges = {0, 2, 1, 4, 3, 8, 7, 6, 5, 0, 0, 0, 0, 0, 0, 0, 16, 18, 17, 20, 19, 24, 23, 22, 21};

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
        Puppet();
        void getSolved(State& state);
        bool testSolved(const State& state);
        void turn(State& state, uint8_t face, uint8_t dir);
        uint32_t getShapeHash(const State&);
        uint8_t getCornerIdx(const State& state);
        uint32_t getEPIdx(const State& state);
        uint8_t getFlipIdx(const State& state);
        std::pair<uint32_t, uint8_t> getEdgeIdxs(const State& state);
        void rotateCorners(State& state);
        void mirrorCorners(State& state);
        void rotateEdges(State& state);
        void mirrorEdges(State& state);
        void solveParity(State& state);
};