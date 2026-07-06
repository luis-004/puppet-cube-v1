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

    for(int i = 0; i < 4; i++){
        new_edges[edge_idxs[i]] = edges[edge_idxs[roll_dir[i]]]; // permute edges

        uint8_t &c = new_corners[corner_idxs[i]];
        c = corners[corner_idxs[roll_dir[i]]]; // permute corners
        c = twist[twist_info[i]][c]; // twist corners
    }

    // flip edges
    new_edges[edge_flips[face][dir][0]] ^= (1u << FLIP_BIT); 
    new_edges[edge_flips[face][dir][1]] ^= (1u << FLIP_BIT);

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
        auto &corner = state.corners[i];
        if(corner & (1u << 0)){
            corners[Oidx++] = idx_in_type[corner];
        }else if(corner != 0){
            corners[Tidx++] = idx_in_type[corner];
        }
    }
    return corner_perms[corners[0]][corners[1]] * 6 + corner_perms[corners[3]][corners[4]];
}

uint32_t Puppet::getEPIdx(const State& state){
    uint32_t edgeIdx = 0;
    uint16_t visited = 0;
    for(size_t i = 0; i < NUM_EDGES - 1; i++){
        edgeIdx *= NUM_EDGES - 1 - i;
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
        if(state.edges[i] & (1u << FLIP_BIT)) flipIdx |= (1u << i);
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

        edgeIdx *= NUM_EDGES - 1 - i;
        edge &= ~(1u << FLIP_BIT); // unflip edge
        uint8_t idx = edge - __builtin_popcount(visited & ((1u << edge) - 1));
        visited |= (1u << edge); // mark as visited
        edgeIdx += idx;
    }
    return {edgeIdx, flipIdx};
}

void Puppet::rotateCorners(State& state){
    std::array<uint8_t, NUM_CORNERS> old;
    std::copy_n(state.corners.begin(), NUM_CORNERS, old.begin());
    for(size_t i = 0; i < NUM_CORNERS; i++){
        state.corners[rotation_corners[i]] = old[i];
    }
    for(size_t i = 0; i < NUM_CORNERS; i++){
        state.corners[i] = rotation_corners[state.corners[i]];
    }
    state.corners[0] = twist_table[1][state.corners[0]]; // corner at pos 0 changes orientation
}

void Puppet::mirrorCorners(State& state){
    std::array<uint8_t, NUM_CORNERS> old;
    std::copy_n(state.corners.begin(), NUM_CORNERS, old.begin());
    for(size_t i = 0; i < NUM_CORNERS; i++){
        uint8_t &c = state.corners[i];
        c = old[mirror_corners[i]];
        c = mirror_corners[c];
    }
}

void Puppet::rotateEdges(State& state){
    std::array<uint8_t, NUM_EDGES> old;
    std::copy_n(state.edges.begin(), NUM_EDGES, old.begin());
    for(size_t i = 0; i < NUM_EDGES; i++){
        state.edges[rotation_edges[i]] = old[i];
    }
    for(size_t i = 0; i < NUM_EDGES; i++){
        state.edges[i] = rotation_edges[state.edges[i]];
    }
}

void Puppet::mirrorEdges(State& state){
    std::array<uint8_t, NUM_EDGES> old;
    std::copy_n(state.edges.begin(), NUM_EDGES, old.begin());
    for(size_t i = 0; i < 3; i++){
        uint8_t &c = state.edges[i];
        c = old[mirror_edges[i]];
        c = mirror_edges[c];
        uint8_t unflipped = c;
        unflipped &= ~(1u << FLIP_BIT); // unflip edge
        if(unflipped >= 3){
            c ^= (1u << FLIP_BIT);
        }
    }
    for(size_t i = 3; i < NUM_EDGES; i++){
        uint8_t &c = state.edges[i];
        c = old[mirror_edges[i]];
        c = mirror_edges[c];
        uint8_t unflipped = c;
        unflipped &= ~(1u << FLIP_BIT); // unflip edge
        if(unflipped < 3){
            c ^= (1u << FLIP_BIT);
        }
    }
}

void Puppet::checkState(const State& state){
    std::array<bool, NUM_CORNERS> cornerst = {};
    std::array<bool, NUM_EDGES> edgest = {};
    for(size_t i = 0; i < NUM_CORNERS; i++){
        uint8_t c = state.corners[i];
        if(cornerst[ctest[c]]){
            std::cout << int(ctest[c]) <<  "invalid state corners: ";
            for(auto i : state.corners){
                std::cout << int(i) << " ";
            }
            for(auto i : state.edges){
                std::cout << int(i) << " ";
            }
            std::cout << std::endl;
            return;
        }else{
            cornerst[ctest[c]] = true;
        }
    }
    for(size_t i = 0; i < NUM_EDGES; i++){
        uint8_t c = state.edges[i];
        if(edgest[etest[c]]){
            std::cout << int(etest[c]) <<  "invalid state edges: ";
            for(auto i : state.corners){
                std::cout << int(i) << " ";
            }
            for(auto i : state.edges){
                std::cout << int(i) << " ";
            } 
            std::cout << std::endl;
        }else{
            edgest[etest[c]] = true;
        }
    }
}