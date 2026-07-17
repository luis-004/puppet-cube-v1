#include "Puppet.hpp"
Puppet::Puppet(){}

void Puppet::getSolved(State& state){
    state.corners = solved_corners;
    state.edges = solved_edges;
}

bool Puppet::testSolved(const State& state){
    return (state.corners == solved_corners) && (state.edges == solved_edges);
}

void Puppet::turn(State& state, uint8_t face, uint8_t dir){
    auto corner_idxs = permute_corners[face];
    auto edge_idxs = permute_edges[face];
    auto& corners = state.corners;
    auto& edges = state.edges;
    auto new_corners = corners;
    auto new_edges = edges;

    auto roll_dir = roll[dir];
    auto twist = twist_table;
    auto twist_info = twist_corners[face][dir];
    auto flips = edge_flips[dir];

    for(int i = 0; i < 4; i++){
        uint8_t &e = new_edges[edge_idxs[i]];
        e = edges[edge_idxs[roll_dir[i]]]; // permute edges
        e ^= (((flips >> i) & 1u) << FLIP_BIT); // flip edges
        uint8_t &c = new_corners[corner_idxs[i]];
        c = corners[corner_idxs[roll_dir[i]]]; // permute corners
        c = twist[twist_info[i]][c]; // twist corners
        //new_edges[edge_idxs[i]] = edges[edge_idxs[roll_dir[i]]] ^ (((flips >> i) & 1u) << FLIP_BIT);
        //new_corners[corner_idxs[i]] = twist[twist_info[i]][corners[corner_idxs[roll_dir[i]]]];
    }
    
    corners = new_corners;
    edges = new_edges;
}

uint32_t Puppet::getShapeHash(const State& state){
    uint8_t permutation = 0;
    uint8_t idxZero;
    uint16_t orientationHash = 0;
    for(int i = 0; i < NUM_CORNERS; i++){
        auto corner = state.corners[i];
        permutation |= (corner & 1u) << i;
        if(corner != 0){
            orientationHash = orientationHash * 3 + orientation[corner];
        }else{
            idxZero = i;
        }
    }
    return static_cast<uint32_t>(orientationHash) * 140u +
       static_cast<uint32_t>(from7choose3[idxZero][permutation]);
}

uint8_t Puppet::getCornerIdx(const State& state){
    std::array<uint8_t, 6> corners;
    uint8_t Oidx = 0;
    uint8_t Tidx = 3;
    for(int i = 0; i < NUM_CORNERS - 1; i++){
        auto corner = state.corners[i];
        if(corner & 1u){
            corners[Oidx++] = idx_in_type[corner];
        }else if(corner != 0){
            corners[Tidx++] = idx_in_type[corner];
        }
    }
    return corner_perms[corners[0]][corners[1]] * 6 + corner_perms[corners[3]][corners[4]];
}

std::pair<uint32_t, uint8_t> Puppet::getCornerIdxs(const State& state){
    uint8_t permutation = 0;
    uint8_t idxZero;
    uint16_t orientationHash = 0;
    std::array<uint8_t, 6> corners;
    uint8_t Oidx = 0;
    uint8_t Tidx = 3;
    for(int i = 0; i < NUM_CORNERS; i++){
        auto corner = state.corners[i];
        if(corner & 1u){
            permutation |= 1u << i;
            corners[Oidx++] = idx_in_type[corner];
            orientationHash = orientationHash * 3 + orientation[corner];
        }else if(corner != 0){
            corners[Tidx++] = idx_in_type[corner];
            orientationHash = orientationHash * 3 + orientation[corner];
        }else{
            idxZero = i;
        }
    }
    return {static_cast<uint32_t>(orientationHash) * 140u + static_cast<uint32_t>(from7choose3[idxZero][permutation]), 
        corner_perms[corners[0]][corners[1]] * 6 + corner_perms[corners[3]][corners[4]]};
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
    uint8_t flipIdx = 0;
    for(size_t i = 0; i < NUM_EDGES - 1; i++){
        flipIdx |= ((state.edges[i] >> FLIP_BIT) << i);
    }
    return flipIdx;
}

std::pair<uint32_t, uint8_t> Puppet::getEdgeIdxs(const State& state){
    uint32_t edgeIdx = 0;
    uint8_t flipIdx = 0;
    uint16_t visited = 0;
    for(size_t i = 0; i < NUM_EDGES - 1; i++){
        auto edge = state.edges[i];
        if(edge & (1u << FLIP_BIT)) flipIdx |= (1u << i); // store flip

        edgeIdx *= NUM_EDGES - i;
        edge &= ~(1u << FLIP_BIT); // unflip edge
        uint8_t idx = edge - __builtin_popcount(visited & ((1u << edge) - 1));
        visited |= (1u << edge); // mark as visited
        edgeIdx += idx;
    }
    return {edgeIdx, flipIdx};
}

void Puppet::rotateCorners(State& state){
    auto old = state.corners;
    for(size_t i = 0; i < NUM_CORNERS; i++){
        state.corners[rotation_corners[i]] = old[i];
    }
    for(size_t i = 0; i < NUM_CORNERS; i++){
        state.corners[i] = rotation_corners[state.corners[i]];
    }
    state.corners[0] = twist_table[1][state.corners[0]]; // corner at pos 0 changes orientation
}

void Puppet::mirrorCorners(State& state){
    auto old = state.corners;
    for(size_t i = 0; i < NUM_CORNERS; i++){
        uint8_t &c = state.corners[i];
        c = old[mirror_corners[i]];
        c = mirror_corners[c];
    }
}

void Puppet::rotateEdges(State& state){
    auto old = state.edges;
    for(size_t i = 0; i < NUM_EDGES; i++){
        state.edges[rotation_edges[i]] = old[i];
    }
    for(size_t i = 0; i < NUM_EDGES; i++){
        state.edges[i] = rotation_edges[state.edges[i]];
    }
}

void Puppet::mirrorEdges(State& state){
    auto old = state.edges;
    for(size_t i = 0; i < NUM_EDGES; i++){
        uint8_t &e = state.edges[i];
        e = old[mirror_edges[i]];
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
    std::array<uint8_t, 6> corners;
    uint8_t Oval = corner_idx / 6;
    uint8_t Tval = corner_idx % 6;
    bool found = 0;
    for(corners[0] = 0; corners[0] < 3; corners[0]++){
        for(corners[1] = 0; corners[1] < 3; corners[1]++){
            if(corner_perms[corners[0]][corners[1]] == Oval){found = 1; break;}
        }
        if(found)break;
    }
    corners[2] = ((corners[0] + corners[1]) * 2) % 3;
    found = 0;
    for(corners[3] = 0; corners[3] < 3; corners[3]++){
        for(corners[4] = 0; corners[4] < 3; corners[4]++){
            if(corner_perms[corners[3]][corners[4]] == Tval){found = 1; break;}
        }
        if(found)break;
    }
    corners[5] = ((corners[3] + corners[4]) * 2) % 3;
    found = 0;
    for(idxZero = 0; idxZero < 7; idxZero++){
        for(permutation = 0; permutation < 128; permutation++){
            if(from7choose3[idxZero][permutation] == from7choose3_val){found = 1; break;} 
        }
        if(found)break;
    }
    uint8_t Oidx = 0;
    uint8_t Tidx = 3;
    uint8_t ori_idx = 0;
    for(size_t i = 0; i < NUM_CORNERS; i++){
        if(i == idxZero){
            state.corners[i] = 0;
        }else{
            if((permutation >> i) & 1u){
                state.corners[i] |= 1;
                state.corners[i] += corners[Oidx++] << 1;
            }else{
                state.corners[i] += (corners[Tidx++] << 1) + 2;
            }
            uint8_t orientation = orientationHash / std::pow(3, NUM_CORNERS - ori_idx - 2);
            orientationHash %= static_cast<uint32_t>(std::pow(3, NUM_CORNERS - ori_idx - 2));
            ori_idx++;
            state.corners[i] += 6 * orientation;
        }
    }

    uint16_t visited = 0;
    uint8_t flipcount = 0;
    for(size_t i = 0; i < NUM_EDGES - 1; i++){
        size_t factorial = 1;
        for(size_t j = 1; j < NUM_EDGES - i; j ++){
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
        state.edges[i] |= (((flip_idx >> i) & 1u) << FLIP_BIT);
        if((flip_idx >> i) & 1u) flipcount++;
    }
    state.edges[NUM_EDGES - 1] |= (flipcount%2) << FLIP_BIT;
    for(size_t j = 0; j < NUM_EDGES; j++){
        if(!((visited >> j) & 1u)) state.edges[NUM_EDGES - 1] += j;
    }
    return state;
}

void Puppet::fixParity(State& state){
    bool parity = false;
    uint8_t save;
    State copy = state;
    for(size_t i = 0; i < NUM_CORNERS; i++){
        if(copy.corners[i] != 0){
           copy.corners[i] = (copy.corners[i] - 1) % 6 + 1;
        }
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