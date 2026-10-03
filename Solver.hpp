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

#include <bitset>
#include <filesystem>

#include "Puppet.hpp"
#include "Move.hpp"
#include "AlgMani.hpp"
#include "Table.hpp"

class Solver{
    Puppet puppet;
    AlgMani am;
    Table table;
    public:
        Solver(const std::string& shape_index_file, const std::string& shape_info_file, const std::string& CF_table_file, const std::string& SE_table_file);
        uint8_t getHeuVal(const Puppet::State& state);
        std::vector<Move> astar(const Puppet::State& start);
        std::vector<Move> astar_parent_nodes(const Puppet::State& start);

        using solvefunc = std::vector<Move> (Solver::*)(const Puppet::State&);

        std::vector<Move> random_state(solvefunc solve);
        void scramble_set(const std::string& filename, size_t count, solvefunc solve, bool sort);
        void solve_scramble_set(const std::string& filename, solvefunc solve);
        void god(solvefunc solve);
        void gods_mirror(solvefunc solve);
};
