#include "Table.hpp"
Table::Table(){}

size_t Table::CEO_Rank(const Puppet::State& state){
    return rank_table[puppet.getShapeKey(state)] * NUM_CORNER_CYCLES * NUM_EDGE_ORIENTATIONS
        + puppet.getCornerRank(state) * NUM_EDGE_ORIENTATIONS
        + puppet.getEORank(state);
}

size_t Table::SEP_Rank(const Puppet::State& state){
    return rank_table[puppet.getShapeKey(state)] * NUM_EDGE_PERMUTATIONS + puppet.getEPRank(state);
}

size_t Table::E_Rank(const Puppet::State& state){
    return  puppet.getEPRank(state) * NUM_EDGE_ORIENTATIONS  + puppet.getEORank(state);
}

void Table::getShapeTables(){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    Puppet::State state;
    PuppetShape::Shape shape;
    struct node{
        Puppet::State state;
        PuppetShape::Shape shape;
        uint8_t last_face;
    };
    bool bucket = 0;
    std::array<std::vector<node>, 2> states;
    std::vector<uint16_t> shape_turns;
    std::vector<uint8_t> valid_shapes;
    std::vector<uint16_t> shape_ranks;
    shape_turns.resize(SHAPE_TABLE_SIZE);
    valid_shapes.resize(SHAPE_TABLE_SIZE);
    shape_ranks.assign(SHAPE_TABLE_SIZE, UINT16_MAX);
    uint8_t num_turns = 1;
    puppet.getSolved(state);
    puppet_shape.getCubeShape(shape);
    uint32_t key = puppet.getShapeKey(state);
    shape_turns[key] = 0b111111111;
    valid_shapes[key] = 0b1111;
    shape_ranks[key] = 0;
    uint16_t rank = 1;
    uint16_t states_visited = 1;
    uint8_t last_face = 255;
    states[bucket].push_back({state, shape, last_face});
    while(true){
        if(!states[bucket].empty()){
            state = states[bucket].back().state;
            shape = states[bucket].back().shape;
            last_face = states[bucket].back().last_face;
            states[bucket].pop_back();
        }else if(!states[!bucket].empty()){
            std::cout << "num turns:" << int(num_turns) << std::endl;
            std::cout << "num cases:" << int(rank) << std::endl;
            std::cout << "num visited:" << int(states_visited) << std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl << std::endl;
            num_turns++;
            bucket = !bucket;
            continue;
        }else{
            std::cout << "end" << std::endl;
            write_table("tables/shape_turns", shape_turns);
            write_table("tables/shape_ranks", shape_ranks);
            write_table("tables/valid_shapes", valid_shapes);
            std::cout << "saved" <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl;
            return;
        }
        key = puppet.getShapeKey(state);
        for(uint8_t face = 0; face < 3; ++face){
            bool half_turn_possible = false;
            for(uint8_t rotation = 0; rotation < 3; ++rotation){
                PuppetShape::Shape new_shape = shape;
                if((half_turn_possible || rotation != 2) && puppet_shape.turn(new_shape, getMove(face, rotation))){
                    half_turn_possible = true;
                    shape_turns[key] |= (1u << (face*3 + rotation)); // add valid turn
                    valid_shapes[key] |= (1u << 0);
                    if(rotation == 2){
                        valid_shapes[key] |= (1u << (face + 1));
                    }
                    if(face == last_face) continue;
                    states_visited++;
                    Puppet::State new_state = state;
                    puppet.turn(new_state, getMove(face, rotation));
                    uint32_t new_key = puppet.getShapeKey(new_state);
                    if(shape_ranks[new_key] > rank){
                        shape_ranks[new_key] = rank++;
                        states[!bucket].push_back({new_state, new_shape, face});
                    }
                }
            }
        }
    }
}

void Table::tableFull(const std::string& filename, const uint64_t num_entries, RankFunc rank_function){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    std::cout << "start" << std::endl;
    load_table<uint16_t> (shape_turns, "tables/shape_turns");
    load_table<uint16_t> (rank_table, "tables/shape_ranks");
    struct node{
        Puppet::State state;
        uint8_t last_face;
    };
    std::vector<uint8_t> table;
    table.resize(num_entries, 255);
    Puppet::State state;
    bool bucket = 0;
    size_t count = 1;
    size_t total_count = 1;
    size_t already_visited = 0;
    size_t sym_visited = 0;
    std::array<std::vector<node>, 2> states;
    uint8_t num_turns = 1;
    uint8_t last_face = 255;
    puppet.getSolved(state);
    table[(this->*rank_function)(state)] = 0;
    states[bucket].push_back({state, last_face});
    while(true){
        if(!states[bucket].empty()){
            state = states[bucket].back().state;
            last_face = states[bucket].back().last_face;
            states[bucket].pop_back();
        }else if(!states[!bucket].empty()){
            std::cout << "switching states: "<< states[!bucket].size() << std::endl;
            std::cout << "num turns:" << int(num_turns) <<  std::endl;
            std::cout << "cases:" << count <<  std::endl;
            std::cout << "total cases:" << total_count <<  std::endl;
            std::cout << "already visited:" << already_visited <<  std::endl;
            std::cout << "sym visited:" << sym_visited <<  std::endl;
            now = std::chrono::high_resolution_clock::now();
            duration = now - start_time;
            std::cout << "time: " << duration.count() <<  std::endl << std::endl;
            num_turns++;
            bucket = !bucket;
            continue;
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
        //uint16_t valid_turns = shape_turns[puppet.getShapeKey(state)];
        //uint8_t move_idx;
        //while((move_idx = __builtin_ctz(valid_turns)) < NUM_MOVES){ // get idx of least significant 1
        for(uint8_t move_idx = 0; move_idx < NUM_MOVES; move_idx++){
            //std::cout << "move" << count <<  std::endl;
            //valid_turns &= valid_turns - 1; // set least significant 1 to 0
            Move m = static_cast<Move>(move_idx);
            uint8_t face = getFace(m);
            if(last_face == face) continue;
            Puppet::State new_state = state;
            puppet.turn(new_state, m);
            uint8_t &current_entry = table[(this->*rank_function)(new_state)];
            if(current_entry > num_turns){
                current_entry = num_turns;
                count++;
                total_count++;
                states[!bucket].push_back({new_state, face});
                for(int j = 0; j < 5; j++){
                    if(j == 2){
                        puppet.mirror(new_state);
                    }else{
                        puppet.rotate(new_state);
                    }
                    //table[(this->*idxfunc)(next_state)] = num_turns;
                    uint8_t &cur = table[(this->*rank_function)(new_state)];
                    if(cur > num_turns){
                        cur = num_turns;
                        total_count++;
                    }else{
                        sym_visited++;
                    }
                }
            }else{already_visited++;}
        }
    }
}

void Table::table_full_turn(const uint64_t entries, RankFunc idxfunc){
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{0.0};
    auto now = start_time;
    std::cout << "start" << std::endl;
    load_table<uint16_t> (rank_table, "Shape_index_htm");
    std::vector<Move> alg = {};
    std::array<std::vector<std::vector<Move>>, 2> algs;
    std::vector<uint8_t> table;
    table.resize(entries, 255);
    Puppet::State state;
    PuppetShape::Shape shape;
    bool v = 0;
    uint32_t count = 1;
    //uint32_t total_count = 1;
    uint32_t already_visited = 0;
    //uint32_t sym_visited = 0;
    uint8_t last_face = 255;
    uint8_t num_turns = 1;
    bool s = true;
    puppet.getSolved(state);
    puppet_shape.getCubeShape(shape); 
    table[(this->*idxfunc)(state)] = 0;
    while(true){
        if(!algs[v].empty()){
            alg = algs[v].back();
            algs[v].pop_back();
            puppet.getSolved(state);
            puppet_shape.getCubeShape(shape); 
            for(auto m : alg){
                puppet_shape.turn(shape, m);
                puppet.turn(state, m);
            }
            last_face = getFace(alg.back());
        }else if(!algs[!v].empty()){
            std::cout << "switching states"<< algs[!v].size() << std::endl;
            std::cout << "num turns:" << int(num_turns) <<  std::endl;
            std::cout << "num cases:" << int(count) <<  std::endl;
            //std::cout << "num total cases:" << int(total_count) <<  std::endl;
            std::cout << "already visited:" << int(already_visited) <<  std::endl;
            //std::cout << "sym visited:" << int(sym_visited) <<  std::endl;
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
        for(uint8_t face = 0; face < 3; ++face){
            if(face == last_face){continue;}
            bool HT_possible = false;
            for(uint8_t t = 0; t < 3; t++){
                if((HT_possible || t != 2) && puppet_shape.turn(shape, getMove(face, t))){
                    Puppet::State next_state = state;
                    puppet.turn(next_state, getMove(face, t));
                    int idx = (this->*idxfunc)(next_state);
                    std::cout << idx << " " << int(puppet.getCornerRank(next_state)) << " "<< std::bitset<8>(puppet.getEORank(next_state)) << " ";
                    if(table[idx] > num_turns){
                        table[idx] = num_turns;
                        count++;
                        //total_count++;
                        alg.push_back(getMove(face, t));
                        std::cout << am.translate(alg) << std::endl;
                        algs[!v].push_back(alg);
                        alg.pop_back();
                        /*for(int j = 0; j < 5; j++){
                            if(j == 2){
                                puppet.mirror(next_state);
                            }else{
                                puppet.rotate(next_state);
                            }
                            idx = (this->*idxfunc)(next_state);
                            //std::cout << int(idx) << " ";
                            if(table[idx] > num_turns){
                                table[idx] = num_turns;
                                //std::cout << std::endl;
                                total_count++;
                            }else{
                                //std::cout << " sym visited: " << int(table[idx]) << std::endl;
                                sym_visited++;
                            }
                        }*/
                    }else{
                        alg.push_back(getMove(face, t));
                        std::cout << am.translate(alg) << " ";
                        alg.pop_back();
                        std::cout << "already visited: " << int(table[idx]) << std::endl;
                        already_visited++;
                    }
                    puppet_shape.turn(shape, getMove(face, getReverseRotation(t)));
                    HT_possible = true;
                }
            }
        }
    }
}
