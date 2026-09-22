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
    while(true){
        if(found[queue].empty()){
            queue = (queue + 1) % 3; 
            //std::cout << "switched to next queue"<< std::endl;
            //now = std::chrono::high_resolution_clock::now();
            //duration = now - start_time;
            //std::cout << "time: " << duration.count() <<  std::endl;
            continue;
        }
        cur = found[queue].back();
        state = cur.state;
        found[queue].pop_back();
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
                    std::cout << "solution found. time: " << duration.count() << std::endl;
                    std::vector<Move> alg;
                    for(uint8_t i = 0; i < cur.len; i++){
                        alg.push_back(cur.alg[i]);
                    }
                    alg.push_back(m);
                    std::cout << am.translate(alg) << std::endl;
                    //std::cout << "found[0]: " << found[0].size() << std::endl;
                    //std::cout << "found[1]: " << found[1].size() << std::endl;
                    //std::cout << "found[2]: " << found[2].size() << std::endl;
                    return alg;
                }
                next.alg = cur.alg;
                next.alg[cur.len] = m;          
                next.len = cur.len + 1;
                next.value = newval;
                if(cur.value > newval + 1 || cur.value < newval - 1){
                    std::cout << "heuristic invalid: " << std::endl;
                    std::cout << int(getHeuVal(cur.state)) << std::endl;
                    for(size_t i = 0; i < 7; i++){
                        std::cout << int(cur.state.corners[i]) << " ";
                    }
                    std::cout << std::endl;
                    for(size_t i = 0; i < 9; i++){
                        std::cout << int(cur.state.edges[i]) << " ";
                    }
                    std::cout << std::endl;
                    std::cout << int(getHeuVal(next.state)) << std::endl;
                    for(size_t i = 0; i < 7; i++){
                        std::cout << int(next.state.corners[i]) << " ";
                    }
                    std::cout << std::endl;
                    for(size_t i = 0; i < 9; i++){
                        std::cout << int(next.state.edges[i]) << " ";
                    }
                    std::cout << std::endl;
                }
                found[(queue + 1 + newval - cur.value)%3].push_back(next);
            }
            info &= info - 1; // set least significant 1 to 0
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

void Solver::scramble_set(const std::string& filename, size_t count, solvefunc solve, bool sort){
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
        auto count = 0; 
        for(auto m : s){
            if(m == END) break;
            puppet.turn(state, m);
            count ++;
        }
        for(auto i: state.corners){
            std::cout << int(i) << " ";
        }
        for(auto i: state.edges){
            std::cout << int(i) << " ";
        }
        std::cout << "  " << int(count) << std::endl;
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