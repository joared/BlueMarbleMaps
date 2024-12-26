#include <CImg.h>
#include "stb_image.h"
#include "Raster.h"
#include <vector>

using namespace cimg_library;

void inline cimgIndexToXYC(int i,
                   int width, int height, int channels,
                   int& x, int& y, int& c) 
{
    x = i % width;
    y = (i / width) % height;
    c = i / (width*height);
}

int cimgXYCToIndex(int x, int y, int c,
                   int width, int height, int channels)
{
    // Taken from operator() in CImg
    return x + y*width + c*width*height*channels;
}

void inline glIndexToXYC(int i,
                 int width, int height, int channels,
                 int& x, int& y, int& c)
{
    x = (i / channels) % width;
    y = i / (channels*width);
    c = i % channels;
}

void convertCImgToOpenGL(cimg_library::CImg<unsigned char>& cimgImage, 
                         unsigned char* openglData, 
                         int width, int height, int channels) 
{
    cimgImage.mirror('y');
    for (int c = 0; c < channels; ++c) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Compute OpenGL index (row-major order)
                int openglIndex = (y * width + x) * channels + c;
                
                // Copy data
                openglData[openglIndex] = cimgImage(x,y,0,c);
            }
        }
    }
}

void convertOpenGLToCImg(cimg_library::CImg<unsigned char>& cimgImage, 
                         const unsigned char* openglData, 
                         int width, int height, int channels) 
{
    for (int c = 0; c < channels; ++c) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Compute OpenGL index (row-major order)
                int openglIndex = (y * width + x) * channels + c;
                
                // Copy data
                cimgImage(x,height-1-y,0,c) = openglData[openglIndex];
            }
        }
    }
}

void convertOpenGLToCImg2(cimg_library::CImg<unsigned char>& cimgImage, 
                         const unsigned char* openglData, 
                         int width, int height, int channels)
{
    int size = width*height*channels;
    double channelsInv = 1.0 / channels;
    double channelsWidthInv = 1.0 / (channels*width);
    for (int i(0); i<size; ++i)
    {
        // int cimgX = i % width;
        // int cimgX = (i / width) % height;
        // int cimgC = i / (width*height);

        int glX = (int)(i * channelsInv) % width;
        int glY = (int)(i * channelsWidthInv);
        int glC = i % channels;

        // std::cout << x << ", " << y << ", " << c << "\n";
        //std::cout << glX << ", " << glY << ", " << glC << "\n";
        cimgImage(glX,height-1-glY,0,glC) = openglData[i];
    }
}

int main() 
{
    std::string filePath = "/home/joar/git-repos/BlueMarbleMaps/geodata/blue_marble_256.jpg";
    CImg<unsigned char> cimgImage(filePath.c_str());
    cimgImage.display();



    BlueMarble::Raster raster(filePath);
    raster.resize(1000, 1000);
    auto raster2 = raster;
    raster = raster2.getCrop(0, 0, 999, 999);


    auto convertedImage = cimg_library::CImg<unsigned char>(raster.width(), raster.height(), 1, raster.channels());
    
    int niterations = 1000;

    // Test performance for version 2
    {
        auto start = BlueMarble::getTimeStampMs();
        for (int i(0); i < niterations; ++i)
        {
            convertOpenGLToCImg2(convertedImage, raster.data(), raster.width(), raster.height(), raster.channels());
        }
        auto elapsed = BlueMarble::getTimeStampMs() - start;
        std::cout << "Elapsed V2: " << elapsed << " ms\n";
    }

    // Test performance for version 1
    {
        auto start = BlueMarble::getTimeStampMs();
        for (int i(0); i < niterations; ++i)
        {
            convertOpenGLToCImg(convertedImage, raster.data(), raster.width(), raster.height(), raster.channels());
        }
        auto elapsed = BlueMarble::getTimeStampMs() - start;
        std::cout << "Elapsed V1: " << elapsed << " ms\n";
    }

    
    convertedImage.display();

    // int width, height, channels;
    // stbi_set_flip_vertically_on_load(1);
    // auto data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    // if (data == nullptr)
    // {
    //     std::cout << "Failed to load image: " << filePath << "\n";
    //     return -1;
    // }

    // auto convertedImage = cimg_library::CImg<unsigned char>(width, height, 1, channels);
    // convertOpenGLToCImg(convertedImage, data, width, height, channels);
    // convertedImage.display();

    return 0;
}