#pragma once
#include <cstdint>
#include <array>

enum class Move: uint8_t {
    //R=0,U,F,Rc,Uc,Fc,R2,U2,F2
    R=0,Rc,U,Uc,F,Fc
};

constexpr Move END = static_cast<Move>(255);

inline uint8_t getFace(Move m){return static_cast<uint8_t>(m) >> 1;}
inline uint8_t getDir(Move m){return static_cast<uint8_t>(m) & 1;}
inline uint8_t getRevMove(Move m){return static_cast<uint8_t>(m) ^ 1;}
inline Move getMove(uint8_t face, bool dir){return static_cast<Move>((face << 1) + dir);}
inline uint8_t rotateFace(uint8_t face){return (face + 1) % 3;}