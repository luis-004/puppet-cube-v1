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

using namespace std;

int main(){
    PuppetShape ps;
    Puppet p;
    //PuppetShape::Shape shape;
    //Puppet::State state;
    AlgMani am;
    Table table;
    table.corner_flip_heuristic();
    //table.shape_table();
    /*std::vector<uint16_t> info_table;
    std::vector<uint16_t> index_table;
    std::vector<uint8_t> tableDCA;
    std::vector<uint16_t> total_index_table;
    table.load_table<uint16_t>(info_table, "Shape_info");
    table.load_table<uint16_t>(index_table, "Shape_index");
    table.load_table<uint8_t>(tableDCA, "ShapeDCA");
    table.load_table<uint16_t>(total_index_table, "Shape_singular");
    for(size_t i = 0; i < info_table.size(); i++){
        if(info_table[i] != 0){

            std::cout << "index:" <<  i << " info: " << std::bitset<16>(info_table[i]) << " singular: " << int(index_table[i]) << " dca: " << std::bitset<8>(tableDCA[i]) << " total: " << int(total_index_table[i]) << std::endl;
        }
    }*/

    /*//auto alg = am.translateYX2(am.translateBack("U' R U' R' U2 R U2 R' F' U R' F R U2 R' F2 R F"));
    //auto alg = am.translateYX2(am.translateBack("R U' R' U' R' U' R U R U"));
    //auto alg = am.translateYX2(am.translateBack("U U2 U"));
    auto alg = am.translateYX2(am.translateBack("U R U R2 F2 R F R' F2 R U2"));
    //auto alg = am.translateBack("U R");
    cout << am.translate(alg) << endl;
    ps.get_cube_shape(shape);
    p.getSolved(state);
    cout << endl << int(p.getShapeHash(state)) << " " << int(p.getCornerIdx(state)) << endl;
    cout << int(p.getEdgeIdxs(state).first) << " " << int(p.getEdgeIdxs(state).second) << endl;
    for(auto move : alg){
        cout << am.translate({move}) << endl;
        if(ps.turn(shape, getFace(move), getDir(move))){
            p.turn(state, getFace(move), getDir(move));
        }
        for(auto i : state.corners){
            cout << int(i) << " ";
        }
        for(auto i : state.edges){
            cout << int(i) << " ";
        }
        cout << endl << int(p.getShapeHash(state)) << " " << int(p.getCornerIdx(state)) << endl;
        cout << int(p.getEdgeIdxs(state).first) << " " << int(p.getEdgeIdxs(state).second) << endl;
    }
    p.rotateCorners(state);
    p.rotateEdges(state);
    p.rotateCorners(state);
    p.rotateEdges(state);
    p.mirrorCorners(state);
    p.mirrorEdges(state);
    p.rotateCorners(state);
    p.rotateEdges(state);
    p.rotateCorners(state);
    p.rotateEdges(state);
    p.mirrorCorners(state);
    p.mirrorEdges(state);
    cout  << endl << int(p.getShapeHash(state)) << " " << int(p.getCornerIdx(state)) << endl;
    for(auto move : am.reverse(alg)){
        cout << am.translate({move}) << endl;
        if(ps.turn(shape, getFace(move), getDir(move))){
            p.turn(state, getFace(move), getDir(move));
        }
        for(auto i : state.corners){
            cout << int(i) << " ";
        }
        for(auto i : state.edges){
            cout << int(i) << " ";
        }
        cout << endl << int(p.getShapeHash(state)) << " " << int(p.getCornerIdx(state)) << endl;
        cout << int(p.getEdgeIdxs(state).first) << " " << int(p.getEdgeIdxs(state).second) << endl;
    }*/
    //std::cout << std::filesystem::current_path() << "\n";
    return 0;
}