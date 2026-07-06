#pragma once
#include <cstdint>
#include <array>

enum class Move: uint8_t {
    R=0,U,F,Rc,Uc,Fc,R2,U2,F2
};

/*static constexpr std::array<Move, 9> revDir = {
    Move::Rc, Move::Uc, Move::Fc,
    Move::R,  Move::U,  Move::F,
    Move::R2, Move::U2, Move::F2
};*/

static constexpr std::array<uint8_t, 3> revDir = {1, 0, 2};

inline uint8_t getFace(Move m){return static_cast<uint8_t>(m) % 3;}
inline uint8_t getDir(Move m){return static_cast<uint8_t>(m) / 3;}
inline uint8_t getRevDir(uint8_t dir){return revDir[dir];}
inline Move getMove(uint8_t face, uint8_t dir){return static_cast<Move>(face + dir * 3);}
inline uint8_t rotateFace(uint8_t face){return (face + 1) % 3;}