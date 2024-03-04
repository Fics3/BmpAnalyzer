#include <iostream>
#include "bmp.h"

int main() {
    BMP bmp("kodim15.bmp");
    bmp.saveFile("SAVE.bmp");
    bmp.saveFileByComponents("component");
}
