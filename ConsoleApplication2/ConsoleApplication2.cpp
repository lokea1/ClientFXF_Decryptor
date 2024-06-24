#include "names.h"
#include "block.h"
#include "WorkConvert.h"
#include "Patterns.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <functional>
#include <chrono>

int ByteToInt(const std::vector<unsigned char>& bytes);
int readIntFromFile(const std::string& filename);
float ByteToFloat(const std::vector<unsigned char>& bytes);
void writeValueToFile(std::ofstream& outputFile, const std::string& string, auto value);
std::string readStringFromFile(const std::vector<unsigned char>& block, size_t currentPosition);

namespace fs = std::filesystem;


int main() {
    std::ofstream outputFile(outputFilename);
    if (!outputFile) {
        std::cerr << "Error creating file " << outputFilename << std::endl;
        return 1;
    }

    if (!fs::exists(filenameFCF)) {
        std::cerr << "Error: CLIENTFX.FCF file not found." << std::endl;
        std::cerr << "CLIENTFX.FCF must be in the same directory." << std::endl;
        outputFile.close();
        system("pause");
        return -1;
    }

    if (!fs::exists(filenameFXF)) {
        std::cerr << "Error: CLIENTFX.FXF file not found." << std::endl;
        std::cerr << "CLIENTFX.FXF must be in the same directory." << std::endl;
        outputFile.close();
        system("pause");
        return -1;
    }
     
    int Groups = readIntFromFile(filenameFXF);
    writeValueToFile(outputFile, "Groups: ", Groups);

    std::streampos PosInFile = 0;
    auto startTime = std::chrono::steady_clock::now();

    for (int BlockIndex = 0; BlockIndex < Groups; BlockIndex++) {
        blockFXF = readBlockBetweenNames(names, filenameFXF, BlockIndex, Groups, PosInFile);
        writeValueToFile(outputFile, "	GroupName: ", names[BlockIndex]);
        int FxInGroup = ByteToInt(std::vector<unsigned char>(blockFXF.begin(), blockFXF.begin() + 4));
        writeValueToFile(outputFile, "	FxInGroup: ", FxInGroup);
        int Phase = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + 132, blockFXF.begin() + 132 + 4));
        writeValueToFile(outputFile, "	Phase: ", Phase);
        size_t currentPosition = 0;

        while (currentPosition < blockFXF.size()) {
            bool patternFound = false;
            for (const auto& [pattern, handler] : bytePatternHandlers) {
                if (currentPosition + pattern.size() <= blockFXF.size()) {
                    if (std::equal(pattern.begin(), pattern.end(), blockFXF.begin() + currentPosition)) {
                        currentPosition = processBytePattern(blockFXF, currentPosition, outputFile, pattern, handler);
                        patternFound = true;
                        break;
                    }
                }
            }
            if (!patternFound) {
                currentPosition++;
            }
        }
        PosInFile += currentPosition;

        if (BlockIndex > 0) {
            system("cls");
            std::cout << "Developed by lokea v1.2" << std::endl;
            std::cout << "Process: " << static_cast<double>(BlockIndex + 1) / Groups * 100 << "%" << std::endl;
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            auto estimatedTotalTime = static_cast<double>(elapsedTime) / (BlockIndex + 1) * (Groups - BlockIndex - 1);
            int hours = static_cast<int>(estimatedTotalTime) / 3600;
            int minutes = (static_cast<int>(estimatedTotalTime) % 3600) / 60;
            int seconds = static_cast<int>(estimatedTotalTime) % 60;
            std::cout << "Estimated remaining time: " << hours << " hours, " << minutes << " minutes, " << seconds << " seconds" << std::endl;
        }
    }
    outputFile.close();
    return 0;
}
