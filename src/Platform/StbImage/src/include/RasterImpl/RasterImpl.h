#ifdef BLUEMARBLE_USE_CIMG_RASTER_IMPL
    #error "Stb image RasterImpl.cpp has not been compiled!";
#endif

#ifndef BLUEMARBLE_RASTERIMPL
#define BLUEMARBLE_RASTERIMPL

#include "BlueMarbleMaps/Core/Raster.h"
#include "stb_image.h"

namespace BlueMarble
{

class Raster::Impl
{
    public:
        Impl();
        Impl(const Impl& impl);
        Impl(Impl&& impl) noexcept;
        Impl(int width, int height, int channels, int fill);
        Impl(unsigned char* data, int width, int height, int channels);
        Impl(const std::string& filePath);
        ~Impl();
        int width() const;
        int height() const;
        int channels() const;
        int getCellIndexAt(int x, int y) const;
        int getIntegerAt(int x, int y) const;
        float getFloatAt(int x, int y) const;
        Color getColorAt(int x, int y) const;
        void setColorAt(int x, int y, const Color& c);
        void resize(int width, int height, ResizeInterpolation interpolation);
        void resize(float scaleRatio, ResizeInterpolation interpolation);
        void rotate(double angle, int cx, int cy, ResizeInterpolation interpolation);
        void fill(int val);
        void blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian);
        Raster getCrop(int x0, int y0, int x1, int y1);
        void save(const std::string& filePath) const;
        void* data() const;

        Impl& operator=(const Impl& impl);
        Impl& operator=(Impl&& impl) noexcept;
    private:
        static unsigned char* allocateData(int size);
        static void deallocateData(unsigned char* data);
        static void copyData(unsigned char* dest, unsigned char* src, int size);

        int m_width;
        int m_height;
        int m_channels;
        unsigned char* m_data;
};

}

#endif /* BLUEMARBLE_RASTERIMPL */
