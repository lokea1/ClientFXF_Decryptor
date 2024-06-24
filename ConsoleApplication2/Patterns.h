#pragma once

#include <vector>
#include <utility>
#include <functional>
#include <fstream>

void handlePattern(std::ofstream& outputFile, const std::vector<unsigned char>& block, size_t patternPosition);
void handlePatternBegin(std::ofstream& outputFile, const std::vector<unsigned char>& block, size_t patternPosition);
void handlePatternCreateFX(std::ofstream& outputFile, const std::vector<unsigned char>& block, size_t patternPosition);

extern std::vector<std::pair<std::vector<unsigned char>, std::function<void(std::ofstream&, const std::vector<unsigned char>&, size_t)>>> bytePatternHandlers;