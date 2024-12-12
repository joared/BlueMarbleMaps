#include "Raster.h"
// TODO: use pimpl?

using namespace BlueMarble;

void takeCImgData(cimg_library::CImg<unsigned char>& cimg, unsigned char** data, int& width, int& height, int& channels)
{
    // width = cimg.width();
    // height = cimg.height();
    // channels = cimg.spectrum();
    // *data = cimg._data;
    //cimg._data = nullptr;
}

Raster::Raster()
    : m_img(1, 1, 1, 3, 0)
    , m_data(nullptr)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
{
    
    std::cout << "Raster::Raster() Warning: Raster not initialized\n";
}

BlueMarble::Raster::Raster(const Raster &raster)
    : m_img(raster.m_img)
    , m_data(nullptr)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
{
    takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
}

Raster::Raster(int width, int height, int channels, int fill)
    : m_img(width, height, 1, channels, fill)
    , m_data(nullptr)
    , m_width(width)
    , m_height(height)
    , m_channels(channels)
{
    takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
}

Raster::Raster(unsigned char* data, int width, int height, int channels)
    : m_img(data, width, height, 1, channels, false)
    , m_data(data) // TODO: need to copy data?
    , m_width(width)
    , m_height(height)
    , m_channels(0)
{
    takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
}

Raster::Raster(const std::string& filePath)
    : m_img(cimg_library::CImg<unsigned char>(filePath.c_str()))
    , m_data()
    , m_width(0)
    , m_height(0)
    , m_channels(0)
{
    takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
}


int Raster::width() const
{
    return m_img.width();
}


int Raster::height() const
{
    return m_img.height();
}

int BlueMarble::Raster::colorDepth() const
{
    return m_img.spectrum();
}

void Raster::resize(int width, int height, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    // interpolationType = interpolationType == 0 ? -1 : interpolationType; // -1 does not work as I expect
    // takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
    //auto img = cimg_library::CImg<unsigned char>(m_data, m_width, m_height, 1, m_channels, true);
    // m_img.display();
    // cimg_library::CImgDisplay display(cimg_library::CImg<unsigned char>(), "BlueMarbleMaps Demo", 3, true, true); //*static_cast<CImgDisplay*>(map->drawable()->getDisplay());
    // //display.resize(500, 500, true);
    // display.display(m_img);
    // while (!display.is_closed() && !display.is_keyESC()) 
    // {
    //     display.wait();
    // }
    // std::cout << "Image resize1: " << img.width() << ", " << img.height() << "\n";
    // std::cout << "My Image resize1: " << m_img.width() << ", " << m_img.height() << "\n";
    // img.resize(width, height, -100, -100, interpolationType);
    // std::cout << "Image resize2: " << img.width() << ", " << img.height() << "\n";
    // std::cout << "My Image resize2: " << m_img.width() << ", " << m_img.height() << "\n";
    // takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
    m_img.resize(width, height, -100, -100, interpolationType);
    takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
    
}
void Raster::resize(float scaleRatio, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    // interpolationType = interpolationType == 0 ? -1 : interpolationType; // -1 does not work as I expect
    m_img.resize(width()*scaleRatio, height()*scaleRatio, -100, -100, interpolationType);
    takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
}

void Raster::rotate(double angle, int cx, int cy, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    m_img.rotate(angle, cx, cy, interpolationType, 0);
    takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
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
    auto raster = Raster(0,0,0,0); // prevent warning
    raster.m_img = m_img.get_crop(x0, y0, x1, y1);;
    raster.m_data = raster.m_img.data();
    raster.m_width = raster.m_img.width();
    raster.m_height = raster.m_img.height();
    raster.m_channels = raster.m_img.spectrum();

    return raster; //Raster(crop.data(), crop.width(), crop.height(), crop.spectrum());
}

const unsigned char* Raster::data() const
{
    return m_img.data();
}

void Raster::operator=(const Raster &raster)
{
    m_img = raster.m_img;
    takeCImgData(m_img, &m_data, m_width, m_height, m_channels);
}
