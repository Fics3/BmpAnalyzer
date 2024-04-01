#include <iostream>
#include "bmp.h"

//4ea
int main() {
    BMP bmp("kodim15.bmp");
    bmp.saveFile("SAVE.bmp");
    bmp.saveFileByComponents("component");

    std::cout << "Coefficient correl between b and g: " << bmp.countCorrelCoef('b', 'g', bmp.getData()) << "\n";
    std::cout << "Coefficient correl between r and g: " << bmp.countCorrelCoef('r', 'g', bmp.getData()) << "\n";
    std::cout << "Coefficient correl between b and r: " << bmp.countCorrelCoef('b', 'r', bmp.getData()) << "\n";

    BMP bmpR("component/Rcomponent.bmp");
    BMP bmpB("component/Bcomponent.bmp");

//    std::cout<<bmp.countPSNR(bmpB.getData(), bmpR.getData(), 'g') << "\n";

    auto yCbCr = bmp.convertRGBToYCbCr();

    std::cout << "Coefficient correl between Cr and Rb: " << bmp.countCorrelCoef('R', 'B', yCbCr) << "\n";
    std::cout << "Coefficient correl between Cr and Y : " << bmp.countCorrelCoef('R', 'Y', yCbCr) << "\n";
    std::cout << "Coefficient correl between Y  and Cb: " << bmp.countCorrelCoef('Y', 'B', yCbCr) << "\n";


    auto rgbRecovered = bmp.convertYbCrToRGB(yCbCr);

    std::cout << "PSNR r: " << bmp.countPSNR(bmp.getData(), rgbRecovered, 'r') << "\n";
    std::cout << "PSNR b: " << bmp.countPSNR(bmp.getData(), rgbRecovered, 'b') << "\n";
    std::cout << "PSNR g: " << bmp.countPSNR(bmp.getData(), rgbRecovered, 'g') << "\n";

    BMP forDecimateEven("YCbCr/YCbCr.bmp");
    forDecimateEven.decimateImageEven(2);

    BMP forDecimateAvg("YCbCr/YCbCr.bmp");
    forDecimateAvg.decimateImageAvg();

    forDecimateEven.restoreImage(2);
}
