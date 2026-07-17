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

using namespace std;

int main(){
    PuppetShape ps;
    Puppet p;
    AlgMani am;
    Table table;
    //table.shape_table();
    string input;
    //table.table_full("shape_ep_full.heu", num_shapes*num_edge_permutations, &Table::SEP_index);
    std::cout << "loading tables" << std::endl;
    Solver solver("Shape_index", "Shape_info", "corner_flip_full.heu", "shape_ep_full.heu");
    std::cout << "loaded" << std::endl;
    auto scramble = solver.random_state(&Solver::astar);
    std::cout << "Ready" << std::endl;
    size_t count = 0;
    while(true){
        getline(std::cin, input);
        std::cout << ++count << " " << am.translate(am.translateYX2(am.reverse(scramble))) << scramble.size() << endl;
        if (input == "q") break;
        if (input == "c") system("cls");
        //system("cls");
        scramble = solver.random_state(&Solver::astar);
    }
    cin.get();
    return 0;
}