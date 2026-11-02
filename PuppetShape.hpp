#pragma once //???
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <utility>
#include <cmath>
#include <array>
#include <algorithm>

#include "Move.hpp"

// shape representation

class PuppetShape {

    // goal state of the shape 
    static constexpr std::array<uint8_t, 125> goal_cubies = {
        7,7,6,6,0, 7,7,6,6,0, 3,3,2,2,0, 3,3,2,2,0, 0,0,0,0,0,
        7,7,6,6,0, 7,7,0,6,0, 3,0,0,0,0, 3,3,0,2,0, 0,0,0,0,0,
        5,5,4,4,0, 5,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 0,0,0,0,0,
        5,5,4,4,0, 5,5,0,4,0, 1,0,0,0,0, 1,1,0,0,0, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0
    };

    static constexpr uint8_t cycle_idxs[3][15][4] = { // 3 faces; 15 different cycles happen on a turn; 4 cubies per cycle
        {   // R
            {31, 41, 91, 81}, 
            {30, 40, 90, 80}, {35, 65, 85, 55},
            {0,  20,120,100}, {5,  45,115, 75}, {10, 70,110, 50}, {15, 95,105, 25},
            {1,  21,121,101}, {6,  46,116, 76}, {11, 71,111, 51}, {16, 96,106, 26},
            {2,  22,122,102}, {7,  47,117, 77}, {12, 72,112, 52}, {17, 97,107, 27}
        },
        {   // U
            {31, 33, 43, 41}, 
            {6,  8,  18, 16}, {7,  13, 17, 11},
            {0,  4,  24, 20}, {1,  9,  23, 15}, {2,  14, 22, 10}, {3,  19, 21, 5},
            {25, 29, 49, 45}, {26, 34, 48, 40}, {27, 39, 47, 35}, {28, 44, 46, 30},
            {50, 54, 74, 70}, {51, 59, 73, 65}, {52, 64, 72, 60}, {53, 69, 71, 55}
        },
        {   // F
            {31, 81, 83, 33}, 
            {26, 76, 78, 28}, {51, 77, 53, 27},
            {0, 100,104, 4},  {25,101, 79, 3},  {50,102, 54, 2},  {75,103, 29, 1},
            {5, 105,109, 9},  {30,106, 84, 8},  {55,107, 59, 7},  {80,108, 34, 6},
            {10,110,114, 14}, {35,111, 89, 13}, {60,112, 64, 12}, {85,113, 39, 11}
        }
    };

    static constexpr uint8_t roll[3][4] = { // 3 different possible rotatios and 4 cubies per cycle
        {3,0,1,2}, // clockwise
        {1,2,3,0}, // counterclockwise
        {2,3,0,1}  // double
    };

    // blocking only happens in the cycles with index 7 and onward
    static constexpr uint8_t BLOCKING_IDX = 7;

    // the cycle at index 0 represents the cubies of the corners in the 3x3x3 base cube.
    // these determine which cubies move on a certain turn
    static constexpr uint8_t INNER_COR_IDX = 0;  

    public:
        struct Shape{
            std::array<uint8_t, 125> cubies;
            bool operator==(const Shape& other) const {
                return cubies == other.cubies;
            }
        };
        PuppetShape();
        void getCubeShape(Shape& shape);
        bool turn(Shape& shape, Move m);
};