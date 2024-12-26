#include "RasterImpl.h"
#include <iostream>
#include <cstring> // For memcpy
#include "stb_image_resize.h"

using namespace BlueMarble;

Raster::Impl::Impl()
    : m_width(0)
    , m_height(0)
    , m_channels(0)
    , m_data()
{
    std::cout << "Raster::Raster() Warning: Raster not initialized\n";
}

Raster::Impl::Impl(const Raster& raster)
    : m_width(raster.m_impl->m_width)
    , m_height(raster.m_impl->m_height)
    , m_channels(raster.m_impl->m_channels)
    , m_data(raster.m_impl->m_data)
{
}

Raster::Impl::Impl(int width, int height, int channels, int fill)
    : m_width(width)
    , m_height(height)
    , m_channels(channels)
    , m_data(width*height*channels)
{
}

Raster::Impl::Impl(unsigned char* data, int width, int height, int channels)
    : m_width(width)
    , m_height(height)
    , m_channels(channels)
    , m_data(width*height*channels)
{
    // TODO: we shouldn't make a copy here?
    m_data.assign(data, data+width*height*channels);
}

Raster::Impl::Impl(const std::string& filePath)
{
    // if (!std::filesystem::exists(filePath)) { // C++17 filesystem
    // std::cerr << "File does not exist: " << filePath << std::endl;
    // }
    stbi_set_flip_vertically_on_load(1);
    
    unsigned char* data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_channels, 0);
    if (data == nullptr)
    {
        std::cout << "Failed to load image: " << filePath << "\n";
        return;
    }

    // TODO: ideally we would just move the data into our m_data, no need to free
    m_data = std::vector<unsigned char>(data, data + m_width*m_height*m_channels);
    stbi_image_free(data);

    std::cout << "Stb image raster loaded: " << m_width << ", " << m_height << ", " << m_channels << "\n";
}


int Raster::Impl::width() const
{
    return m_width;
}


int Raster::Impl::height() const
{
    return m_height;
}

int Raster::Impl::channels() const
{
    return m_channels;
}

void Raster::Impl::resize(int width, int height, ResizeInterpolation interpolation)
{
    std::cout << "Resize in: " << m_width << ", " << m_height << ", " << m_channels << "\n";
    std::cout << "Resize out: " << width << ", " << height << ", " << m_channels << "\n";
    //std::vector<unsigned char> resizedImage(width * height * m_channels);
    //unsigned char* resizedImageData = new unsigned char[width * height * m_channels];
    std::vector<unsigned char> resizedImageData(width * height * m_channels);
    
    // Resize the image using the fastest interpolation
    int result = stbir_resize_uint8_generic(
        m_data.data(), m_width, m_height, 0,                          // Source data and dimensions
        resizedImageData.data(), width, height, 0,                  // Destination data and dimensions
        m_channels,                                             // Number of channels
        STBIR_ALPHA_CHANNEL_NONE,                                // No alpha channel handling
        0,                                                       // Default alpha value
        STBIR_EDGE_CLAMP, STBIR_FILTER_BOX, STBIR_COLORSPACE_LINEAR, // Fastest filter type
        nullptr); 

    // int result = stbir_resize_uint8(
    //     m_data.data(), m_width, m_height, 0,                     // Input image and dimensions
    //     resizedImageData.data(), width, height, 0,     // Output image and dimensions
    //     m_channels                                         // Number of channels
    // );

    if (!result) {
        std::cerr << "Failed to resize image!" << std::endl;
        //stbi_image_free(inputImage);
        return;
    }
    
    //m_data.assign(resizedImageData, resizedImageData+width*height*m_channels);
    m_data = std::move(resizedImageData);
    m_width = width;
    m_height = height;

    //delete [] resizedImageData;

}
void Raster::Impl::resize(float scaleRatio, ResizeInterpolation interpolation)
{
    resize(width()*scaleRatio, height()*scaleRatio, interpolation);
}

void Raster::Impl::rotate(double angle, int cx, int cy, ResizeInterpolation interpolation)
{
    std::cout << "Raster::Impl::rotate() not implemented!\n";
}

void Raster::Impl::fill(int val)
{
    std::cout << "Raster::Impl::fill() not implemented!\n";
}

void Raster::Impl::blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian)
{
    std::cout << "Raster::Impl::blur() not implemented!\n";
}

Raster Raster::Impl::getCrop(int x0, int y0, int x1, int y1)
{
// Validate input coordinates
    if (x0 < 0 || y0 < 0 || x1 >= m_width || y1 >= m_height || x0 > x1 || y0 > y1) {
        throw std::invalid_argument("Invalid crop coordinates.");
    }

    // Dimensions of the cropped region
    int cropWidth = x1 - x0 + 1;
    int cropHeight = y1 - y0 + 1;

    // Allocate memory for the cropped image
    auto croppedRaster = Raster(cropWidth, cropHeight, m_channels);

    // Copy the pixels from the source image to the cropped image
    for (int y = 0; y < cropHeight; ++y) {
        for (int x = 0; x < cropWidth; ++x) {
            for (int c = 0; c < m_channels; ++c) {
                // Calculate source and destination indices
                int srcIndex = ((m_height - 1 - y - y0) * m_width + (x + x0)) * m_channels + c;
                int destIndex = ((cropHeight - 1 - y) * cropWidth + x) * m_channels + c;

                // Prev implementation, flipped y
                // int srcIndex = ((y + y0) * m_width + (x + x0)) * m_channels + c;
                // int destIndex = (y * cropWidth + x) * m_channels + c;

                // Copy the pixel
                croppedRaster.m_impl->m_data[destIndex] = m_data[srcIndex];
            }
        }
    }

    return std::move(croppedRaster);
}

const unsigned char* Raster::Impl::data() const
{
    return m_data.data();
}

void Raster::Impl::operator=(const Raster &raster)
{
    m_data = raster.m_impl->m_data;
    m_width = raster.m_impl->m_width;
    m_height = raster.m_impl->m_height;
    m_channels = raster.m_impl->m_channels;
}

void BlueMarble::Raster::Impl::setData(unsigned char *data, int width, int height, int channels)
{
    // Takes ownership of data

}

