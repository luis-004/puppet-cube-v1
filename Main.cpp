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
    //string input;
    Puppet p;
    AlgMani am;
    Table table;

    // -- methods to generate the shape tables and PDBs --
    //table.getShapeTables();
    //table.tableFull("PDBs/CEO.heu", CEO_SIZE, &Table::CEO_Rank);
    //table.tableFull("PDBs/SEP.heu", SEP_SIZE, &Table::SEP_Rank);

    // -- calling constructor of the PDB solver (required for the following methods) --
    Solver solver("tables/shape_ranks", "tables/shape_moves", "PDBs/CEO.heu", "PDBs/SEP.heu");

    // -- generating the scramblesets used in the work --
    //solver.scramble_set_random_states("scramblesets/scrambles_500_sorted_2", 500, [&](const Puppet::State& s) { return solver.astar(s);}, true);
    //solver.scramble_set_decreasing_random_states("scramblesets/scrambles_20perdistance_2", 35, {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20}, true);

    // -- solving scramblesets with different solvers --
    //solver.solve_scramble_set("scramblesets/scrambles_500_sorted", [&](const Puppet::State& s) { return solver.astar(s);});
    //solver.solve_scramble_set("scramblesets/scrambles_500_sorted", [&](const Puppet::State& s) { return solver.iterative_deepening(s, 50);});
    solver.solve_scramble_set("scramblesets/scrambles_20perdistance", [&](const Puppet::State& s) { return solver.bwas(s, 10000, 0.0);});
    //solver.solve_scramble_set("scramblesets/scrambles_20perdistance", [&](const Puppet::State& s) { return solver.beam_search(s, 100);});

    // -- other stuff irrelevant for the work --

    //solver.scramble_set_random_moves("500_random", 500, 1000, [&](const Puppet::State& s) { return solver.astar(s);}, true);
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