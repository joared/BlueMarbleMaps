#include "Raster.h"
// TODO: use pimpl?

using namespace BlueMarble;


Raster::Raster()
    : m_img(1, 1, 1, 4, 0)
{
    std::cout << "Raster::Raster() Warning: Raster not initialized\n";
}

BlueMarble::Raster::Raster(const Raster &raster)
    : m_img(raster.m_img)
{
}

Raster::Raster(int width, int height)
    : m_img(width, height, 1, 4, 0)
{

}

Raster::Raster(void* data)
    : m_img(*static_cast<cimg_library::CImg<unsigned char>*>(data))
{
}

Raster::Raster(const std::string& filePath)
    : m_img(cimg_library::CImg<unsigned char>(filePath.c_str()))
{
}


int Raster::width() const
{
    return m_img.width();
}


int Raster::height() const
{
    return m_img.height();
}

void Raster::resize(int width, int height, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    // interpolationType = interpolationType == 0 ? -1 : interpolationType; // -1 does not work as I expect
    m_img.resize(width, height, -100, -100, interpolationType);
}

void Raster::fill(int val)
{
    m_img.fill(val);
}

void Raster::blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian)
{
    m_img.blur(sigmaX, sigmaY, sigmaZ, isGaussian);
}

Raster Raster::getCrop(int x0, int y0, int x1, int y1)
{
    auto crop = m_img.get_crop(x0, y0, x1, y1);

    return Raster((void*)&crop);
}


void* Raster::data() const
{
    return (void*)&m_img;
}

