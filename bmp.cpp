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

    palette.resize(fileHeader.bfOffBits - sizeof(fileInfoHeader) - sizeof(fileHeader));
    file.read(reinterpret_cast<char *>(palette.data()), palette.size());


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

    file.write(reinterpret_cast<char *>(palette.data()), palette.size());

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

    file.write(reinterpret_cast<char *>(palette.data()), palette.size());

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


int getIndexComponent(char componentChar) {
    int componentIdx;
    switch (componentChar) {
        case 'r':
            componentIdx = 2;
            break;
        case 'g':
            componentIdx = 1;
            break;
        case 'b':
            componentIdx = 0;
            break;
        case 'Y':
            componentIdx = 2;
            break;
        case 'B':
            componentIdx = 1;
            break;
        case 'R':
            componentIdx = 0;
            break;
        default:
            throw std::runtime_error("component does not exist");
    }
    return componentIdx;
}


double BMP::countMathExp(char component, const std::vector<uint8_t> &data) {
    double sum = 0;

    int componentIdx = getIndexComponent(component);

    for (int i = 0; i < fileInfoHeader.biSizeImage; i += 3) {
        sum += data[i + componentIdx];
    }

    return sum / (fileInfoHeader.biHeight * fileInfoHeader.biWidth);
}

double BMP::countStandardDeviation(char component, const std::vector<uint8_t> &data) {
    double sum = 0;

    double mean = countMathExp(component, data);
    int componentIdx = getIndexComponent(component);

    for (int i = 0; i < fileInfoHeader.biSizeImage; i += 3) {
        double tmp = data[i + componentIdx] - mean;
        sum += tmp * tmp;
    }
    sum = sum / ((fileInfoHeader.biWidth * fileInfoHeader.biHeight) - 1);
    return sqrt(sum);
}

double BMP::countCorrelCoef(char component1, char component2, const std::vector<uint8_t> &data) {
    int componentIdx1 = getIndexComponent(component1);
    int componentIdx2 = getIndexComponent(component2);

    double mean1 = countMathExp(component1, data);
    double mean2 = countMathExp(component2, data);

    double deviation1 = countStandardDeviation(component1, data);
    double deviation2 = countStandardDeviation(component2, data);

    double sumCorrel = 0;
    double sumPixComDiv1 = 0;
    double sumPixComDiv2 = 0;


    for (int i = 0; i < fileInfoHeader.biSizeImage; i += 3) {
        double pixelComponent1 = data[i + componentIdx1];
        double pixelComponent2 = data[i + componentIdx2];

        double deviationPixelComponent1 = pixelComponent1 - mean1;
        double deviationPixelComponent2 = pixelComponent2 - mean2;

        sumCorrel += deviationPixelComponent1 * deviationPixelComponent2;
        sumPixComDiv1 = deviationPixelComponent1 * deviationPixelComponent2;
        sumPixComDiv2 = deviationPixelComponent1 * deviationPixelComponent2;
    }

    return (sumCorrel / (fileInfoHeader.biHeight * fileInfoHeader.biWidth)) / (deviation1 * deviation2);
}

std::vector<uint8_t> BMP::convertRGBToYCbCr() {
    std::vector<uint8_t> result;
    std::vector<uint8_t> resultY;
    std::vector<uint8_t> resultCb;
    std::vector<uint8_t> resultCr;

    resultY.reserve(imageData.size());
    resultCb.reserve(imageData.size());
    resultCr.reserve(imageData.size());

    for (int i = 0; i < imageData.size(); i += 3) {

        uint8_t r = imageData[i];
        uint8_t g = imageData[i + 1];
        uint8_t b = imageData[i + 2];


        auto y = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
        auto cb = static_cast<uint8_t>(0.5643 * (b - y) + 128);
        auto cr = static_cast<uint8_t>(0.7132 * (r - y) + 128);

        resultY.emplace_back(y);
        resultY.emplace_back(y);
        resultY.emplace_back(y);
        result.emplace_back(y);
        resultCb.emplace_back(cb);
        resultCb.emplace_back(cb);
        resultCb.emplace_back(cb);
        result.emplace_back(cb);
        resultCr.emplace_back(cr);
        resultCr.emplace_back(cr);
        resultCr.emplace_back(cr);
        result.emplace_back(cr);
    }
    std::string dir = "YCbCr";
    createNewDir(dir);
    saveFile(dir + "/Y", resultY);
    saveFile(dir + "/Cb", resultCb);
    saveFile(dir + "/Cr", resultCr);
    saveFile(dir + "/YCbCr", result);
    return result;
}

std::vector<uint8_t> BMP::convertYbCrToRGB(const std::vector<uint8_t> &data) {
    std::vector<uint8_t> result;

    result.reserve(data.size());


    for (int i = 0; i < data.size(); i += 3) {
        uint8_t Y = data[i];
        uint8_t Cb = data[i + 1];
        uint8_t Cr = data[i + 2];

        auto r = saturation(Y + 1.402 * (Cr - 128), 0, 255);
        auto g = saturation(Y - 0.714 * (Cr - 128) - 0.334 * (Cb - 128), 0, 255);
        auto b = saturation(Y + 1.772 * (Cb - 128), 0, 255);

        result.emplace_back(r);
        result.emplace_back(g);
        result.emplace_back(b);
    }
    std::string dir = "RGB";
    createNewDir(dir);

    saveFile(dir + "/reconvertedRGB", result);
    return result;
}

double BMP::countPSNR(std::vector<uint8_t> data1, std::vector<uint8_t> data2, char component) {
    auto componentIdx = getIndexComponent(component);
    double sum{};
    for (int i = 0; i < fileInfoHeader.biHeight; ++i) {
        for (int j = 0; j < fileInfoHeader.biWidth; ++j) {
            size_t pixelOffset = (i * fileInfoHeader.biWidth + j) * 3;
            sum += pow((data1[pixelOffset + componentIdx] - data2[pixelOffset + componentIdx]), 2);
        }
    }
    double result = 10 * std::log10(fileInfoHeader.biWidth * fileInfoHeader.biHeight * \
        std::pow(std::pow(2, 8) - 1, 2) / sum);
    return result;
}

void BMP::decimateImageEven(int num) {
    int originalWidth = fileInfoHeader.biWidth;
    int originalHeight = fileInfoHeader.biHeight;

    int newWidth = originalWidth / num;
    int newHeight = originalHeight / num;

    std::vector<uint8_t> decimatedImageData;

    for (int y = 0; y < originalHeight; y += num) {
        for (int x = 0; x < originalWidth; x += num) {
            int index = (y * originalWidth + x) * 3;

            decimatedImageData.push_back(imageData[index]);
            decimatedImageData.push_back(imageData[index + 1]);
            decimatedImageData.push_back(imageData[index + 2]);
        }
    }

    fileInfoHeader.biWidth = newWidth;
    fileInfoHeader.biHeight = newHeight;
    fileInfoHeader.biSizeImage = decimatedImageData.size();
    fileHeader.bfSize = decimatedImageData.size() + fileHeader.bfOffBits;

    imageData = decimatedImageData;
    saveFile("RGB/decimationEven");
}

void BMP::decimateImageAvg() {
    int originalWidth = fileInfoHeader.biWidth;
    int originalHeight = fileInfoHeader.biHeight;

    int newWidth = originalWidth / 2;
    int newHeight = originalHeight / 2;

    std::vector<uint8_t> decimatedImageData;

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            int index1 = ((2 * y) * originalWidth + (2 * x)) * 3; // left-up pixel
            int index2 = index1 + 3; // right-up pixel
            int index3 = index1 + originalWidth * 3; // left-bottom pixel
            int index4 = index3 + 3; // left bottom pixel


            uint8_t averageR = (imageData[index1] + imageData[index2] +
                                imageData[index3] + imageData[index4]) / 4;
            uint8_t averageG = (imageData[index1 + 1] + imageData[index2 + 1] +
                                imageData[index3 + 1] + imageData[index4 + 1]) / 4;
            uint8_t averageB = (imageData[index1 + 2] + imageData[index2 + 2] +
                                imageData[index3 + 2] + imageData[index4 + 2]) / 4;

            decimatedImageData.push_back(averageR);
            decimatedImageData.push_back(averageG);
            decimatedImageData.push_back(averageB);
        }
    }

    fileInfoHeader.biWidth = newWidth;
    fileInfoHeader.biHeight = newHeight;
    fileInfoHeader.biSizeImage = decimatedImageData.size();
    fileHeader.bfSize = decimatedImageData.size() + fileHeader.bfOffBits;

    imageData = decimatedImageData;
    saveFile("RGB/decimationAvg");
}

    void BMP::restoreImage(int num) {
        int originalWidth = fileInfoHeader.biWidth;
        int originalHeight = fileInfoHeader.biHeight;

        int newWidth = originalWidth * num;
        int newHeight = originalHeight * num;

        std::vector<uint8_t> restoredImageData(newWidth * newHeight * 3);

        auto decimatedImageData = imageData;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                int restoredIndex = (y * newWidth + x) * 3;
                if ((x % 2 == 0 && y % 2 == 0) || (x % 2 != 0 && y % 2 != 0)) {
                    int originalX = x / num;
                    int originalY = y / num;
                    int originalIndex = (originalY * originalWidth + originalX) * 3;
                    restoredImageData[restoredIndex] = decimatedImageData[originalIndex];
                    restoredImageData[restoredIndex + 1] = decimatedImageData[originalIndex + 1];
                    restoredImageData[restoredIndex + 2] = decimatedImageData[originalIndex + 2];
                } else {
                    if (x > 0) {
                        int neighborIndex = ((y * newWidth) + (x - 1)) * 3;
                        restoredImageData[restoredIndex] = restoredImageData[neighborIndex];
                        restoredImageData[restoredIndex + 1] = restoredImageData[neighborIndex + 1];
                        restoredImageData[restoredIndex + 2] = restoredImageData[neighborIndex + 2];
                    } else if (y > 0) {
                        int neighborIndex = (((y - 1) * newWidth) + x) * 3;
                        restoredImageData[restoredIndex] = restoredImageData[neighborIndex];
                        restoredImageData[restoredIndex + 1] = restoredImageData[neighborIndex + 1];
                        restoredImageData[restoredIndex + 2] = restoredImageData[neighborIndex + 2];
                    }
                }
            }
        }
        fileInfoHeader.biWidth = newWidth;
        fileInfoHeader.biHeight = newHeight;
        fileInfoHeader.biSizeImage = restoredImageData.size();
        fileHeader.bfSize = restoredImageData.size() + fileHeader.bfOffBits;

        imageData = restoredImageData;
        saveFile("RGB/restored");
    }
