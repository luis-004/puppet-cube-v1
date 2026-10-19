#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <utility>
//#include <cmath>
//#include <array>
#include <algorithm>
//#include <queue>
#include <cstdlib>
#include <bits/stdc++.h>
#include <random>
#include <chrono>

#include <bitset>
#include <filesystem>

#include "PuppetShape.hpp"
#include "AlgMani.hpp"
#include "Move.hpp"
#include "Puppet.hpp"
#include "Table.hpp"
#include "Solver.hpp"
#include "BackwardGenerator.hpp"

using namespace std;

int main(){
    string input;
    Puppet p;
    AlgMani am;
    Table table;

    //cout << "SHAPE TABLES:" << endl;

    //cout << "CORNERS FLIPS:" << endl;
    //table.tableFull("tables/CEO.heu", CEO_SIZE, &Table::CEO_Rank);
    //cout << "SHAPE EDGES:" << endl;
    //table.getShapeTables();
    //table.table_full("shape_ep_full.heu", num_shapes*num_edge_permutations, &Table::SEP_index);
    //table.tableFullurn(num_shapes*num_corner_permutations*num_edge_orientations, &Table::CEO_index);
    //std::cout << "loading tables" << std::endl;
    Solver solver("tables/shape_ranks", "tables/shape_turns", "tables/corner_flip_full.heu", "tables/shape_ep_full.heu");
    //solver.gods_mirror(&Solver::astar);
    //std::cout << "loaded" << std::endl;
    //solver.scramble_set_random_moves("500_random", 500, 1000, [&](const Puppet::State& s) { return solver.astar(s);}, true);
    //solver.scramble_set_decreasing_random_states("tables/decreasing_set", 25, {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10}, true);
    solver.solve_scramble_set("results/500_random", [&](const Puppet::State& s) { return solver.astar(s);});
    /*auto scramble = solver.random_state([&](const Puppet::State& s) { return solver.iterative_deepening(s, 25);});
    std::cout << "Ready" << std::endl;
    size_t count = 0;
    while(true){
        getline(std::cin, input);
        if(scramble.empty()){
            std::cout << "no sol found in xx moves" << std::endl;
        }else{std::cout << ++count << " " << am.translate(am.translateYX2(scramble)) << scramble.size() << endl;}
        
        if (input == "q") break;
        if (input == "c") system("cls");
        //system("cls");
        scramble = solver.random_state([&](const Puppet::State& s) { return solver.iterative_deepening(s, 25);});
    }
    cin.get();*/
    //BackwardGenerator bg("lookup/Shape_info");
    //bg.scramble_set_backward_moves_increasing("results/bmi_30_10", 1, {1, 4, 5, 5, 5, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10});
    /*auto scramble = bg.backward_moves(20);
    std::cout << "Ready" << std::endl;
    size_t count = 0;
    while(true){
        getline(std::cin, input);
        std::cout << ++count << " " << am.translate(am.translateYX2(scramble)) << scramble.size() << endl;
        if (input == "q") break;
        if (input == "c") system("cls");
        //system("cls");
        scramble = bg.backward_moves(20);
    }
    cin.get();*/
    return 0;
}