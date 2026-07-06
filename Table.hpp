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

#include <bitset>
#include <filesystem>

#include "Puppet.hpp"
#include "PuppetShape.hpp"
#include "Move.hpp"
#include "AlgMani.hpp"

class Table {
    Puppet puppet;
    PuppetShape ps;
    AlgMani am;

    static constexpr uint32_t shape_table_size = 102060; // (7 choose 3) * 4 * 3^6
    static constexpr uint32_t num_shapes = 13174;
    static constexpr uint32_t CF_size = 2214*36*256;
    static constexpr uint32_t SE_size = 2214*362880;
    static constexpr uint32_t num_distinct_shapes = 2214;
    static constexpr uint32_t num_corner_permutations = 36;
    static constexpr uint32_t num_edge_permutations = 362880;
    static constexpr uint32_t num_edge_orientations = 256;

    public:
        Table();
        /*void write_table(const std::string& filename, const void* data, size_t size) {
            std::ofstream os(filename, std::ios::binary | std::ios::trunc);
            os.write(reinterpret_cast<const char*>(data), size);
            os.close();
        }*/
        template <typename E> void write_table(const std::string& filename, const std::vector<E>& data) {
            std::ofstream os(filename, std::ios::binary | std::ios::trunc);
            //std::cout << "filename: "<< std::endl;
            //std::cout << std::filesystem::absolute(filename) << std::endl;
            os.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(E));
        }
        template <typename E> void load_table(std::vector<E>& table, const std::string& file){
            std::ifstream cor(file, std::ios::binary | std::ios::ate); // 'ate' springt zum Ende
            //std::cout << "file: "<< std::endl;
            //std::cout << std::filesystem::absolute(file) << '\n';
            if (!cor){ std::cout << "file not found" << std::endl; return;}
            std::streamsize fileSize = cor.tellg();
            cor.seekg(0, std::ios::beg);
            size_t numEntries = fileSize / sizeof(E); 
            table.resize(numEntries); 
            cor.read(reinterpret_cast<char*>(table.data()), fileSize);
        }
        void shape_table(); // [0]: valid turns + representative bit; [1]: canonial shapeidx
        void corner_flip_heuristic_full();
        void corner_flip_heuristic_turns();
        void corner_flip_heuristic_broke();
        void corner_flip_heuristic_sym();
        void shape_edge_heuristic(); 
};