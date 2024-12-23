#ifndef DRAWABLE
#define DRAWABLE

#include "Core.h"
#include "Raster.h"
#include "Utils.h"
#include "Color.h"

#include <string>
#include <vector>

namespace BlueMarble
{    
    class Drawable
    {
        public:
            Drawable(int width, int height, int colorDepth=3);
            Drawable(const Drawable& drawable) = delete;
            // Properties
            int width() const;
            int height() const;
            const Color& backgroundColor();
            void backgroundColor(const Color& color);
            
            // Methods
            void resize(int width, int height);
            void fill(int val);
            void drawCircle(int x, int y, double radius, const Color& color);
            void drawLine(const std::vector<Point>& points, const Color& color, double width=1.0);
            void drawPolygon(const std::vector<Point>& points, const Color& color);
            void drawRect(const Rectangle& rect, const Color& color);
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
            void drawRaster(int x, int y, const Raster& raster, double alpha);
            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent());
            Raster& getRaster();
            void swapBuffers();
            void* getDisplay();
        private:
            class Impl;   // Forward declaration
            Impl* m_impl; // Using "pimpl" design pattern for fun
    };
}

#endif /* DRAWABLE */
