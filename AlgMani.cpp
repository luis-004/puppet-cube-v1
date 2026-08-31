#include "AlgMani.hpp"
AlgMani::AlgMani(){}

std::string AlgMani::translate(const std::vector<Move>& alg) {
    std::string translatedAlg;
    for (auto  move : alg) {
        translatedAlg += Moves[static_cast<size_t>(move)];
        translatedAlg += " ";
    }
    return translatedAlg;
}

std::string AlgMani::translateLeftie(const std::vector<Move>& alg) {
    std::string translatedAlg;
    for (auto move : alg) {
        translatedAlg += LeftieMoves[static_cast<size_t>(move)];
        translatedAlg += " ";
    }
    return translatedAlg;
}

std::vector<Move> AlgMani::translateBack(const std::string& alg) {
    std::vector<Move> translatedAlg;
    std::stringstream ss(alg);
    std::string move;
    while (ss >> move) {
        for (size_t i = 0; i < 9; ++i) {
            if (Moves[i] == move) {
                translatedAlg.push_back(static_cast<Move>(i));
                break;
            }
        }
    }
    return translatedAlg;
}

std::vector<Move>  AlgMani::translateYX2(std::vector<Move> alg){
    uint8_t pos = 7;
    std::array<uint8_t, 3> translateFace = {2,1,0};
    std::vector<Move> translatedAlg;
    for(auto turn : alg){
        uint8_t face = getFace(turn);
        uint8_t dir = getDir(turn);
        uint8_t newFace = translateFace[face];
        translatedAlg.push_back(getMove(newFace, dir));
        std::array<uint8_t, 4> movingCorners;
        if (face == 0) {
            movingCorners = {7, 3, 1, 5};
        } else if (face == 1) {
            movingCorners = {7, 6, 2, 3};
        } else {
            movingCorners = {7, 5, 4, 6};
        }
        constexpr uint8_t roll[3] = {1, 3, 2};
        auto corner = std::find(movingCorners.begin(), movingCorners.end(), pos);
        if(movingCorners.end() != corner){
            size_t index = std::distance(movingCorners.begin(), corner);
            pos = movingCorners[(index + roll[dir]) % 4];
            if(dir!=2){
                switch(face){
                    case 0:
                        translateFace = {translateFace[0],translateFace[2],translateFace[1]};
                        break;
                    case 1:
                        translateFace = {translateFace[2],translateFace[1],translateFace[0]};
                        break;
                    case 2:
                        translateFace = {translateFace[1],translateFace[0],translateFace[2]};
                        break;
                }
            }
        }
    }
    return translatedAlg;
}

std::vector<Move>  AlgMani::reverse(std::vector<Move> alg){
    std::vector<Move> reversedAlg;
    for(int8_t i=alg.size()-1; i >= 0; i--){
        reversedAlg.push_back(getMove(getFace(alg[i]), rev_dir[getDir(alg[i])]));
    }
    return reversedAlg;
}

std::pair<std::vector<Move>, std::array<uint8_t, 6>> AlgMani::optimizeRUF(std::vector<Move> alg, std::array<uint8_t, 6> turns){
    std::array<uint8_t, 3> swaps = {0,1,2};
    uint8_t max = 0;
    uint8_t max_index = 0;
    uint8_t min = turns[0];
    uint8_t min_index = 0;
    for(uint8_t i = 0; i < 3; ++i){
        if(turns[i]>max){
            max = turns[i];
            max_index = i;
        }else if(turns[i]<min){
            min = turns[i];
            min_index = i;
        }
    }
    if(max_index != min_index){
        swaps[0] = max_index;
        swaps[1] = 3 - max_index - min_index;
        swaps[2] = min_index;
    }
    uint8_t stay = 0;
    for(uint8_t i = 0; i<3; ++i){
        if(swaps[i] == i){
            stay += (i + 1);
        }
    }
    if(stay == 6 ){
        return std::make_pair(alg, turns);
    }else if(stay == 0){
        return std::make_pair(twist(alg, swaps[0] % 2),
        std::array<uint8_t, 6>{turns[swaps[0]],turns[swaps[1]],turns[swaps[2]],turns[swaps[0]+3],turns[swaps[1]+3],turns[swaps[2]+3]});
    }else{
        return std::make_pair(swap(alg, stay - 1),
        std::array<uint8_t, 6>{turns[swaps[0]],turns[swaps[1]],turns[swaps[2]],turns[swaps[0]+3],turns[swaps[1]+3],turns[swaps[2]+3]});
    }   
}

std::pair<std::vector<Move>, std::array<uint8_t, 6>> AlgMani::optimizeRF(std::vector<Move> alg, std::array<uint8_t, 6> turns){
    if(turns[0]<turns[2] || (turns[0]==turns[2] && (turns[3]<turns[5]))){
        return std::make_pair(swap(alg, 1),
        std::array<uint8_t, 6>{turns[2],turns[1],turns[0],turns[5],turns[4],turns[3]});
    }else{
        return std::make_pair(alg,turns);
    }   
}

std::array<uint8_t, 6> AlgMani::getRUF(std::vector<Move> alg){
    std::array<uint8_t, 6> turns = {0,0,0,0,0,0};
    for(auto turn : alg){
        turns[getFace(turn)]++;
        turns[getFace(turn)+3]++;
        if(getDir(turn) == 2){
            turns[getFace(turn)+3]++;
        }
    }
    return turns;
}

std::vector<Move> AlgMani::swap(std::vector<Move> alg, uint8_t stay){
    std::vector<Move> swap_alg;
    uint8_t new_face;
    for(auto turn : alg){
        if(getFace(turn) == stay){
           swap_alg.push_back(getMove(getFace(turn), rev_dir[getDir(turn)]));
        }else{
            new_face = 3 - stay - getFace(turn); 
            swap_alg.push_back(getMove(new_face, rev_dir[getDir(turn)]));
        }
    }
    return swap_alg;
}

std::vector<Move>  AlgMani::twist(std::vector<Move> alg, bool dir){
    std::vector<Move> twist_alg;
    uint8_t new_face;
    for(auto turn : alg){
        new_face = (getFace(turn) + 1 + dir) % 3;
        twist_alg.push_back(getMove(new_face, getDir(turn)));
    }
    return twist_alg;
}