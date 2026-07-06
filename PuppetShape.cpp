#include "PuppetShape.hpp"
PuppetShape::PuppetShape(){}

void PuppetShape::get_cube_shape(Shape& shape){
    shape.pieces = goal_pieces;
}

bool PuppetShape::test_cube_shape(const Shape& shape){
    return shape.pieces == goal_pieces;
}

// assumes double turns are possible
// R:0 U:1 F:2 R':3 U':4 F':5 R2:6 U2:7 F2:8
bool PuppetShape::turn(Shape& shape, uint8_t face, uint8_t dir){
    auto& idxs = rotate_idxs[face];
    auto& pieces = shape.pieces;
    auto cur = pieces;
    bool moving[7] = {};
    moving[pieces[idxs[inner_cor_idx][0]]] = true;
    moving[pieces[idxs[inner_cor_idx][1]]] = true;
    moving[pieces[idxs[inner_cor_idx][2]]] = true;
    moving[pieces[idxs[inner_cor_idx][3]]] = true;
    for(uint8_t i = blocking_idx; i < std::size(idxs); i++){
        for(uint8_t j = 0; j < 4; j++){
            auto const& row = idxs[i];
            uint8_t src = pieces[row[roll[dir][j]]];
            uint8_t dst = pieces[row[j]];
            if(moving[src]){
                if(moving[dst] || dst == 0){
                    cur[row[j]] = src;
                }else{
                    return false;
                }
            }else if(moving[dst]){
                cur[row[j]] = 0;
            }
        }
    }
    for(uint8_t i = 0; i < blocking_idx; i++){
        for(uint8_t j = 0; j < 4; j++){
            cur[idxs[i][j]] = pieces[idxs[i][roll[dir][j]]];
        }
    }
    pieces = cur;
    return true;
}