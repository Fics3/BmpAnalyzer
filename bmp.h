//
// Created by zxggx on 04.03.2024.
//

#ifndef BMPANALYZER_BMP_H
#define BMPANALYZER_BMP_H

#include <cstdint>
#include <string>
#include <vector>

class BMP {
#pragma pack(push)
#pragma pack(1)
    struct bmpHeader {
        uint16_t bfType; // must be 0x4D42
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;
    } fileHeader{};
    struct bmpInfoHeader {
        uint32_t biSize;
        int32_t biWidth;
        int32_t biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t biXPelsPerMeter;
        int32_t biYPelsPerMeter;
        uint32_t biColorsUsed;
        uint32_t biColorsImportant;
    } fileInfoHeader{};
#pragma pack(pop)
    std::vector<uint8_t> imageData;
    std::vector<uint8_t> palette;


public:
    BMP() = default;

    BMP(const std::string &filename);

    void saveFile(const std::string &filename);

    void saveFile(const std::string &filename, std::vector<uint8_t> &data);

    std::vector<uint8_t> getRComponent();

    std::vector<uint8_t> getGComponent();

    std::vector<uint8_t> getBComponent();

    std::vector<uint8_t> convertRGBToYCbCr();

    std::vector<uint8_t> convertYbCrToRGB(const std::vector<uint8_t> &data);

    void saveFileByComponents(const std::string &filename);

    double countMathExp(char component, const std::vector<uint8_t> &data);

    double countStandardDeviation(char component, const std::vector<uint8_t> &data);

    double countCorrelCoef(char component1, char component2, const std::vector<uint8_t> &data);

    double countPSNR(std::vector<uint8_t> data1, std::vector<uint8_t> data2, char component);

    std::vector<uint8_t> getData() {
        return imageData;
    }

    void decimateImageEven(int num);

    void decimateImageAvg();

    void restoreImage(int num);

private:

    uint8_t saturation(double x, int x_min, int x_max) {
        if (x < x_min) {
            return x_min;
        } else if (x > x_max) {
            return x_max;
        }
        return static_cast<uint8_t>(x);
    }
};

#endif //BMPANALYZER_BMP_H
