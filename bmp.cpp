#include <filesystem>
#include <fstream>
#include <iostream>
#include <complex>
#include "bmp.h"

BMP::BMP(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);

    if (!file.is_open()) {
        throw std::runtime_error("Error opening file");
    }

    file.read(reinterpret_cast<char *>(&fileHeader), sizeof(fileHeader));

    if (fileHeader.bfType != 0x4D42) {
        throw std::runtime_error("Invalid BMP file");
    }

    file.read(reinterpret_cast<char *>(&fileInfoHeader), sizeof(fileInfoHeader));

    if (fileInfoHeader.biBitCount != 24) {
        throw std::runtime_error("Only 24-bit BMP files are supported");
    }

    imageData.resize(fileInfoHeader.biSizeImage + fileHeader.bfOffBits);
    file.read(reinterpret_cast<char *>(imageData.data()), imageData.size());

    file.close();
}

void BMP::saveFile(const std::string &filename) {
    std::ofstream file(filename + ".bmp", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    file.write(reinterpret_cast<char *>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<char *>(&fileInfoHeader), sizeof(fileInfoHeader));

    file.write(reinterpret_cast<char *>(const_cast<std::vector<uint8_t> &>(imageData).data()), imageData.size());

    file.close();
}

void BMP::saveFile(const std::string &filename, std::vector<uint8_t> &data) {
    std::ofstream file(filename + ".bmp", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    file.write(reinterpret_cast<char *>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<char *>(&fileInfoHeader), sizeof(fileInfoHeader));

    file.write(reinterpret_cast<char *>(const_cast<std::vector<uint8_t> &>(data).data()), data.size());

    file.close();
}

void createNewDir(std::string &dir) {
    if (!std::filesystem::is_directory(std::filesystem::path(std::filesystem::current_path() / dir))) {
        std::filesystem::create_directory(dir);
    }
}

std::vector<uint8_t> BMP::getRComponent() {
    std::vector<uint8_t> RData{imageData};
    for (int i = 0; i < RData.size(); i += 3) {
        RData[i] = 0x00;
        RData[i + 1] = 0x00;
    }
    return RData;
}

std::vector<uint8_t> BMP::getGComponent() {
    std::vector<uint8_t> GData{imageData};
    for (int i = 0; i < GData.size(); i += 3) {
        GData[i] = 0x00;
        GData[i + 2] = 0x00;
    }
    return GData;
}

std::vector<uint8_t> BMP::getBComponent() {
    std::vector<uint8_t> BData{imageData};
    for (int i = 0; i < BData.size(); i += 3) {
        BData[i + 1] = 0x00;
        BData[i + 2] = 0x00;
    }
    return BData;
}

void BMP::saveFileByComponents(const std::string &filename) {
    std::string dir = "component";
    createNewDir(dir);

    auto R = getRComponent();
    auto G = getGComponent();
    auto B = getBComponent();

    saveFile(dir + "/R" + filename, R);
    saveFile(dir + "/G" + filename, G);
    saveFile(dir + "/B" + filename, B);
}






