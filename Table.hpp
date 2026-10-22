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

static constexpr size_t SHAPE_TABLE_SIZE = 102060; // (7 choose 3) * 4 * 3^6
static constexpr size_t NUM_SHAPES = 13174;
static constexpr size_t CEO_SIZE = 13174*36*256;
static constexpr size_t SEP_SIZE = 13174ULL * 362880;
static constexpr size_t NUM_DISTINCT_SHAPES = 2214;
static constexpr size_t NUM_CORNER_CYCLES = 36;
static constexpr size_t NUM_EDGE_PERMUTATIONS = 362880;
static constexpr size_t NUM_EDGE_ORIENTATIONS = 256;
static constexpr uint8_t NUM_MOVES = 9; 

class Table{
    Puppet puppet;
    PuppetShape puppet_shape;
    AlgMani am;

    public:
        Table();
        std::vector<uint16_t> shape_turns;
        std::vector<uint16_t> rank_table;
        std::vector<uint8_t> CEO_table;
        std::vector<uint8_t> SEP_table;

        template <typename E> void write_table(const std::string& filename, const std::vector<E>& data){
            std::ofstream os(filename, std::ios::binary | std::ios::trunc);
            //std::cout << "filename: "<< std::endl;
            //std::cout << std::filesystem::absolute(filename) << std::endl;
            os.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(E));
        }
        template <typename E> void write_large_table(const std::string& filename, const std::vector<E>& data){
            std::ofstream os(filename, std::ios::binary | std::ios::trunc);

            if (!os) {
                std::cerr << "error while opening file!" << std::endl;
                return;
            }

            const char* ptr = reinterpret_cast<const char*>(data.data());
            size_t remaining = data.size() * sizeof(E);

            constexpr size_t chunkSize = 256ull * 1024 * 1024;

            while (remaining > 0) {
                size_t toWrite = std::min(remaining, chunkSize);

                os.write(ptr, static_cast<std::streamsize>(toWrite));

                if (!os) {
                    std::cerr << "error while writing!" << std::endl;
                    return;
                }

                ptr += toWrite;
                remaining -= toWrite;
            }
        }

        template <typename E> void load_table(std::vector<E>& table, const std::string& file){
            std::ifstream cor(file, std::ios::binary | std::ios::ate);
            if (!cor){ std::cout << "file not found: " << file << std::endl; return;}
            std::streamsize fileSize = cor.tellg();
            cor.seekg(0, std::ios::beg);
            size_t numEntries = fileSize / sizeof(E); 
            table.resize(numEntries); 
            cor.read(reinterpret_cast<char*>(table.data()), fileSize);
        }
        template <typename E> void load_large_table(std::vector<E>& table, const std::string& file){
            std::ifstream cor(file, std::ios::binary | std::ios::ate);
            if (!cor) { std::cout << "file not found: " << file << std::endl; return;}

            std::streamsize fileSize = cor.tellg();
            cor.seekg(0, std::ios::beg);

            size_t numEntries = static_cast<size_t>(fileSize) / sizeof(E);
            table.assign(numEntries, 0); // Vektor vorbereiten

            char* buffer = reinterpret_cast<char*>(table.data());
            std::streamsize remaining = fileSize;
            std::streamsize chunkSize = 512 * 1024 * 1024;

            while (remaining > 0) {
                std::streamsize toRead = std::min(remaining, chunkSize);
                cor.read(buffer, toRead);
                
                std::streamsize actualRead = cor.gcount();
                if (actualRead == 0) break;

                buffer += actualRead;
                remaining -= actualRead;
            }
        }

        size_t CEO_Rank(const Puppet::State&);
        size_t SEP_Rank(const Puppet::State&);
        size_t EP_Rank(const Puppet::State&);

        using RankFunc = size_t (Table::*)(const Puppet::State&);
        void getShapeTables();
        //void corner_table();
        void tableFull(const std::string& filename, const uint64_t entries, RankFunc idxfunc);
        void table_full_turn(const uint64_t entries, RankFunc idxfunc);
};