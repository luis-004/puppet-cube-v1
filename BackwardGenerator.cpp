#include "BackwardGenerator.hpp"

BackwardGenerator::BackwardGenerator(const std::string& shape_info_file){
    table.load_table(table.shape_moves, shape_info_file);
}

std::vector<Move> BackwardGenerator::backward_moves(const uint16_t depth){
    Puppet::State state;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::array<uint8_t, 9> turns = {0,1,2,3,4,5,6,7,8};
    std::vector<uint8_t> valid_turns = {};
    std::vector<std::vector<uint8_t>> impossible_turns;
    std::vector<Move> scramble = {};
    bool impossible;
    uint8_t length = 0;
    puppet.getSolved(state);
    std::uniform_int_distribution<> pick(0, 8);
    Move m = static_cast<Move>(turns[pick(gen)]);
    uint8_t last_face = getFace(static_cast<Move>(m));
    puppet.turn(state, m);
    scramble.push_back(m);
    impossible_turns.push_back({});
    while(scramble.size() < depth){
        length = scramble.size() - 1;
        for (const auto& t : turns) {
            impossible = 0;
            for(const auto& i : impossible_turns.at(length)){
                if(t == i){
                    impossible = 1;
                    break;
                }
            }
            if (getFace(static_cast<Move>(t)) != last_face && !impossible) {
                valid_turns.push_back(t);
            }
        }
        if(valid_turns.size() == 0){
            m = scramble.back();
            scramble.pop_back();
            puppet.turn(state, getMove(getFace(m), getReverseRotation(getRotation(m))));
            last_face = getFace(scramble.back());
            impossible_turns.at(length).clear();
            impossible_turns.at(length-1).push_back(static_cast<uint8_t>(m));
            continue;   
        }
        std::uniform_int_distribution<> pick(0, valid_turns.size() - 1);
        m = static_cast<Move>(valid_turns[pick(gen)]);
        uint16_t info = table.shape_moves[puppet.getShapeKey(state)];
        if((info >> static_cast<uint8_t>(m)) & 1u){
            puppet.turn(state, m);
            last_face = getFace(m);
            scramble.push_back(m);
            impossible_turns.push_back({});
        }else{
            impossible_turns.at(length).push_back(static_cast<uint8_t>(m));
        }
        valid_turns.clear();
    }
    return scramble;
}

void BackwardGenerator::scramble_set_backward_moves(const std::string& filename, const size_t count, const uint16_t depth){
    if(depth > 50){std::cout << "maximum depth is 50"; return;}
    std::vector<std::array<Move, 50>> scrambles;
    for(size_t i = 0; i < count; i++){
        std::vector<Move> scramble = backward_moves(depth);
        std::array<Move, 50> scramble_array;
        scramble_array.fill(END);
        uint8_t idx = 0;
        for(auto m : scramble){
            scramble_array[idx++] = m;
        }
        scrambles.push_back(scramble_array);
    }
    table.write_table<std::array<Move, 50>>(filename, scrambles);
}

void BackwardGenerator::scramble_set_backward_moves_increasing(const std::string& filename, const uint16_t start_depth, const std::vector<size_t> count_per_depth){
    std::vector<std::array<Move, 50>> scrambles;
    for(size_t depth = start_depth; depth < start_depth + count_per_depth.size(); depth++){
        if(depth > 50){break;}
        for(size_t c = 0; c < count_per_depth[depth - start_depth]; c++){
            std::vector<Move> scramble = backward_moves(depth);
            std::array<Move, 50> scramble_array;
            scramble_array.fill(END);
            uint8_t idx = 0;
            for(auto m : scramble){
                scramble_array[idx++] = m;
            }
            scrambles.push_back(scramble_array);
        }
    }
    table.write_table<std::array<Move, 50>>(filename, scrambles);
}