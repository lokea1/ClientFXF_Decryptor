#pragma once
#include "names.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iomanip>

std::vector<unsigned char> readBlockBetweenNames(const std::vector<std::string>& names, const std::string& filename, int BlockIndex, int CountBlocks, std::streampos PosInFile) {
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error opening file " << filename << std::endl;
        return {};
    }

    inputFile.seekg(0, std::ios::end);
    std::streampos fileSize = inputFile.tellg();
    inputFile.seekg(PosInFile, std::ios::beg);  
    std::vector<unsigned char> buffer(fileSize - PosInFile);
    inputFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    inputFile.close();
    auto first = std::search(buffer.begin(), buffer.end(), names[BlockIndex].begin(), names[BlockIndex].end());

    while (first != buffer.end() &&
        (first == buffer.begin() || *(first - 1) != 0x00 ||
            (first + names[BlockIndex].size() == buffer.end() || *(first + names[BlockIndex].size()) != 0x00))) {
        first = std::search(first + 1, buffer.end(), names[BlockIndex].begin(), names[BlockIndex].end());
    }

    if (first == buffer.end()) {
        std::cerr << "The first name could not be found or it does not end or start with 0x00 in the file " << PosInFile << std::endl;
        std::cerr << "Most likely the versions of CLIENTFX.FXF and CLIENTFX.FCF do not match. Please check the correctness." << std::endl;
        return {};
    }

    auto second = buffer.end();  

    if (BlockIndex < CountBlocks - 1) {
        second = std::search(first + names[BlockIndex].size() + 1, buffer.end(), names[BlockIndex + 1].begin(), names[BlockIndex + 1].end());
        while (second != buffer.end() &&
            (second == buffer.begin() || *(second - 1) != 0x00 ||
                (second + names[BlockIndex + 1].size() == buffer.end() || *(second + names[BlockIndex + 1].size()) != 0x00))) {
            second = std::search(second + 1, buffer.end(), names[BlockIndex + 1].begin(), names[BlockIndex + 1].end());
        }
    }

    auto blockStart = first;
    if (blockStart != buffer.begin())
        blockStart -= 4;

    auto blockEnd = second;
    if (blockEnd != buffer.begin())
        blockEnd -= 4;

    if (BlockIndex >= CountBlocks - 1) {
        blockEnd = second;
    }

    std::vector<unsigned char> block(blockStart, blockEnd);

    return block;
}
