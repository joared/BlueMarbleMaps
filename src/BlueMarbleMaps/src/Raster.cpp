#include "Raster.h"
#include "RasterImpl.cpp"

using namespace BlueMarble;

Raster::Raster()
    : m_impl(std::make_unique<Impl>())
{
}

Raster::Raster(const Raster& raster)
    : m_impl(std::make_unique<Impl>(raster))
{
}

Raster::Raster(int width, int height, int channels, int fill)
    : m_impl(std::make_unique<Impl>(width, height, channels, fill))
{
}

Raster::Raster(unsigned char* data, int width, int height, int channels)
    : m_impl(std::make_unique<Impl>(data, width, height, channels))
{
}

Raster::Raster(const std::string& filePath)
    : m_impl(std::make_unique<Impl>(filePath))
{
}

Raster::~Raster() = default; // Needed for unique pointer to m_impl to work

int Raster::width() const
{
    return m_impl->width();
}

int Raster::height() const
{
    return m_impl->height();
}

int Raster::channels() const
{
    return m_impl->channels();
}

void Raster::resize(int width, int height, ResizeInterpolation interpolation)
{
    m_impl->resize(width, height, interpolation);
}

void Raster::resize(float scaleRatio, ResizeInterpolation interpolation)
{
    resize(width()*scaleRatio, height()*scaleRatio, interpolation);
}

void Raster::rotate(double angle, int cx, int cy, ResizeInterpolation interpolation)
{
    m_impl->rotate(angle, cx, cy, interpolation);
}

void Raster::fill(int val)
{
    m_impl->fill(val);
}

void Raster::blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian)
{
    m_impl->blur(sigmaX, sigmaY, sigmaZ, isGaussian);
}

Raster Raster::getCrop(int x0, int y0, int x1, int y1)
{
    return m_impl->getCrop(x0, y0, x1, y1);
}

const unsigned char* Raster::data() const
{
    return m_impl->data();
}

void Raster::operator=(const Raster& raster)
{
    m_impl->operator=(raster);
}