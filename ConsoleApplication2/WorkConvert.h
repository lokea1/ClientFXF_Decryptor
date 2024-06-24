#pragma once

#include "names.h"
#include "block.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <functional>

std::string filenameFCF = (fs::current_path() / "CLIENTFX.FCF").string();;
std::string filenameFXF = (fs::current_path() / "CLIENTFX.FXF").string();;
std::string outputFilename = (fs::current_path() / "CLIENTFX_Decrypted.FXF").string();

std::vector<std::string> names = extractNamesFromFile(filenameFCF);

std::vector<unsigned char> blockFXF;

template<typename Handler>
size_t processBytePattern(const std::vector<unsigned char>& blockFXF, size_t startPosition, std::ofstream& outputFile,
                          const std::vector<unsigned char>& pattern, Handler patternHandler) {

    auto it = std::search(blockFXF.begin() + startPosition, blockFXF.end(), pattern.begin(), pattern.end());

    if (it != blockFXF.end()) {
        size_t patternPosition = std::distance(blockFXF.begin(), it);
        size_t currentPosition = patternPosition + pattern.size();
        patternHandler(outputFile, blockFXF, currentPosition);

        return currentPosition;
    }

    return startPosition;
}

int ByteToInt(const std::vector<unsigned char>& bytes) {
    int result = (bytes[0] << 0) | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    return result;
}

int readIntFromFile(const std::string& filename) {
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error opening file " << filename << std::endl;
        return -1;
    }

    int value = 0;
    inputFile.read(reinterpret_cast<char*>(&value), sizeof(value));
    inputFile.close();

    return value;
}

float ByteToFloat(const std::vector<unsigned char>& bytes) {
    float result;
    std::memcpy(&result, bytes.data(), sizeof(result));

    if (std::isnan(result)) {
        return 0.0f;
    }
    return result;
}

void writeValueToFile(std::ofstream& outputFile, const std::string& string, auto value) {
    outputFile << string << value << std::endl;
}

std::string readStringFromFile(const std::vector<unsigned char>& block, size_t currentPosition) {
    std::string result;
    while (block[currentPosition] != 0x00) {
        result += block[currentPosition];
        currentPosition++;
    }
    return result;
}

std::string bytesToString(const std::vector<unsigned char>& bytes) {
    std::string result;
    for (unsigned char byte : bytes) {
        result += static_cast<char>(byte);
    }
    return result;
}


void handlePatternCreateFX(std::ofstream& outputFile, const std::vector<unsigned char>& block, size_t patternPosition) {
    size_t propNameStart = patternPosition;
    while (propNameStart > 0 && block[propNameStart - 1] >= 0x20) {
        --propNameStart;
    }
    size_t FxNamePos = propNameStart - 201;
    {
        auto it = std::find(block.begin() + FxNamePos, block.end(), 0x00);
        std::string comboString(block.begin() + FxNamePos, it);
        if (!comboString.empty()) {
            comboString.pop_back();
        }
        writeValueToFile(outputFile, "		FxName: ", comboString);
    }
    size_t FxIDPos = propNameStart - 73;
    int FxID = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + FxIDPos, blockFXF.begin() + FxIDPos + 4));
    writeValueToFile(outputFile, "		FxID: ", FxID);
    size_t LinkedPos = propNameStart - 69;
    int Linked = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + LinkedPos, blockFXF.begin() + LinkedPos + 4));
    writeValueToFile(outputFile, "		Linked: ", Linked);
    size_t LinkIDPos = propNameStart - 65;
    int LinkID = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + LinkIDPos, blockFXF.begin() + LinkIDPos + 4));
    writeValueToFile(outputFile, "		LinkID: ", LinkID);
    size_t LinkNodePos = propNameStart - 61;
    {
        auto it = std::find(block.begin() + LinkNodePos, block.end(), 0x00);
        std::string comboString(block.begin() + LinkNodePos, it);
        writeValueToFile(outputFile, "		LinkNode: ", comboString);
    }
    size_t StartTimePos = propNameStart - 29;
    float StartTime = ByteToFloat(std::vector<unsigned char>(blockFXF.begin() + StartTimePos, blockFXF.begin() + StartTimePos + 4));
    outputFile << "		StartTime: " << std::fixed << std::setprecision(6) << StartTime << std::endl;
    size_t EndTimePos = propNameStart - 25;
    float EndTime = ByteToFloat(std::vector<unsigned char>(blockFXF.begin() + EndTimePos, blockFXF.begin() + EndTimePos + 4));
    outputFile << "		EndTime: " << std::fixed << std::setprecision(6) << EndTime << std::endl;
    size_t RepeatPos = propNameStart - 21;
    int Repeat = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + RepeatPos, blockFXF.begin() + RepeatPos + 4));
    writeValueToFile(outputFile, "		Repeat: ", Repeat);
    size_t TrackIDPos = propNameStart - 17;
    int TrackID = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + TrackIDPos, blockFXF.begin() + TrackIDPos + 4));
    writeValueToFile(outputFile, "		TrackID: ", TrackID);
    size_t MinScalePos = propNameStart - 13;
    float MinScale = ByteToFloat(std::vector<unsigned char>(blockFXF.begin() + MinScalePos, blockFXF.begin() + MinScalePos + 4));
    outputFile << "		MinScale: " << std::fixed << std::setprecision(6) << MinScale << std::endl;
    size_t MaxScalePos = propNameStart - 9;
    float MaxScale = ByteToFloat(std::vector<unsigned char>(blockFXF.begin() + MaxScalePos, blockFXF.begin() + MaxScalePos + 4));
    outputFile << "		MaxScale: " << std::fixed << std::setprecision(6) << MaxScale << std::endl;
    size_t PropertiesPos = propNameStart - 5;
    int Properties = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + PropertiesPos, blockFXF.begin() + PropertiesPos + 4));
    writeValueToFile(outputFile, "		Properties: ", Properties);

    std::vector<unsigned char> propNameBytes(block.begin() + propNameStart, block.begin() + patternPosition);
    std::string propName = bytesToString(propNameBytes);
    outputFile << "			PropName: " << propName << std::endl;
    int Type = ByteToInt(std::vector<unsigned char>(block.begin() + patternPosition, block.begin() + patternPosition + 4));
    outputFile << "				Type: " << Type << std::endl;
    auto it = std::find(block.begin() + patternPosition + 4, block.end(), 0x00);
    std::string comboString(block.begin() + patternPosition + 4, it);
    outputFile << "				STRING: " << comboString << std::endl;
}

void handlePatternBegin(std::ofstream& outputFile, const std::vector<unsigned char>& block, size_t patternPosition) {
    size_t propNameStart = patternPosition;
    while (propNameStart > 0 && block[propNameStart - 1] >= 0x20) {
        --propNameStart;
    }
    size_t FxNamePos = propNameStart - 201;
    {
        auto it = std::find(block.begin() + FxNamePos, block.end(), 0x00);
        std::string comboString(block.begin() + FxNamePos, it);
        if (!comboString.empty()) {
            comboString.pop_back();
        }
        writeValueToFile(outputFile, "		FxName: ", comboString);
    }
    size_t FxIDPos = propNameStart - 73;
    int FxID = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + FxIDPos, blockFXF.begin() + FxIDPos + 4));
    writeValueToFile(outputFile, "		FxID: ", FxID);
    size_t LinkedPos = propNameStart - 69;
    int Linked = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + LinkedPos, blockFXF.begin() + LinkedPos + 4));
    writeValueToFile(outputFile, "		Linked: ", Linked);
    size_t LinkIDPos = propNameStart - 65;
    int LinkID = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + LinkIDPos, blockFXF.begin() + LinkIDPos + 4));
    writeValueToFile(outputFile, "		LinkID: ", LinkID);
    size_t LinkNodePos = propNameStart - 61;
    {
        auto it = std::find(block.begin() + LinkNodePos, block.end(), 0x00);
        std::string comboString(block.begin() + LinkNodePos, it);
        writeValueToFile(outputFile, "		LinkNode: ", comboString);
    }
    size_t StartTimePos = propNameStart - 29;
    float StartTime = ByteToFloat(std::vector<unsigned char>(blockFXF.begin() + StartTimePos, blockFXF.begin() + StartTimePos + 4));
    outputFile << "		StartTime: " << std::fixed << std::setprecision(6) << StartTime << std::endl;
    size_t EndTimePos = propNameStart - 25;
    float EndTime = ByteToFloat(std::vector<unsigned char>(blockFXF.begin() + EndTimePos, blockFXF.begin() + EndTimePos + 4));
    outputFile << "		EndTime: " << std::fixed << std::setprecision(6) << EndTime << std::endl;
    size_t RepeatPos = propNameStart - 21;
    int Repeat = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + RepeatPos, blockFXF.begin() + RepeatPos + 4));
    writeValueToFile(outputFile, "		Repeat: ", Repeat);
    size_t TrackIDPos = propNameStart - 17;
    int TrackID = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + TrackIDPos, blockFXF.begin() + TrackIDPos + 4));
    writeValueToFile(outputFile, "		TrackID: ", TrackID);
    size_t MinScalePos = propNameStart - 13;
    float MinScale = ByteToFloat(std::vector<unsigned char>(blockFXF.begin() + MinScalePos, blockFXF.begin() + MinScalePos + 4));
    outputFile << "		MinScale: " << std::fixed << std::setprecision(6) << MinScale << std::endl;
    size_t MaxScalePos = propNameStart - 9;
    float MaxScale = ByteToFloat(std::vector<unsigned char>(blockFXF.begin() + MaxScalePos, blockFXF.begin() + MaxScalePos + 4));
    outputFile << "		MaxScale: " << std::fixed << std::setprecision(6) << MaxScale << std::endl;
    size_t PropertiesPos = propNameStart - 5;
    int Properties = ByteToInt(std::vector<unsigned char>(blockFXF.begin() + PropertiesPos, blockFXF.begin() + PropertiesPos + 4));
    writeValueToFile(outputFile, "		Properties: ", Properties);

    std::vector<unsigned char> propNameBytes(block.begin() + propNameStart, block.begin() + patternPosition);
    std::string propName = bytesToString(propNameBytes);
    outputFile << "			PropName: " << propName << std::endl;
    int Type = ByteToInt(std::vector<unsigned char>(block.begin() + patternPosition, block.begin() + patternPosition + 4));
    outputFile << "				Type: " << Type << std::endl;
    auto it = std::find(block.begin() + patternPosition + 4, block.end(), 0x00);
    std::string comboString(block.begin() + patternPosition + 4, it);
    outputFile << "				COMBO: " << comboString << std::endl;
}

void handlePattern(std::ofstream& outputFile, const std::vector<unsigned char>& block, size_t patternPosition) {
    size_t propNameStart = patternPosition;
    while (propNameStart > 0 && block[propNameStart - 1] >= 0x20) {
        --propNameStart;
    }
    std::vector<unsigned char> propNameBytes(block.begin() + propNameStart, block.begin() + patternPosition);
    std::string propName = bytesToString(propNameBytes);
    outputFile << "			PropName: " << propName << std::endl;
    int Type = ByteToInt(std::vector<unsigned char>(block.begin() + patternPosition, block.begin() + patternPosition + 4));
    outputFile << "				Type: " << Type << std::endl;
         if (Type == 0) {
        auto it = std::find(block.begin() + patternPosition + 4, block.end(), 0x00);
        std::string comboString(block.begin() + patternPosition + 4, it);
        outputFile << "				STRING: " << comboString << std::endl;
    }
    else if (Type == 1) {
        int MenuLayer = ByteToInt(std::vector<unsigned char>(block.begin() + patternPosition + 4, block.begin() + patternPosition + 8));
        outputFile << "				INTEGER: " << MenuLayer << std::endl;
    }
    else if (Type == 2) {
        float x = ByteToFloat(std::vector<unsigned char>(block.begin() + patternPosition + 4, block.begin() + patternPosition + 8));
        outputFile << "				FLOAT: " << std::fixed << std::setprecision(6) << x << std::endl;
    }
    else if (Type == 3) {
        auto it = std::find(block.begin() + patternPosition + 4, block.end(), 0x00);
        std::string comboString(block.begin() + patternPosition + 4, it);
        outputFile << "				COMBO: " << comboString << std::endl;
    }
    else if (Type == 4) {
        float x = ByteToFloat(std::vector<unsigned char>(block.begin() + patternPosition + 4, block.begin() + patternPosition + 8));
        float y = ByteToFloat(std::vector<unsigned char>(block.begin() + patternPosition + 8, block.begin() + patternPosition + 12));
        float z = ByteToFloat(std::vector<unsigned char>(block.begin() + patternPosition + 12, block.begin() + patternPosition + 16));
        outputFile << "				VECTOR: " << std::fixed << std::setprecision(6) << x << " " << y << " " << z << std::endl;
    }
    else if (Type == 5) {
        float w = ByteToFloat(std::vector<unsigned char>(block.begin() + patternPosition + 4, block.begin() + patternPosition + 8));
        float x = ByteToFloat(std::vector<unsigned char>(block.begin() + patternPosition + 8, block.begin() + patternPosition + 12));
        float y = ByteToFloat(std::vector<unsigned char>(block.begin() + patternPosition + 12, block.begin() + patternPosition + 16));
        float z = ByteToFloat(std::vector<unsigned char>(block.begin() + patternPosition + 16, block.begin() + patternPosition + 20));
        outputFile << "				VECTOR4: " << std::fixed << std::setprecision(6) << w << " " << x << " " << y << " " << z << std::endl;
    }
    else if (Type == 6) {
        float v = ByteToFloat(std::vector<unsigned char>(block.begin() + patternPosition + 4, block.begin() + patternPosition + 8));
        unsigned char w_byte = block[patternPosition + 8];
        unsigned char x_byte = block[patternPosition + 9];
        unsigned char y_byte = block[patternPosition + 10];
        unsigned char z_byte = block[patternPosition + 11];
        float w = static_cast<float>(w_byte) / 255.0f;
        float x = static_cast<float>(x_byte) / 255.0f;
        float y = static_cast<float>(y_byte) / 255.0f;
        float z = static_cast<float>(z_byte) / 255.0f;
        outputFile << "				CLRKEY: " << std::fixed << std::setprecision(6) << v << " " << w << " " << x << " " << y << " " << z << std::endl;
    }
    else if (Type == 7) {
        auto it = std::find(block.begin() + patternPosition + 4, block.end(), 0x00);
        std::string comboString(block.begin() + patternPosition + 4, it);
        outputFile << "				PATH: " << comboString << std::endl;
    }
}
