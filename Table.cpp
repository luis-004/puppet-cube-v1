#include "Table.hpp"
Table::Table(){}

size_t Table::CEO_index(const Puppet::State& state){
    return shape_index[puppet.getShapeHash(state)] * num_corner_permutations * num_edge_orientations
        + puppet.getCornerIdx(state) * num_edge_orientations
        + puppet.getFlipIdx(state);
}

size_t Table::SEP_index(const Puppet::State& state){
    return shape_index[puppet.getShapeHash(state)] * num_edge_permutations + puppet.getEPIdx(state);
}

size_t Table::EP_index(const Puppet::State& state){
    return puppet.getEPIdx(state);
}

void Table::shape_table(){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    Puppet::State state;
    PuppetShape::Shape shape;
    bool v = 0;
    uint16_t index = 1;
    uint16_t distinct = 1;
    std::array<std::vector<std::vector<Move>>, 2> algs;
    std::vector<Move> alg = {};
    std::vector<uint8_t> info_table;
    std::vector<uint16_t> repr_index_table;
    std::vector<uint64_t> DCA_table;
    std::vector<uint16_t> index_table;
    info_table.resize(shape_table_size);
    DCA_table.resize(shape_table_size/64 + 1);
    index_table.assign(shape_table_size, num_shapes*2); // assigning values indicating invalid shape
    repr_index_table.resize(shape_table_size, 65535);
    uint8_t num_turns = 1;
    bool s = true;
    puppet.getSolved(state);
    ps.get_cube_shape(shape);
    uint32_t hash = puppet.getShapeHash(state);
    info_table[hash] = 0b1111111; // all moves possible; repr bit set
    repr_index_table[hash] = 0;
    DCA_table[hash >> 6] |= (uint64_t{1} << (hash & 63));
    index_table[hash] = 0;
    while(true){
        if(!algs[v].empty()){
            alg = algs[v].back();
            algs[v].pop_back();
            puppet.getSolved(state);
            ps.get_cube_shape(shape);
            for(auto move : alg){
                ps.turn(shape, getFace(move), getDir(move));
                puppet.turn(state, move);
            }
        }else if(!algs[!v].empty()){
            std::cout << "num turns:" << int(num_turns) <<  std::endl;
            std::cout << "num cases:" << int(index) <<  std::endl;
            std::cout << "num distinct cases:" << int(distinct) <<  std::endl;
            //std::cout << "already visited:" << int(already_visited) <<  std::endl;
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
            write_table("lookup/Shape_info", info_table);
            write_table("lookup/Shape_index", index_table);
            write_table("lookup/Shape_repr", repr_index_table);
            write_table("lookup/Shape_DCA", DCA_table);
            std::cout << "saved" <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl;
            return;
        }
        hash = puppet.getShapeHash(state);
        DCA_table[hash >> 6] |= (uint64_t{1} << (hash & 63));
        for(int i = 0; i < 6; ++i){
            Move m = static_cast<Move>(i);
            if(ps.turn(shape, getFace(m), getDir(m))){
                info_table[hash] |= (1u << i); // add valid turn
                puppet.turn(state, m);
                uint32_t new_hash = puppet.getShapeHash(state);
                if(index_table[new_hash] > index){
                    index_table[new_hash] = index++;
                    Puppet::State similar_state = state;
                    bool new_singular = true;
                    for(int i = 0; i < 5; i++){
                        i == 2 ? puppet.mirror(similar_state) : puppet.rotate(similar_state);
                        if(repr_index_table[puppet.getShapeHash(similar_state)] < distinct){
                            //std::cout << "sym visited: " << int(puppet.getShapeHash(similar_state)) << std::endl;
                            repr_index_table[new_hash] = repr_index_table[puppet.getShapeHash(similar_state)];
                            new_singular = false;
                            break;
                        }
                    }
                    if(new_singular){
                        repr_index_table[new_hash] = distinct++;
                        info_table[new_hash] |= (1u << repr_bit);
                    }
                    alg.push_back(m);
                    //std::cout << am.translate(alg) << " "<< int(new_hash) <<  " " << int(index - 1) << std::endl;
                    algs[!v].push_back(alg);
                    alg.pop_back();
                }/*else{
                    already_visited++;
                    alg.push_back(getMove(face, dir));
                    std::cout << am.translate(alg) << " "<< int(new_hash) << " " << " already visited" << std::endl;
                    alg.pop_back();
                }*/
                Move reverse = static_cast<Move>(getRevMove(m));
                ps.turn(shape, getFace(reverse), getDir(reverse));
                puppet.turn(state, reverse);
            }
        }
    }
}

void Table::table_full(const std::string& filename, const uint64_t entries, IndexFunc idxfunc){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    std::cout << "start" << std::endl;
    load_table<uint8_t> (shape_info, "lookup/Shape_info");
    load_table<uint16_t> (shape_index, "lookup/Shape_index");
    struct node{
        Puppet::State state;
        uint8_t allowed_turns;
    };
    std::vector<uint8_t> table;
    table.resize(entries, 255);
    Puppet::State state;
    bool v = 0;
    size_t count = 1;
    size_t total_count = 1;
    size_t already_visited = 0;
    size_t sym_visited = 0;
    uint8_t allowed = 0b00111111;
    std::array<std::vector<node>, 2> states;
    uint8_t num_turns = 1;
    bool s = true;
    puppet.getSolved(state);
    table[(this->*idxfunc)(state)] = 0;
    while(true){
        if(!states[v].empty()){
            state = states[v].back().state;
            allowed = states[v].back().allowed_turns;
            states[v].pop_back();
        }else if(!states[!v].empty()){
            std::cout << "switching states: "<< states[!v].size() << std::endl;
            std::cout << "num turns:" << int(num_turns) <<  std::endl;
            std::cout << "cases:" << count <<  std::endl;
            std::cout << "total cases:" << total_count <<  std::endl;
            std::cout << "already visited:" << already_visited <<  std::endl;
            std::cout << "sym visited:" << sym_visited <<  std::endl;
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
            write_large_table<uint8_t>(filename, table);
            std::cout << "saved" <<  std::endl;
            std::cout << "num cases:" << count <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl;
            return;
        }
        uint8_t valid_turns = shape_info[puppet.getShapeHash(state)];
        valid_turns &= allowed;
        uint8_t i;
        while((i = __builtin_ctz(valid_turns)) < repr_bit){ // get idx of least significant 1
            Move m = static_cast<Move>(i);
            Puppet::State next_state = state;
            puppet.turn(next_state, m);
            uint8_t &cur = table[(this->*idxfunc)(next_state)];
            if(cur > num_turns){
                cur = num_turns;
                count++;
                total_count++;
                node next;
                next.state = next_state;
                next.allowed_turns = 0b00111111 & ~(1u << (i ^ 1)); // cancel reverse turn
                if(!(allowed & (1u << (i ^ 1)))){ // if reverse turn was previously canceled, also cancel this turn
                   next.allowed_turns &= ~(1u << i);
                }
                states[!v].push_back(next);
                for(int j = 0; j < 5; j++){
                    if(j == 2){
                        puppet.mirror(next_state);
                    }else{
                        puppet.rotate(next_state);
                    }
                    //table[(this->*idxfunc)(next_state)] = num_turns;
                    uint8_t &cur = table[(this->*idxfunc)(next_state)];
                    if(cur > num_turns){
                        cur = num_turns;
                        total_count++;
                    }else{
                        sym_visited++;
                    }
                }
            }else{already_visited++;}
            valid_turns &= valid_turns - 1; // set least significant 1 to 0
        }
    }
}