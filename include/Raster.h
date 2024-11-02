#ifndef RASTER
#define RASTER

#include "Color.h"

#include <string>

#include <CImg.h> // TODO: remove this dependency when implementing pimpl

namespace BlueMarble
{
    class Raster
    {
        public:
            enum class ResizeInterpolation
            {
                // - 0 = no interpolation: additional space is filled according to \p boundary_conditions.
                // - 1 = nearest-neighbor interpolation.
                // - 2 = moving average interpolation.
                // - 3 = linear interpolation.
                // - 4 = grid interpolation.
                // - 5 = cubic interpolation.
                // - 6 = lanczos interpolation.
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
            Raster(int width, int height, int depth=3, int fill=0);
            Raster(void* data);
            Raster(const std::string& filePath);
            int width() const;
            int height() const;
            int colorDepth() const;
            void resize(int x, int y, ResizeInterpolation interpolation = ResizeInterpolation::NearestNeighbor);
            void rotate(double angle, int cx, int cy, ResizeInterpolation interpolation = ResizeInterpolation::NearestNeighbor);
            void fill(int val);
            void blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian=false);
            Raster getCrop(int x0, int y0, int x1, int y1);
            void drawPolygon(const std::vector<Point>& points, const Color& color);
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
            void drawLine(const std::vector<Point>& points, const Color& color, double width);
            void drawCircle(int x, int y, double radius, const Color& color);
            void* data() const; // Specific implementation for used framework (e.g. CImg)
        private:
            cimg_library::CImg<unsigned char> m_img; // TODO: remove this dependency when implementing pimpl
    };
}

#endif /* RASTER */
