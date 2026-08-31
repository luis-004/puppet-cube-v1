#include "Puppet.hpp"
Puppet::Puppet(){}

void Puppet::getSolved(State& state){
    for(int m = 0; m < 3; m++){
        for (int i = 0; i < NUM_CORNER_VALS; ++i) {
            assert(turn_corners[m+3][turn_corners[m][i]] == i);
            assert(turn_corners[m][turn_corners[m+3][i]] == i);

            assert(turn_corners[m][turn_corners[m][i]] == turn_corners[m+6][i]);
            assert(turn_corners[m+6][turn_corners[m+6][i]] == i);
        }
    }
    state.corners = solved_corners;
    state.edges = solved_edges;
}

bool Puppet::testSolved(const State& state){
    return (state.corners == solved_corners) && (state.edges == solved_edges);
}

void Puppet::turn(State& state, Move m){
    for(auto &corner : state.corners){
        corner = turn_corners[static_cast<uint8_t>(m)][corner];
    }
    for(auto &edge : state.edges){
        edge = turn_edges[static_cast<uint8_t>(m)][edge];
    }
}

/*uint32_t Puppet::getCornerHash(const State& state){
    uint32_t hash = 0;
    uint8_t visited = 0;
    for(uint8_t i = 1; i < NUM_CORNERS; i++){
        hash *= NUM_CORNER_VALS - i * 3;
        auto corner = state.corners[i];
        auto pos = position[corner];
        uint8_t idx = corner - 3 * __builtin_popcount(visited & ((1u << pos) - 1));
        visited |= (1u << pos); // mark as visited
        hash += idx;
    }
    return hash;
}*/

uint32_t Puppet::getShapeHash(const State& state){
    uint8_t permutation = 0;
    uint16_t orientationHash = 0;
    for(uint8_t i = 0; i < NUM_CORNERS; i++){
        auto corner = state.corners[i];
        auto pos = position[corner];
        permutation |= (i & 1u) << pos;
        orientationHash +=  orientation[corner] * pow3[pos];
    }
    return static_cast<uint32_t>(orientationHash) * 140u +
       static_cast<uint32_t>(from7choose3[position[state.corners[0]]][permutation]);
}

uint8_t Puppet::getCornerIdx(const State& state){
    uint8_t a = position[state.corners[1]];
    uint8_t b = position[state.corners[3]];
    uint8_t c = position[state.corners[5]];
    uint8_t d = position[state.corners[2]];
    uint8_t e = position[state.corners[4]];
    uint8_t f = position[state.corners[6]];
    return corner_perms[(a > b) | ((a > c) << 1) | ((b > c) << 2)] * 6 + corner_perms[(d > e) | ((d > f) << 1) | ((e > f) << 2)];
}

uint32_t Puppet::getEPIdx(const State& state){
    uint32_t edgeIdx = 0;
    uint16_t visited = 0;
    for(size_t i = 0; i < NUM_EDGES - 1; i++){
        edgeIdx *= NUM_EDGES - i;
        auto edge = state.edges[i];
        edge &= ~(1u << FLIP_BIT); // unflip edge
        uint8_t idx = edge - __builtin_popcount(visited & ((1u << edge) - 1));
        visited |= (1u << edge); // mark as visited
        edgeIdx += idx;
    }
    return edgeIdx;
}

uint8_t Puppet::getFlipIdx(const State& state){
    uint16_t flipIdx = 0;
    for(auto edge : state.edges){
        flipIdx |= ((edge >> FLIP_BIT) << (edge & ~(1u << FLIP_BIT)));
    }
    return static_cast<uint8_t>(flipIdx);
}

std::pair<uint32_t, uint8_t> Puppet::getEdgeIdxs(const State& state){
    uint32_t edgeIdx = 0;
    uint16_t flipIdx = 0;
    uint16_t visited = 0;
    for(size_t i = 0; i < NUM_EDGES; i++){
        auto edge = state.edges[i];
        flipIdx |= ((edge >> FLIP_BIT) << (edge & ~(1u << FLIP_BIT)));

        edgeIdx *= NUM_EDGES - i;
        edge &= ~(1u << FLIP_BIT); // unflip edge
        uint8_t idx = edge - __builtin_popcount(visited & ((1u << edge) - 1));
        visited |= (1u << edge); // mark as visited
        edgeIdx += idx;
    }
    return {edgeIdx, static_cast<uint8_t>(flipIdx)};
}

void Puppet::rotate(State& state){
    auto old_c = state.corners;
    state.corners[0] = (state.corners[0] + 14) % NUM_CORNER_VALS;
    for(size_t i = 1; i < NUM_CORNERS; i++){
        state.corners[rotation_corners[i]] = old_c[i];
    }
    for(auto &corner : state.corners){
        corner = rotation_corners[corner];
    }
    auto old_e = state.edges;
    for(size_t i = 0; i < NUM_EDGES; i++){
        state.edges[rotation_edges[i]] = old_e[i];
    }
    for(auto &edge : state.edges){
        edge = rotation_edges[edge];
    }
}

void Puppet::mirror(State& state){
    auto old_c = state.corners;
    for(size_t i = 0; i < NUM_CORNERS; i++){
        uint8_t &c = state.corners[i];
        c = old_c[mirror_corners[i]];
        c = mirror_corners[c];
    }
    auto old_e = state.edges;
    for(size_t i = 0; i < NUM_EDGES; i++){
        uint8_t &e = state.edges[i];
        e = old_e[mirror_edges[i]];
        e = mirror_edges[e];
    }
}

Puppet::State Puppet::stateFromIdxs(uint32_t shape_hash, uint8_t corner_idx, uint32_t edge_idx, uint8_t flip_idx){
    State state;
    for(int i = 0; i < NUM_CORNERS; i++){
        state.corners[i] = 0;
    }
    for(int i = 0; i < NUM_EDGES; i++){
        state.edges[i] = 0;
    }

    auto orientationHash = shape_hash / 140u;
    auto from7choose3_val = shape_hash % 140u;
    uint8_t idxZero;
    uint8_t permutation;
    std::array<uint8_t, 6> pos;
    uint8_t Oval = corner_idx / 6;
    uint8_t Tval = corner_idx % 6;
    switch(Oval){
        case 0:
            pos[0] = 0; pos[1] = 1; pos[2] = 2;
            break;
        case 1:
            pos[1] = 0; pos[0] = 1; pos[2] = 2;
            break;
        case 2:
            pos[2] = 0; pos[0] = 1; pos[1] = 2;
            break;
        case 3:
            pos[0] = 0; pos[2] = 1; pos[1] = 2;
            break;
        case 4:
            pos[1] = 0; pos[2] = 1; pos[0] = 2;
            break;
        case 5:
            pos[2] = 0; pos[1] = 1; pos[0] = 2;
    }
    switch(Tval){
        case 0:
            pos[3] = 0; pos[4] = 1; pos[5] = 2;
            break;
        case 1:
            pos[4] = 0; pos[3] = 1; pos[5] = 2;
            break;
        case 2:
            pos[5] = 0; pos[3] = 1; pos[4] = 2;
            break;
        case 3:
            pos[3] = 0; pos[5] = 1; pos[4] = 2;
            break;
        case 4:
            pos[4] = 0; pos[5] = 1; pos[3] = 2;
            break;
        case 5:
            pos[5] = 0; pos[4] = 1; pos[3] = 2;
    }
    bool found = 0;
    for(idxZero = 0; idxZero < 7; idxZero++){
        for(permutation = 0; permutation < 128; permutation++){
            if(from7choose3[idxZero][permutation] == from7choose3_val){found = 1; break;} 
        }
        if(found)break;
    }
    uint8_t Oidx = 0;
    uint8_t Tidx = 3;
    
    state.corners[0] = idxZero;
    for(size_t i = 0; i < NUM_CORNERS; i++){
        if((permutation >> i) & 1u){
            state.corners[1 + 2*pos[Oidx++]] = i;
        }else if(i != idxZero){
            state.corners[2 + 2*pos[Tidx++]] = i;
        }
    }
    uint16_t visited = 0;
    uint8_t oricount = 0;
    for(size_t i = NUM_CORNERS - 1; i > 0 ; i--){
        uint8_t idx;
        for(size_t j = 0; j < NUM_CORNERS ; j++){
            if(state.corners[j] == i){
                idx = j;
                visited |= (1u << j);
                break;
            }
        }
        uint8_t orientation = orientationHash / pow3[i];
        //std::cout << int(orientation) << " " << int(orientationHash) << " " << i << std::endl;
        orientationHash %= pow3[i];
        state.corners[idx] += 7 * orientation;
        oricount += orientation;
    }
    for(size_t j = 0; j < NUM_CORNERS; j++){
        if(!((visited >> j) & 1u)){
            state.corners[j] += (14 * (oricount % 3)) % 21; 
            break;
        }
    }

    visited = 0;
    for(size_t i = 0; i < NUM_EDGES - 1; i++){
        size_t factorial = 1;
        for(size_t j = 1; j < NUM_EDGES - i; j++){
            factorial *= j;
        }
        uint8_t idx = edge_idx / factorial;
        edge_idx %= factorial;
        uint8_t edge = idx;
        for(size_t j = 0; j < NUM_EDGES; j++){
            if((visited >> j) & 1u){
                edge++;
            }else if(idx == 0){
                break;
            }else{
                idx--;
            }
        }
        visited |= (1u << edge); // mark as visited
        state.edges[i] = edge;

    }
    for(size_t j = 0; j < NUM_EDGES; j++){
        if(!((visited >> j) & 1u)) state.edges[NUM_EDGES - 1] += j;
    }
    uint8_t flipcount = 0;
    for(size_t i = 0; i < NUM_EDGES; i++){
        if(state.edges[i] != 8){
            if((flip_idx >> state.edges[i]) & 1u) flipcount++;
            state.edges[i] |= (((flip_idx >> state.edges[i]) & 1u) << FLIP_BIT);
        }
    }
    for(size_t i = 0; i < NUM_EDGES; i++){
        if(state.edges[i] == 8){
            state.edges[i] |= (flipcount%2) << FLIP_BIT;
        }
    }
    return state;
}

void Puppet::fixParity(State& state){
    bool parity = false;
    uint8_t save;
    State copy = state;
    for(size_t i = 0; i < NUM_CORNERS; i++){
        copy.corners[i] = copy.corners[i] % 7;
    }
    for(size_t i = 0; i < NUM_EDGES; i++){
        copy.edges[i] &= ~(1u << FLIP_BIT);
    }
    while(copy.corners != solved_corners){
        for(size_t i = 0; i < NUM_CORNERS; ++i){
            if(copy.corners[i] != i){
                save = copy.corners[copy.corners[i]]; 
                copy.corners[copy.corners[i]] = copy.corners[i];
                copy.corners[i] = save;
                parity = !parity;
            }
        }
    }
    while(copy.edges != solved_edges){
        for(size_t i = 0; i < NUM_EDGES; ++i){
            if(copy.edges[i] != i){
                save = copy.edges[copy.edges[i]]; 
                copy.edges[copy.edges[i]] = copy.edges[i];
                copy.edges[i] = save;
                parity = !parity;
            }
        }
    }
    if(parity){
        save = state.edges[0]; 
        state.edges[0] = state.edges[1];
        state.edges[1] = save;
    }
}