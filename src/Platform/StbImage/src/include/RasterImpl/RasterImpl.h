#ifdef BLUEMARBLE_USE_CIMG_RASTER_IMPL
    #error "Stb image RasterImpl.cpp has not been compiled!";
#endif

#ifndef BLUEMARBLE_RASTERIMPL
#define BLUEMARBLE_RASTERIMPL

#include "Raster.h"
#include "stb_image.h"

namespace BlueMarble
{

class Raster::Impl
{
    public:
        Impl();
        Impl(const Raster &raster);
        Impl(int width, int height, int channels, int fill);
        Impl(unsigned char* data, int width, int height, int channels);
        Impl(const std::string& filePath);
        int width() const;
        int height() const;
        int channels() const;
        void resize(int width, int height, ResizeInterpolation interpolation);
        void resize(float scaleRatio, ResizeInterpolation interpolation);
        void rotate(double angle, int cx, int cy, ResizeInterpolation interpolation);
        void fill(int val);
        void blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian);
        Raster getCrop(int x0, int y0, int x1, int y1);
        const unsigned char* data() const;
        void operator=(const Raster &raster);

    private:
        void setData(unsigned char* data, int width, int height, int channels);
        int m_width;
        int m_height;
        int m_channels;
        std::vector<unsigned char> m_data;
};

}

#endif /* BLUEMARBLE_RASTERIMPL */
