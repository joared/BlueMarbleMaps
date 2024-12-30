#ifndef BLUEMARBLE_USE_CIMG_RASTER_IMPL
    #error "CImg RasterImpl.cpp has not been compiled. Define BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL to compile it!";
#endif

#ifndef BLUEMARBLE_RASTERIMPL
#define BLUEMARBLE_RASTERIMPL

#include "Raster.h"
#include "CImg.h"

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
        ~Impl() = default;
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
        Impl& operator=(const Impl& impl);
        Impl& operator=(Impl&& impl) noexcept;

    private:
        cimg_library::CImg<unsigned char> m_img; // TODO: remove this dependency when implementing pimpl
};

}

#endif /* BLUEMARBLE_RASTERIMPL */
