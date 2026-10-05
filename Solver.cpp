#include "Solver.hpp"

Solver::Solver(const std::string& shape_index_file, const std::string& shape_info_file, const std::string& CF_table_file, const std::string& SE_table_file){
    table.load_table(table.shape_index, shape_index_file);
    table.load_table(table.shape_info, shape_info_file);
    table.load_large_table(table.table_CF, CF_table_file);
    table.load_large_table(table.table_SE, SE_table_file);
}

uint8_t Solver::getHeuVal(const Puppet::State& state){
    auto EdgeIdxs = puppet.getEdgeIdxs(state);
    auto ShapeIdx = table.shape_index[puppet.getShapeHash(state)];
    uint8_t cfVal = table.table_CF[ShapeIdx * num_corner_permutations * num_edge_orientations + puppet.getCornerIdx(state) * num_edge_orientations + EdgeIdxs.second]; // puppet.getFlipIdx(state)
    uint8_t seVal = table.table_SE[ShapeIdx * num_edge_permutations + EdgeIdxs.first]; // puppet.getEPIdx(state)
    //uint8_t cfVal = table.table_CF[table.CEO_index(state)];
    //uint8_t seVal = table.table_SE[table.SEP_index(state)];
    return cfVal > seVal ? cfVal : seVal;
}

std::vector<Move> Solver::astar(const Puppet::State& start){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    struct node{
        Puppet::State state;
        std::array<Move, 50> alg;
        uint8_t len;
        uint8_t value;
    };
    node cur;
    node next;
    Puppet::State state = start;
    uint8_t last_face = 255;
    uint8_t queue = 0;
    uint8_t newval = 0;
    std::array<std::vector<node>, 3> found;
    found[0].push_back({state, {}, 0, getHeuVal(start)});
    size_t nodes_gen = 1;
    size_t max_nodes = 1;
    while(true){
        size_t size = found[0].size() + found[1].size() + found[2].size();
        if(size > max_nodes){
            max_nodes = size;
        }
        if(found[queue].empty()){
            queue = (queue + 1) % 3; 
            continue;
        }
        cur = found[queue].back();
        found[queue].pop_back();
        state = cur.state;
        if(cur.len > 0) last_face = getFace(cur.alg[cur.len-1]);
        uint16_t info = table.shape_info[puppet.getShapeHash(state)];
        uint8_t i;
        while((i = __builtin_ctz(info)) < repr_bit){ // get idx of least significant 1
            Move m = static_cast<Move>(i);
            uint8_t face = getFace(m);
            if(face != last_face){
                next.state = state;
                puppet.turn(next.state, m);
                newval = getHeuVal(next.state);
                if(newval == 0){
                    now = std::chrono::high_resolution_clock::now();
                    duration = now - start_time;
                    std::cout << "solution found. time: " << duration.count() <<" nodes generated: " << nodes_gen << " maximum nodes stored: "<< max_nodes << " nodes per second: " << double(duration.count() > 0 ? nodes_gen / double(duration.count()) : 0.0) << std::endl;
                    std::vector<Move> alg;
                    for(uint8_t i = 0; i < cur.len; i++){
                        alg.push_back(cur.alg[i]);
                    }
                    alg.push_back(m);
                    std::cout << am.translate(alg) << "  " << int(cur.len + 1) << std::endl;
                    return alg;
                }
                next.alg = cur.alg;
                next.alg[cur.len] = m;
                next.len = cur.len + 1;
                next.value = newval;
                found[(queue + 1 + newval - cur.value)%3].push_back(next);
                nodes_gen++;
            }
            info &= info - 1; // set least significant 1 to 0
        }
    }
}

namespace {
// Annahme: Puppet::State ist trivially copyable und hat keine Padding-Bytes.
// Falls nicht, ersetze beide Funktoren durch eine eigene Hash-/Vergleichsfunktion.
struct StateHash {
    size_t operator()(const Puppet::State& s) const {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&s);
        uint64_t h = 1469598103934665603ull; // FNV-1a
        for(size_t i = 0; i < sizeof(s); i++){
            h ^= p[i];
            h *= 1099511628211ull;
        }
        return static_cast<size_t>(h);
    }
};
struct StateEq {
    bool operator()(const Puppet::State& a, const Puppet::State& b) const {
        return std::memcmp(&a, &b, sizeof(a)) == 0;
    }
};
}

std::vector<Move> Solver::astar_weighted(const Puppet::State& start){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    double weight = 0.3;
    struct node{
        Puppet::State state;
        std::array<Move, 50> alg;
        uint8_t len;
        uint8_t value;
    };
    auto cmp = [weight](const node& a, const node& b){
        double fa = a.len * weight + a.value;
        double fb = b.len * weight + b.value;
        if(fa != fb) return fa > fb;
        return a.value > b.value;
    };
    std::priority_queue<node, std::vector<node>, decltype(cmp)> found(cmp);
    std::unordered_map<Puppet::State, uint8_t, StateHash, StateEq> seen;
    size_t dup_total = 0;      // State schon einmal generiert

    node cur;
    node next;
    Puppet::State state = start;
    uint8_t last_face = 255;
    uint8_t newval = 0;
    found.push({state, {}, 0, getHeuVal(start)});
    size_t nodes_gen = 1;
    size_t max_nodes = 1;
    while(!found.empty()){
        if(found.size() > max_nodes){
            max_nodes = found.size();
        }
        cur = found.top();
        found.pop();
        state = cur.state;
        last_face = cur.len > 0 ? getFace(cur.alg[cur.len-1]) : 255;
        if(cur.len >= 50) continue;
        uint16_t info = table.shape_info[puppet.getShapeHash(state)];
        uint8_t i;
        while((i = __builtin_ctz(info)) < repr_bit){ // get idx of least significant 1
            Move m = static_cast<Move>(i);
            uint8_t face = getFace(m);
            if(face != last_face){
                next.state = state;
                puppet.turn(next.state, m);
                newval = getHeuVal(next.state);
                if(newval == 0){
                    now = std::chrono::high_resolution_clock::now();
                    duration = now - start_time;
                    std::cout << "solution found. time: " << duration.count() <<" nodes generated: " << nodes_gen << " maximum nodes stored: "<< max_nodes 
                    << " nodes per second: " << double(duration.count() > 0 ? nodes_gen / double(duration.count()) : 0.0) << " states already seen: "<< dup_total << std::endl;
                    std::vector<Move> alg;
                    for(uint8_t i = 0; i < cur.len; i++){
                        alg.push_back(cur.alg[i]);
                    }
                    alg.push_back(m);
                    std::cout << am.translate(alg) << "  " << int(cur.len + 1) << std::endl;
                    return alg;
                }
                uint8_t newlen = cur.len + 1;
                auto it = seen.find(next.state);
                if(it != seen.end()){
                    dup_total++;
                    if(it->second <= newlen){
                        info &= info - 1;
                        //continue;
                    } else {
                        it->second = newlen;
                    }
                } else {
                    seen.emplace(next.state, newlen);
                }
                next.alg = cur.alg;
                next.alg[cur.len] = m;
                next.len = cur.len + 1;
                next.value = newval;
                found.push(next);
                nodes_gen++;
            }
            info &= info - 1; // set least significant 1 to 0
        }
    }
    return {}; 
}

std::vector<Move> Solver::astar_parent_nodes(const Puppet::State& start){
    struct Node {
        Puppet::State state;
        uint32_t parent;
        Move move;
        uint8_t value;
    };
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<Node> arena;
    arena.reserve(1 << 24);
    arena.push_back({start, UINT16_MAX, Move{}, getHeuVal(start)});

    std::array<std::vector<uint32_t>, 3> found;
    found[0].push_back(0);
    uint8_t queue = 0;

    while(true){
        if(found[queue].empty()){ queue = (queue + 1) % 3; continue; }
        uint32_t cur_idx = found[queue].back();
        found[queue].pop_back();
        const Node cur = arena[cur_idx];
        uint8_t last_face = (cur.parent == UINT16_MAX) ? 255 : getFace(cur.move);

        uint16_t info = table.shape_info[puppet.getShapeHash(cur.state)];
        uint8_t i;
        while((i = __builtin_ctz(info)) < repr_bit){
            Move m = static_cast<Move>(i);
            if(getFace(m) != last_face){
                Puppet::State ns = cur.state;
                puppet.turn(ns, m);
                uint8_t newval = getHeuVal(ns);
                if(newval == 0){
                    std::vector<Move> alg{m};
                    for(uint32_t k = cur_idx; arena[k].parent != UINT16_MAX; k = arena[k].parent)
                        alg.push_back(arena[k].move);
                    std::reverse(alg.begin(), alg.end());
                    std::chrono::duration<double> d = std::chrono::high_resolution_clock::now() - start_time;
                    std::cout << "solution found. time: " << d.count() << std::endl;
                    std::cout << am.translate(alg) << std::endl;
                    return alg;
                }
                arena.push_back({ns, cur_idx, m, newval});
                found[(queue + 1 + newval - cur.value) % 3].push_back(arena.size() - 1);
            }
            info &= info - 1;
        }
    }
}

std::vector<Move> Solver::random_state(solvefunc solve){
    std::random_device rd;
    std::mt19937 gen(rd());
    //for(int k = 0; k < 20; k++){
    std::uniform_int_distribution<> pickShape(0, num_shapes - 1);
    size_t shapeIdx = pickShape(gen);
    std::uniform_int_distribution<> pickCorners(0, num_corner_permutations - 1);
    size_t cornerIdx = pickCorners(gen);
    std::uniform_int_distribution<> pickEdges(0, num_edge_permutations - 1);
    size_t edgeIdx = pickEdges(gen);
    std::uniform_int_distribution<> pickFlips(0, num_edge_orientations - 1);
    size_t flipIdx = pickFlips(gen);
    size_t shapeHash;
    for(shapeHash = 0; shapeHash < shape_table_size; shapeHash++){
        if(table.shape_index[shapeHash] == shapeIdx) break;
    }
    //std::cout << shapeIdx <<" " << shapeHash <<" " << cornerIdx <<" "<< edgeIdx <<" "<< flipIdx << " " << std::bitset<8>(flipIdx)<< std::endl;
    Puppet::State start = puppet.stateFromIdxs(shapeHash, cornerIdx, edgeIdx, flipIdx);
    /*for(size_t i = 0; i < 7; i++){
        std::cout << int(start.corners[i]) << " ";
    }
    std::cout << std::endl;
    for(size_t i = 0; i < 9; i++){
        std::cout << int(start.edges[i]) << " ";
    }
    std::cout << std::endl;*/
    puppet.fixParity(start);
    return am.reverse((this->*solve)(start));
}

void Solver::scramble_set_random_states(const std::string& filename, size_t count, solvefunc solve, bool sort){
    std::vector<std::array<Move, 50>> scrambles;
    for(size_t i = 0; i < count; i++){
        std::vector<Move> scramble = random_state(solve);
        std::array<Move, 50> scramble_array;
        scramble_array.fill(END);
        uint8_t idx = 0;
        for(auto m : scramble){
            scramble_array[idx++] = m;
        }
        scrambles.push_back(scramble_array);
    }

    if (sort) {
        std::sort(scrambles.begin(), scrambles.end(),
            [](const auto& a, const auto& b) {
                auto length = [](const auto& scramble) {
                    return std::find(scramble.begin(), scramble.end(), END) - scramble.begin();
                };
                return length(a) < length(b);
            });
    }

    table.write_table<std::array<Move, 50>>(filename, scrambles);
}

std::vector<Move> Solver::backward_moves(uint16_t_t depth){
    PuppetShape::Shape shape;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::array<uint8_t, 9> turns = {0,1,2,3,4,5,6,7,8,9};
    std::vector<uint8_t> valid_turns = {};
    std::vector<std::vector<uint8_t>> impossible_turns;
    std::vector<Move> scramble = {};
    bool impossible;
    uint8_t length = 0;

}

void Solver::solve_scramble_set(const std::string& filename, solvefunc solve){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    std::vector<std::array<Move, 50>> scrambles;
    table.load_table<std::array<Move, 50>>(scrambles, filename);
    Puppet::State state;
    auto scount = 0; 
    for(auto s : scrambles){
        std::cout << int(scount++) <<"  ";
        puppet.getSolved(state);
        //auto count = 0; 
        for(auto m : s){
            if(m == END) break;
            puppet.turn(state, m);
            //count ++;
        }
        /*for(auto i: state.corners){
            std::cout << int(i) << " ";
        }
        for(auto i: state.edges){
            std::cout << int(i) << " ";
        }
        std::cout << "  " << int(count) << std::endl;*/
        (this->*solve)(state);
    }
    now = std::chrono::high_resolution_clock::now();
    duration = now - start_time;
    std::cout << "endtime: " << duration.count() << std::endl;
}

void Solver::god(solvefunc solve){
    uint8_t maxCF = 27;
    uint8_t maxSE = 30;

    size_t gods_number = 0;
    while(maxSE + maxCF > 0){
        std::cout << "new era: " << size_t(maxCF) << " " << size_t(maxSE) << std::endl;
        for(size_t i = 0; i < num_shapes * num_corner_permutations * num_edge_orientations; i++){
            if(table.table_CF[i] == maxCF){
                size_t shape = i / (num_corner_permutations * num_edge_orientations);
                for(size_t j = shape * num_edge_permutations; j <  (shape + 1) * num_edge_permutations; j++){
                    if(table.table_SE[j] == maxSE){
                        size_t shapeHash = 0;
                        for(shapeHash = 0; shapeHash < shape_table_size; shapeHash++){
                            if(table.shape_index[shapeHash] == shape) break;
                        }
                        Puppet::State start = puppet.stateFromIdxs(shapeHash, 
                        (i % (num_corner_permutations * num_edge_orientations)) / num_edge_orientations,
                        j % (num_edge_permutations), 
                        (i % (num_corner_permutations * num_edge_orientations)) % num_edge_orientations);
                        puppet.fixParity(start);
                        auto alg = (this->*solve)(start);
                        if(alg.size() > gods_number){
                            gods_number = alg.size();
                            std::cout << size_t(puppet.getShapeHash(start)) << " " 
                            << size_t(puppet.getCornerIdx(start)) << " " 
                            << size_t(puppet.getEPIdx(start)) << " " 
                            << size_t(puppet.getFlipIdx(start)) << std::endl;     
                            std::cout << am.translate(am.translateYX2(am.reverse(alg))) << " " << size_t(alg.size()) << std::endl;
                        }                      
                    }
                }
            }
        }
        maxSE > maxCF ? maxSE-- : maxCF--;
    }
}

void Solver::gods_mirror(solvefunc solve){
    Puppet::State state;
    puppet.getSolved(state);
    std::vector<uint32_t> sym_corners;
    for(size_t corner = 0; corner <  num_shapes * num_corner_permutations; corner++){
        Puppet::State start = puppet.stateFromIdxs(corner/num_corner_permutations, corner%num_corner_permutations,
        puppet.getEPIdx(state), puppet.getFlipIdx(state));
        Puppet::State sym = start;
        for(int j = 0; j < 5; j++){
            if(j == 2){
                puppet.mirror(sym);
            }else{
                puppet.rotate(sym);
            }
            if(sym == start){
                sym_corners.push_back(corner);
                break;
            }
        }
    }
    std::cout << "all symmetric corner cases found" << std::endl;

    puppet.getSolved(state);
    std::vector<uint32_t> sym_edges;
    for(size_t edge = 0; edge <  num_edge_permutations * num_edge_orientations; edge++){
        Puppet::State start = puppet.stateFromIdxs(puppet.getShapeHash(state), puppet.getCornerIdx(state),
        edge / num_edge_orientations, edge % num_edge_orientations);
        Puppet::State sym = start;
        for(int j = 0; j < 5; j++){
            if(j == 2){
                puppet.mirror(sym);
            }else{
                puppet.rotate(sym);
            }
            if(sym == start){
                sym_edges.push_back(edge);
                break;
            }
        }
    }
    std::cout << "all symmetric edge cases found" << std::endl;

    size_t gods_number = 0;
    for(size_t corners = 0; corners < sym_corners.size(); corners++){
        std::cout << "new corner era" << std::endl;
        for(size_t edges = 0; edges < sym_edges.size(); edges++){
            size_t shape = sym_corners[corners]/num_corner_permutations;
            if(table.table_CF[shape * num_corner_permutations * num_edge_orientations +  
            sym_corners[corners]%num_corner_permutations * num_edge_orientations + sym_edges[edges] % num_edge_orientations] +
            table.table_SE[shape * num_edge_permutations + sym_edges[edges] / num_edge_orientations] > 40){
                size_t shapeHash = 0;
                for(shapeHash = 0; shapeHash < shape_table_size; shapeHash++){
                    if(table.shape_index[shapeHash] == shape) break;
                }
                Puppet::State start = puppet.stateFromIdxs(shapeHash, sym_corners[corners]%num_corner_permutations,
                sym_edges[edges] / num_edge_orientations, sym_edges[edges] % num_edge_orientations);
                puppet.fixParity(start);
                auto alg = (this->*solve)(start);
                if(alg.size() > gods_number){
                    gods_number = alg.size();
                    std::cout << size_t(puppet.getShapeHash(start)) << " " 
                    << size_t(puppet.getCornerIdx(start)) << " " 
                    << size_t(puppet.getEPIdx(start)) << " " 
                    << size_t(puppet.getFlipIdx(start)) << std::endl;     
                    std::cout << am.translate(am.translateYX2(am.reverse(alg))) << " " << size_t(alg.size()) << std::endl;
                }       
            }          
        }
    }
}