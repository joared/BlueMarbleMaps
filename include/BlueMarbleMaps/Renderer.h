#ifndef BLUEMARBLE_RENDERER
#define BLUEMARBLE_RENDERER

#include "Core.h"
#include "Utils.h"
#include "Raster.h"

#include <memory>

namespace BlueMarble
{

    enum class RendererImplementation
    {
        Software,
        OpenGl
    };

    class Renderer
    {
        public:
            virtual void drawCircle(int x, int y, double radius, const Color& color) = 0;
            virtual void drawLine(const std::vector<Point>& points, const Color& color, double width=1.0) = 0;
            virtual void drawPolygon(const std::vector<Point>& points, const Color& color) = 0;
            virtual void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color) = 0;
            virtual void drawRaster(int x, int y, const Raster& raster, double alpha) = 0;
            virtual void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent()) = 0;
    };
    typedef std::shared_ptr<Renderer> RendererPtr;
};

#endif /* BLUEMARBLE_RENDERER */
