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
    //table.table_full("corner_flip_full.heu", num_shapes*num_corner_permutations*num_edge_orientations, &Table::CEO_index);
    //cout << "SHAPE EDGES:" << endl;
    //table.shape_table();
    //table.table_full("shape_ep_full.heu", num_shapes*num_edge_permutations, &Table::SEP_index);
    //table.table_full_turn(num_shapes*num_corner_permutations*num_edge_orientations, &Table::CEO_index);
    //std::cout << "loading tables" << std::endl;
    Solver solver("lookup/Shape_index", "lookup/Shape_info", "lookup/corner_flip_full.heu", "lookup/shape_ep_full.heu");
    //solver.gods_mirror(&Solver::astar);
    //std::cout << "loaded" << std::endl;
    //solver.scramble_set("500_sorted", 500, &Solver::astar, true);
    solver.solve_scramble_set("results/bmi_30_10", &Solver::iterative_deepening);
    /*auto scramble = solver.random_state(&Solver::iterative_deepening);
    std::cout << "Ready" << std::endl;
    size_t count = 0;
    while(true){
        getline(std::cin, input);
        std::cout << ++count << " " << am.translate(am.translateYX2(scramble)) << scramble.size() << endl;
        if (input == "q") break;
        if (input == "c") system("cls");
        //system("cls");
        scramble = solver.random_state(&Solver::iterative_deepening);
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