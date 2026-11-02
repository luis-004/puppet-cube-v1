#include "PuppetShape.hpp"
PuppetShape::PuppetShape(){}

void PuppetShape::getCubeShape(Shape& shape){
    shape.cubies = goal_cubies;
}

// assumes double turns are possible
bool PuppetShape::turn(Shape& shape, Move m){
    const auto& rotation_idxs = roll[getRotation(m)];
    const auto& cycles = cycle_idxs[getFace(m)];
    auto& cubies = shape.cubies;
    auto next_cubies = cubies;

    uint8_t moving = 0;
    for(uint8_t j = 0; j < 4; ++j)
        moving |= 1u << cubies[cycles[INNER_COR_IDX][j]];

    for(uint8_t i =  BLOCKING_IDX; i < std::size(cycles); ++i){
        const auto& cycle = cycles[i];
        for(uint8_t j = 0; j < 4; ++j){
            uint8_t src = cubies[cycle[rotation_idxs[j]]];
            uint8_t dst = cubies[cycle[j]];
            if(moving & (1u << src)){
                if(moving & (1u << dst) || dst == 0){
                    next_cubies[cycle[j]] = src;
                }else{
                    return false;
                }
            }else if(moving & (1u << dst)){
                next_cubies[cycle[j]] = 0;
            }
        }
    }

    for(uint8_t i = 0; i <  BLOCKING_IDX; ++i){
        const auto& cycle = cycles[i];
        for(uint8_t j = 0; j < 4; ++j){
            next_cubies[cycles[i][j]] = cubies[cycle[rotation_idxs[j]]];
        }
    }

    cubies = next_cubies;
    return true;
}