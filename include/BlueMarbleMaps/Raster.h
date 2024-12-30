#ifndef BLUEMARBLE_RASTER
#define BLUEMARBLE_RASTER

#include "Color.h"
#include <string>
#include <memory>

namespace BlueMarble
{
    class Raster
    {
        public:
            enum class ResizeInterpolation
            {                
                NoInterpolation = 0, 
                NearestNeighbor,
                MovingAverage,
                Linear,
                Grid,
                Cubic,
                Lanczos
            };
            Raster();
            Raster(const Raster& raster);
            Raster(Raster&& raster) noexcept;
            Raster(int width, int height, int channels, int fill=0);
            Raster(unsigned char* data, int width, int height, int channels);
            Raster(const std::string& filePath);
            ~Raster();
            int width() const;
            int height() const;
            int channels() const;
            void resize(int width, int height, ResizeInterpolation interpolation = ResizeInterpolation::NearestNeighbor);
            void resize(float scaleRatio, ResizeInterpolation interpolation = ResizeInterpolation::NearestNeighbor);
            void rotate(double angle, int cx, int cy, ResizeInterpolation interpolation = ResizeInterpolation::NearestNeighbor);
            void fill(int val);
            void blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian=false);
            Raster getCrop(int x0, int y0, int x1, int y1);

            const unsigned char* data() const;

            Raster& operator=(const Raster& raster);
            Raster& operator=(Raster&& raster) noexcept;
        private:
            class Impl;
            std::unique_ptr<Impl> m_impl;
    };
}

#endif /* BLUEMARBLE_RASTER */
