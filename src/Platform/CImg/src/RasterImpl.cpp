#include "RasterImpl.h"

using namespace BlueMarble;

void takeCImgData(cimg_library::CImg<unsigned char>& cimg, unsigned char** data, int& width, int& height, int& channels)
{
    // width = cimg.width();
    // height = cimg.height();
    // channels = cimg.spectrum();
    // *data = cimg._data;
    //cimg._data = nullptr;
}

Raster::Impl::Impl()
    : m_img(1, 1, 1, 3, 0)
{
    
    std::cout << "Raster::Raster() Warning: Raster not initialized\n";
}

Raster::Impl::Impl(const Impl& impl)
    : m_img(impl.m_img)
{
}

Raster::Impl::Impl(Impl&& impl) noexcept
    : m_img(std::move(impl.m_img))
{
}

Raster::Impl::Impl(int width, int height, int channels, int fill)
    : m_img(width, height, 1, channels, fill)
{
}

Raster::Impl::Impl(unsigned char* data, int width, int height, int channels)
    : m_img(data, width, height, 1, channels, false)
{
}

Raster::Impl::Impl(const std::string& filePath)
    : m_img(cimg_library::CImg<unsigned char>(filePath.c_str()))
{
}


int Raster::Impl::width() const
{
    return m_img.width();
}


int Raster::Impl::height() const
{
    return m_img.height();
}

int Raster::Impl::channels() const
{
    return m_img.spectrum();
}

void Raster::Impl::resize(int width, int height, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    m_img.resize(width, height, -100, -100, interpolationType);
}

void Raster::Impl::resize(float scaleRatio, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    // interpolationType = interpolationType == 0 ? -1 : interpolationType; // -1 does not work as I expect
    m_img.resize(width()*scaleRatio, height()*scaleRatio, -100, -100, interpolationType);
}

void Raster::Impl::rotate(double angle, int cx, int cy, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    m_img.rotate(angle, cx, cy, interpolationType, 0);
}

void Raster::Impl::fill(int val)
{
    m_img.fill(val);
}

void Raster::Impl::blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian)
{
    m_img.blur(sigmaX, sigmaY, sigmaZ, isGaussian);
}

Raster Raster::Impl::getCrop(int x0, int y0, int x1, int y1)
{
    auto raster = Raster(0,0,0,0); // prevent warning
    raster.m_impl->m_img = m_img.get_crop(x0, y0, x1, y1);

    return raster;
}

const unsigned char* Raster::Impl::data() const
{
    return m_img.data();
}

Raster::Impl& Raster::Impl::operator=(const Impl& impl)
{
    m_img = impl.m_img;
    return *this;
}

Raster::Impl& Raster::Impl::operator=(Impl&& impl) noexcept
{
    m_img = std::move(impl.m_img);
    return *this;
}
