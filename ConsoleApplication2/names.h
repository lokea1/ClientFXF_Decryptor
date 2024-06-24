#pragma once
#include "block.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

std::vector<std::string> extractNamesFromFile(const std::string& filename) {
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error opening file " << filename << std::endl;
        return {};
    }
    std::vector<std::string> names;

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputFile), {});
    inputFile.close();

    std::vector<unsigned char> sequence = { 0x0D, 0x0A, 0x09, 0x4E, 0x61, 0x6D, 0x65, 0x3A, 0x20 };
    size_t seq_len = sequence.size();
    bool isFirstFound = true;

    for (size_t i = 0; i <= buffer.size() - seq_len; ++i) {
        if (std::equal(sequence.begin(), sequence.end(), buffer.begin() + i)) {
            size_t start = i + seq_len;
            size_t end = start;

            while (end < buffer.size() && !(buffer[end] == 0x0D && buffer[end + 1] == 0x0A)) {
                ++end;
            }

            if (end < buffer.size()) {
                std::string name(buffer.begin() + start, buffer.begin() + end);

                if (isFirstFound) {
                    isFirstFound = false;
                    continue;
                }

                names.push_back(name);
                i = end;
            }
        }
    }

    return names;
}