#include "Table.hpp"
Table::Table(){}

void Table::shape_table(){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    Puppet::State state;
    PuppetShape::Shape shape;
    uint8_t last_face = 255;
    bool v = 0;
    uint16_t index = 1;
    uint16_t distinct = 1;
    std::array<std::vector<std::vector<Move>>, 2> algs;
    std::vector<Move> alg = {};
    std::vector<uint16_t> info_table;
    std::vector<uint16_t> repr_index_table;
    std::vector<uint8_t> DCA_table;
    std::vector<uint16_t> index_table;
    info_table.resize(shape_table_size);
    DCA_table.resize(shape_table_size);
    index_table.assign(shape_table_size, num_shapes);
    repr_index_table.resize(shape_table_size, num_distinct_shapes);
    uint8_t num_turns = 1;
    bool s = true;
    puppet.getSolved(state);
    ps.get_cube_shape(shape);
    const uint8_t repr_bit = 9; 
    info_table[puppet.getShapeHash(state)] = 0b1111111111; // all moves possible; repr bit set
    repr_index_table[puppet.getShapeHash(state)] = 0;
    DCA_table[puppet.getShapeHash(state)] = 0b1111;
    index_table[puppet.getShapeHash(state)] = 0;
    while(true){
        if(!algs[v].empty()){
            alg = algs[v].back();
            algs[v].pop_back();
            puppet.getSolved(state);
            ps.get_cube_shape(shape);
            for(auto move : alg){
                ps.turn(shape, getFace(move), getDir(move));
                puppet.turn(state, getFace(move), getDir(move));
            }
            last_face = getFace(alg.back());
        }else if(!algs[!v].empty()){
            std::cout << "num turns:" << int(num_turns) <<  std::endl;
            std::cout << "num cases:" << int(index) <<  std::endl;
            std::cout << "num distinct cases:" << int(distinct) <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl << std::endl;
            num_turns++;
            v = !v;
            continue;
        }else if(s){
            s = false;
        }else{
            std::cout << "end" << std::endl;
            write_table("Shape_info", info_table);
            write_table("Shape_index", index_table);
            write_table("Shape_repr", repr_index_table);
            write_table("Shape_DCA", DCA_table);
            std::cout << "saved" <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl;
            return;
        }
        uint32_t hash = puppet.getShapeHash(state);
        for(uint8_t face = 0; face < 3; ++face){
            bool HT_possible = false;
            for(uint8_t dir = 0; dir < 3; ++dir){
                if((HT_possible || dir != 2) && ps.turn(shape, face, dir)){
                    info_table[hash] |= (1u << (face + dir*3)); // add valid turn
                    DCA_table[hash] |= (1u << 0);
                    if(dir == 2){
                        DCA_table[hash] |= (1u << (face + 1));
                    }
                    if(face == last_face){
                        ps.turn(shape, face, getRevDir(dir));
                        HT_possible = true;
                        continue;
                    }
                    puppet.turn(state, face, dir);
                    uint32_t new_hash = puppet.getShapeHash(state);
                    if(index_table[new_hash] > index){
                        index_table[new_hash] = index++;
                        Puppet::State similar_state = state;
                        bool new_singular = true;
                        for(int i = 0; i < 5; i++){
                            i == 2 ? puppet.mirrorCorners(similar_state) : puppet.rotateCorners(similar_state);
                            if(repr_index_table[puppet.getShapeHash(similar_state)] < distinct){
                                repr_index_table[new_hash] = repr_index_table[puppet.getShapeHash(similar_state)];
                                new_singular = false;
                                break;
                            }
                        }
                        if(new_singular){
                            repr_index_table[new_hash] = distinct++;
                            info_table[new_hash] |= (1u << repr_bit);
                        }
                        alg.push_back(getMove(face, dir));
                        algs[!v].push_back(alg);
                        alg.pop_back();
                    }
                    ps.turn(shape, face, getRevDir(dir));
                    puppet.turn(state, face, getRevDir(dir));
                    HT_possible = true;
                }
            }
        }
    }
}

void Table::corner_flip_heuristic_full(){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    std::cout << "start" << std::endl;
    std::vector<uint16_t> shape_info;
    std::vector<uint16_t> shape_index;
    load_table<uint16_t> (shape_info, "Shape_info");
    load_table<uint16_t> (shape_index, "Shape_index");
    auto get_CPEO_index = [&](const Puppet::State& state) -> uint32_t {
        return shape_index[puppet.getShapeHash(state)] + puppet.getCornerIdx(state) * num_shapes
            + puppet.getFlipIdx(state) * num_shapes * num_corner_permutations;
    };
    struct node{
        Puppet::State state;
        uint8_t last_face;
    };
    node next;
    std::vector<uint8_t> table;
    table.resize(num_shapes*num_corner_permutations*num_edge_orientations, 255);
    Puppet::State state;
    bool v = 0;
    uint32_t count = 1;
    uint32_t already_visited = 0;
    uint8_t last_face = 255;
    std::array<std::vector<node>, 2> states;
    uint8_t num_turns = 1;
    bool s = true;
    puppet.getSolved(state);
    const uint8_t repr_bit = 9; 
    table[get_CPEO_index(state)] = 0;
    while(true){
        if(!states[v].empty()){
            state = states[v].back().state;
            last_face = states[v].back().last_face;
            states[v].pop_back();
        }else if(!states[!v].empty()){
            std::cout << "switching states"<< states[!v].size() << std::endl;
            std::cout << "num turns:" << int(num_turns) <<  std::endl;
            std::cout << "num cases:" << int(count) <<  std::endl;
            std::cout << "already visited:" << int(already_visited) <<  std::endl;
            write_table<uint8_t>("corner_flip_heu_full", table);
            std::cout << "saved" <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl << std::endl;
            num_turns++;
            v = !v;
            continue;
        }else if(s){
            s = false;
        }else{
            std::cout << "end" << std::endl;
            std::cout << "num cases:" << int(count) <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl;
            return;
        }
        uint16_t info = shape_info[puppet.getShapeHash(state)];
        uint8_t i;
        while((i = __builtin_ctz(info)) < repr_bit){ // get idx of least significant 1
            Move m = static_cast<Move>(i);
            uint8_t face = getFace(m);
            if(last_face != face){
                Puppet::State next_state = state;
                puppet.turn(next_state, face, getDir(m));
                uint8_t &cur = table[get_CPEO_index(next_state)];
                if(cur > num_turns){
                    cur = num_turns;
                    count++;
                    next.state = next_state;
                    next.last_face = face;
                    states[!v].push_back(next);
                    /*for(int i = 0; i < 5; i++){
                        if(i == 2){
                            puppet.mirrorCorners(next_state);
                            puppet.mirrorEdges(next_state);
                        }else{
                            puppet.rotateCorners(next_state);
                            puppet.rotateEdges(next_state);
                        }
                        table[get_CPEO_index(next_state)] = num_turns;
                    }*/
                    for(int i = 0; i < 3; i++){
                        puppet.rotateCorners(next_state);
                        puppet.rotateEdges(next_state);
                        table[get_CPEO_index(next_state)] = num_turns;
                    }
                }else{already_visited++;}
            }
            info &= info - 1; // set least significant 1 to 0
        }
    }
}

void Table::corner_flip_heuristic_turns(){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    std::array<std::vector<std::vector<Move>>, 2> algs;
    std::vector<Move> alg = {};
    std::vector<uint16_t> shape_info;
    std::vector<uint16_t> shape_index;
    std::vector<uint16_t> shape_singular;
    load_table<uint16_t> (shape_info, "Shape_info");
    load_table<uint16_t> (shape_index, "Shape_index");
    load_table<uint16_t> (shape_singular, "Shape_singular");
    auto get_CPEO_index = [&](const Puppet::State& state, const uint32_t hash) -> uint32_t {
        return  shape_index.at(hash) + puppet.getCornerIdx(state) * num_distinct_shapes
            + puppet.getFlipIdx(state) * num_distinct_shapes * num_corner_permutations;
    };
    auto get_total_CPEO_index = [&](const Puppet::State& state, const uint32_t hash) -> uint32_t {
        return shape_singular.at(hash) + puppet.getCornerIdx(state) * num_shapes
            + puppet.getFlipIdx(state) * num_shapes * num_corner_permutations;
    };
    uint8_t num_turns = 1;
    std::vector<uint8_t> table;
    std::vector<uint64_t> visited;
    table.assign(CF_size, 255);
    visited.resize(num_shapes*num_corner_permutations*num_edge_orientations/64);
    uint8_t last_face = 255;
    bool v = 0;
    uint32_t count = 1;
    uint32_t index = 1;
    uint32_t already_visited = 0;
    uint32_t no_repr_bit = 0;
    bool s = true;
    Puppet::State state;
    PuppetShape::Shape shape;
    puppet.getSolved(state);
    ps.get_cube_shape(shape);
    const uint8_t repr_bit = 9; 
    table[get_CPEO_index(state, puppet.getShapeHash(state))] = 0;
    while(true){
        if(!algs[v].empty()){
            alg = algs[v].back();
            algs[v].pop_back();
            puppet.getSolved(state);
            ps.get_cube_shape(shape);
            for(auto move : alg){
                ps.turn(shape, getFace(move), getDir(move));
                puppet.turn(state, getFace(move), getDir(move));
            }
            last_face = getFace(alg.back());
        }else if(!algs[!v].empty()){
            std::cout << "switching states: "<< algs[!v].size() << std::endl;
            std::cout << "num turns:" << int(num_turns) <<  std::endl;
            std::cout << "num cases:" << int(index) <<  std::endl;
            std::cout << "num total:" << int(count) <<  std::endl;
            std::cout << "already visited:" << int(already_visited) <<  std::endl;
            std::cout << "no repr bit:" << int(no_repr_bit) <<  std::endl;
            write_table<uint8_t>("corner_flip_heu", table);
            std::cout << "saved" <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl << std::endl;
            num_turns++;
            v = !v;
            continue;
        }else if(s){
            s = false;
        }else{
            std::cout << "end" << std::endl;
            std::cout << "num cases:" << int(index) <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl;
            return;
        }
        for(uint8_t face = 0; face < 3; ++face){
            if(face == last_face) continue;
            bool HT_possible = false;
            for(uint8_t dir = 0; dir < 3; ++dir){
                if((HT_possible || dir != 2) && ps.turn(shape, face, dir)){
                    puppet.turn(state, face, dir);
                    uint32_t hash = puppet.getShapeHash(state);
                    auto idx = get_total_CPEO_index(state, hash);
                    //std::cout << "found: " << "hash: " << int(hash) << " " << int(idx) << std::endl; /////
                    /*alg.push_back(getMove(face, dir));
                    std::cout <<  "found: " <<  am.translate(alg) << 
                    " hash: " << int(hash) << " " << int(idx) << std::endl; /////
                    alg.pop_back();*/
                    auto block = idx/64;
                    auto offset = idx%64;
                    if(!(visited.at(block) & (1ULL << (offset)))){
                        count++;
                        visited.at(block) |= (1ULL << (offset));
                        alg.push_back(getMove(face, dir));
                        algs[!v].push_back(alg);
                        alg.pop_back();
                        if(shape_info.at(hash) & (1u << repr_bit)){
                            table.at(get_CPEO_index(state, hash)) = num_turns;
                            index++;
                        }else{no_repr_bit++;}
                    }else{
                        //std::cout << int(idx) << ": already visited!" << std::endl; ////
                        already_visited++;
                    }
                    ps.turn(shape, face, getRevDir(dir));
                    puppet.turn(state, face, getRevDir(dir));
                    HT_possible = true;
                }
            }
        }
    }
}

void Table::corner_flip_heuristic_broke(){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    std::cout << "start" << std::endl;
    std::vector<uint16_t> shape_info;
    std::vector<uint16_t> shape_index;
    std::vector<uint16_t> shape_singular;
    load_table<uint16_t> (shape_info, "Shape_info");
    load_table<uint16_t> (shape_index, "Shape_index");
    load_table<uint16_t> (shape_singular, "Shape_singular");
    auto get_CPEO_index = [&](const Puppet::State& state, const uint32_t hash) -> uint32_t {
        return  shape_index[hash] + puppet.getCornerIdx(state) * num_distinct_shapes
            + puppet.getFlipIdx(state) * num_distinct_shapes * num_corner_permutations;
    };
    auto get_total_CPEO_index = [&](const Puppet::State& state, const uint32_t hash) -> uint32_t {
        return shape_singular[hash] + puppet.getCornerIdx(state) * num_shapes
            + puppet.getFlipIdx(state) * num_shapes * num_corner_permutations;
    };
    struct node{
        Puppet::State state;
        uint8_t last_face;
    };
    node next;
    std::vector<uint8_t> table;
    std::vector<uint64_t> visited;
    table.assign(CF_size, 255);
    visited.resize(num_shapes*num_corner_permutations*num_edge_orientations/64);
    Puppet::State state;
    bool v = 0;
    uint32_t count = 1;
    uint32_t index = 1;
    uint32_t already_visited = 0;
    uint32_t no_repr_bit = 0;
    uint8_t last_face = 255;
    std::array<std::vector<node>, 2> states;
    uint8_t num_turns = 1;
    bool s = true;
    puppet.getSolved(state);
    const uint8_t repr_bit = 9; 
    table[get_CPEO_index(state, puppet.getShapeHash(state))] = 0;
    while(true){
        if(!states[v].empty()){
            state = states[v].back().state;
            last_face = states[v].back().last_face;
            states[v].pop_back();
        }else if(!states[!v].empty()){
            std::cout << "switching states"<< states[!v].size() << std::endl;
            std::cout << "num turns:" << int(num_turns) <<  std::endl;
            std::cout << "num cases:" << int(index) <<  std::endl;
            std::cout << "num total:" << int(count) <<  std::endl;
            std::cout << "already visited:" << int(already_visited) <<  std::endl;
            std::cout << "no repr bit:" << int(no_repr_bit) <<  std::endl;
            write_table<uint8_t>("corner_flip_heu", table);
            std::cout << "saved" <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl << std::endl;
            num_turns++;
            v = !v;
            continue;
        }else if(s){
            s = false;
        }else{
            std::cout << "end" << std::endl;
            std::cout << "num cases:" << int(count) <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl;
            return;
        }
        uint32_t hash = puppet.getShapeHash(state);
        uint16_t info = shape_info[hash];
        uint8_t i;
        while((i = __builtin_ctz(info)) < repr_bit){ // get idx of least significant 1
            Move m = static_cast<Move>(i);
            uint8_t face = getFace(m);
            if(last_face != face){
                Puppet::State next_state = state;
                puppet.turn(next_state, face, getDir(m));
                hash = puppet.getShapeHash(next_state);
                auto idx = get_total_CPEO_index(next_state, hash);
                auto block = idx/64;
                auto offset = idx%64;
                if(!(visited[block] & (1ULL << (offset)))){
                    count++;
                    visited[block] |= (1ULL << (offset));
                    next.state = next_state;
                    next.last_face = face;
                    states[!v].push_back(next);
                    if(shape_info[hash] & (1u << repr_bit)){
                        table[get_CPEO_index(state, hash)] = num_turns;
                        index++;
                    }else{no_repr_bit++;}
                }else{already_visited++;}
            }
            info &= info - 1; // set least significant 1 to 0
        }
    }
}

void Table::corner_flip_heuristic_sym(){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    std::cout << "start" << std::endl;
    std::vector<uint16_t> shape_info;
    std::vector<uint16_t> shape_index;
    std::vector<uint16_t> shape_singular;
    load_table<uint16_t> (shape_info, "Shape_info");
    load_table<uint16_t> (shape_index, "Shape_index");
    load_table<uint16_t> (shape_singular, "Shape_singular");
    auto get_CPEO_index = [&](const Puppet::State& state, const uint32_t hash) -> uint32_t {
        return  shape_index[hash] + puppet.getCornerIdx(state) * num_distinct_shapes
            + puppet.getFlipIdx(state) * num_distinct_shapes * num_corner_permutations;
    };
    auto get_total_CPEO_index = [&](const Puppet::State& state, const uint32_t hash) -> uint32_t {
        return shape_singular[hash] + puppet.getCornerIdx(state) * num_shapes
            + puppet.getFlipIdx(state) * num_shapes * num_corner_permutations;
    };
    struct node{
        Puppet::State state;
        uint8_t last_face;
    };
    node next;
    std::vector<uint8_t> table;
    std::vector<uint64_t> visited;
    table.assign(CF_size, 255);
    visited.resize(num_shapes*36*256/64);
    Puppet::State state;
    bool v = 0;
    uint32_t count = 1;
    uint32_t index = 1;
    uint32_t already_visited = 0;
    uint32_t no_repr_bit = 0;
    uint8_t last_face = 255;
    std::array<std::vector<node>, 2> states;
    uint8_t num_turns = 1;
    bool s = true;
    puppet.getSolved(state);
    const uint8_t repr_bit = 9; 
    table[get_CPEO_index(state, puppet.getShapeHash(state))] = 0;
    while(true){
        if(!states[v].empty()){
            state = states[v].back().state;
            last_face = states[v].back().last_face;
            states[v].pop_back();
        }else if(!states[!v].empty()){
            std::cout << "switching states"<< states[!v].size() << std::endl;
            std::cout << "num turns:" << int(num_turns) <<  std::endl;
            std::cout << "num cases:" << int(index) <<  std::endl;
            std::cout << "num visited:" << int(count) <<  std::endl;
            std::cout << "already visited:" << int(already_visited) <<  std::endl;
            std::cout << "no repr bit:" << int(no_repr_bit) <<  std::endl;
            write_table<uint8_t>("corner_flip_heu", table);
            std::cout << "saved" <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl << std::endl;
            num_turns++;
            v = !v;
            continue;
        }else if(s){
            s = false;
        }else{
            std::cout << "end" << std::endl;
            std::cout << "num cases:" << int(count) <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl;
            return;
        }
        uint32_t hash = puppet.getShapeHash(state);
        uint16_t info = shape_info[hash];
        uint8_t i;
        while((i = __builtin_ctz(info)) < repr_bit){ // get idx of least significant 1
            Move m = static_cast<Move>(i);
            uint8_t face = getFace(m);
            if(last_face != face){
                Puppet::State next_state = state;
                puppet.turn(next_state, face, getDir(m));
                hash = puppet.getShapeHash(next_state);
                auto idx = get_total_CPEO_index(next_state, hash);
                auto block = idx/64;
                auto offset = idx%64;
                if(!(visited[block] & (1ULL << (offset)))){
                    count++;
                    visited[block] |= (1ULL << (offset));
                    next.state = next_state;
                    next.last_face = face;
                    states[!v].push_back(next);
                    if(shape_info[hash] & (1u << repr_bit)){
                        table.at(get_CPEO_index(state, hash)) = num_turns;
                        index++;
                    }else{no_repr_bit++;}
                    for(int i = 0; i < 5; i++){
                        if(i == 2){
                            puppet.mirrorCorners(next_state);
                            puppet.mirrorEdges(next_state);
                        }else{
                            puppet.rotateCorners(next_state);
                            puppet.rotateEdges(next_state);
                        }
                        hash = puppet.getShapeHash(next_state);
                        idx = get_total_CPEO_index(next_state, hash);
                        block = idx/64;
                        offset = idx%64;
                        visited[block] |= (1ULL << (offset));
                        if(shape_info[hash] & (1u << repr_bit)){
                            table.at(get_CPEO_index(state, hash)) = num_turns;
                            index++;
                        }else{no_repr_bit++;}
                    }
                }else{already_visited++;}
            }
            info &= info - 1; // set least significant 1 to 0
        }
    }
}