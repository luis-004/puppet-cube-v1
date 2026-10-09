#pragma once
#include <cstdint>
#include <array>

enum class Move: uint8_t {
    R=0,Rc,R2,U,Uc,U2,F,Fc,F2
};

constexpr Move END = static_cast<Move>(255);

static constexpr std::array<uint8_t, 3> reverseRotation = {1, 0, 2};

inline uint8_t getFace(Move m){return static_cast<uint8_t>(m) / 3;}
inline uint8_t getRotation(Move m){return static_cast<uint8_t>(m) % 3;}
inline uint8_t getReverseRotation(uint8_t rotation){return reverseRotation[rotation];}
inline Move getMove(uint8_t face, uint8_t rotation){return static_cast<Move>(face * 3 + rotation);}
inline uint8_t rotateFace(uint8_t face){return (face + 1) % 3;}