#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <utility>
#include <cmath>
#include <array>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <random>
#include <queue>
#include <unordered_map>
#include <cstring>

#include <bitset>
#include <filesystem>

#include "Puppet.hpp"
#include "Move.hpp"
#include "AlgMani.hpp"
#include "Table.hpp"

class BackwardGenerator{
    Puppet puppet;
    AlgMani am;
    Table table;
    public:
        BackwardGenerator(const std::string& shape_info_file);
        std::vector<Move> backward_moves(const uint16_t depth);
        void scramble_set_backward_moves(const std::string& filename, const size_t count, const uint16_t depht);
        void scramble_set_backward_moves_increasing(const std::string& filename, const uint16_t start_depth, const std::vector<size_t> count_per_depth);
};    
