#include "RasterImpl.h"
#include <iostream>
#include <cstring> // For memcpy
#include "stb_image_resize2.h"
#include "stb_image_write.h"

using namespace BlueMarble;

Raster::Impl::Impl()
    : m_width(0)
    , m_height(0)
    , m_channels(0)
    , m_data(nullptr)
{
    std::cout << "Raster::Raster() Warning: Raster not initialized\n";
}

Raster::Impl::Impl(const Impl& impl)
    : m_width(impl.m_width)
    , m_height(impl.m_height)
    , m_channels(impl.m_channels)
    , m_data(nullptr)
{
    std::cout << "Raster::Impl::Impl(const Impl& impl)\n";
    int size = m_width*m_height*m_channels;
    m_data = allocateData(size);
    copyData(m_data, impl.m_data, size);
}

Raster::Impl::Impl(Impl&& impl) noexcept
    : m_width(impl.m_width)
    , m_height(impl.m_height)
    , m_channels(impl.m_channels)
    , m_data(impl.m_data)
{
    std::cout << "Raster::Impl::Impl(const Impl&& impl)\n";
}

Raster::Impl::Impl(int width, int height, int channels, int fill)
    : m_width(width)
    , m_height(height)
    , m_channels(channels)
    , m_data(nullptr)
{
    int size = m_width*m_height*m_channels;
    m_data = allocateData(size);
}

Raster::Impl::Impl(unsigned char* data, int width, int height, int channels)
    : m_width(width)
    , m_height(height)
    , m_channels(channels)
    , m_data(nullptr)
{
    int size = m_width*m_height*m_channels;
    m_data = allocateData(size);
    copyData(m_data, data, size);
}

Raster::Impl::Impl(const std::string& filePath)
{
    // if (!std::filesystem::exists(filePath)) { // C++17 filesystem
    // std::cerr << "File does not exist: " << filePath << std::endl;
    // }
    stbi_set_flip_vertically_on_load(true);

    m_data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_channels, 0);

    

    if (m_data == NULL)
    {
        std::cout << "couldn't load texture: " << stbi_failure_reason() << "\n";
        return;
    }
    /*
    stbi_set_flip_vertically_on_load(true);
    
    m_data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
    if (m_data == nullptr)
    {
        std::cout << "Failed to load image: " << filePath << "\n";
        throw std::exception();
    }

    std::cout << "Stb image raster loaded: " << m_width << ", " << m_height << ", " << m_channels << "\n";
    */
}

Raster::Impl::~Impl()
{
    deallocateData(m_data);
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
    if (m_width == width && m_height == height) 
    {
        // TODO: should be part of Raster?
        return;
    }

    // Resize the image using the fastest interpolation
    stbir_pixel_layout pixelLayout; 
    
    switch (m_channels)
    {
    case 1:
        pixelLayout = stbir_pixel_layout::STBIR_1CHANNEL;
        break;
    case 2:
        pixelLayout = stbir_pixel_layout::STBIR_2CHANNEL;
        break;
    case 3:
        pixelLayout = stbir_pixel_layout::STBIR_RGB;
        break;
    case 4:
        pixelLayout = stbir_pixel_layout::STBIR_RGBA;
        //stbir_pixel_layout::STBIR_4CHANNEL
        break;
    default:
        std::cout << "Erronous number of channels: " << m_channels << "\n";
        throw std::exception();
        break;
    }

    auto newData = stbir_resize_uint8_linear(m_data, m_width, m_height, 0, 
                                             nullptr, width, height, 0, 
                                             pixelLayout);

    if (!newData) {
        std::cerr << "Failed to resize image! (" << width << ", " << height << ")" << std::endl;
        return;
    }

    // Update dimensions
    deallocateData(m_data);
    m_data = newData;
    m_width = width;
    m_height = height;
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

void Raster::Impl::save(const std::string& filePath) const
{
    // Flip vertically if needed (OpenGL's origin is bottom-left)
    stbi_flip_vertically_on_write(1);

    // Save as PNG
    stbi_write_png(filePath.c_str(), width(), height(), channels(), data(), width() * channels());
}

const unsigned char* Raster::Impl::data() const
{
    return m_data;
}

Raster::Impl& Raster::Impl::operator=(const Impl& impl)
{
    m_width = impl.m_width;
    m_height = impl.m_height;
    m_channels = impl.m_channels;

    deallocateData(m_data);
    int size = m_width*m_height*m_channels;
    m_data = allocateData(size);
    copyData(m_data, impl.m_data, size);

    return *this;
}

Raster::Impl& Raster::Impl::operator=(Impl&& impl) noexcept
{
    m_width = impl.m_width;
    m_height = impl.m_height;
    m_channels = impl.m_channels;
    m_data = impl.m_data;

    return *this;
}

unsigned char* Raster::Impl::allocateData(int size)
{
    return (unsigned char *)malloc(sizeof(unsigned char) * size);
}

void Raster::Impl::deallocateData(unsigned char* data)
{
    stbi_image_free(data);
    //free(data);
    data = nullptr;
}

void Raster::Impl::copyData(unsigned char* dest, unsigned char* src, int size)
{
    std::memcpy(dest, src, size);
}
