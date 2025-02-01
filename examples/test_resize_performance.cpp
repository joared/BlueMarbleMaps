#include "CImg.h"
#include "stb_image.h"
#include "BlueMarbleMaps/Core/Raster.h"
#include <vector>
#include "BlueMarbleMaps/Utility/ImageDataOperations.h"

using namespace cimg_library;

int main() 
{
    std::string filePath = "/home/joar/git-repos/BlueMarbleMaps/geodata/blue_marble_256.jpg";
    BlueMarble::Raster raster(filePath);

    int niterations = 1000;
    int64_t copyElapsed = 0;
    int64_t resizeElapsed = 0;
    for (int i(0); i<niterations; ++i)
    {
        auto t1 = BlueMarble::getTimeStampMs();
        BlueMarble::Raster r2 = raster;
        auto t2 = BlueMarble::getTimeStampMs();
        copyElapsed += t2-t1;
        r2.resize(1000, 1000);
        auto t3 = BlueMarble::getTimeStampMs();
        resizeElapsed += t3-t2;
    }
    std::cout << "Elapsed copy: " << copyElapsed << " ms\n";
    std::cout << "Elapsed resize: " << resizeElapsed << " ms\n";

    return 0;
}