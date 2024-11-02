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
        private:
            class Impl;   // Forward declaration
            Impl* m_impl; // Using "pimpl" design pattern for fun
    };

    // TODO: It could be useful to test performance where rendering is turned off.
    // Adding an IDrawable interface and implementing a DummyDrawable that does not 
    // draw anything could be a solution.
    // class DummyDrawable : public Drawable
    // {
    //     public:
    //         DummyDrawable(Drawable& actualDrawable)
    //             : Drawable(1, 1)
    //             , m_actualDrawable(actualDrawable)
    //         {

    //         }
    //         void resize(int width, int height)
    //         {
    //             m_actualDrawable.resize(width, height);
    //         }
    //         int width() const { return m_actualDrawable.width(); };
    //         int height() const  { return m_actualDrawable.height(); };
    //         void fill(int val)  { m_actualDrawable.fill(val); };
    //         void drawCircle(int x, int y, double radius, const Color& color) {}
    //         void drawLine(const std::vector<Point>& points, const Color& color, double width=1.0)  {}
    //         void drawPolygon(const std::vector<Point>& points, const Color& color)  {}
    //         void drawRect(const Rectangle& rect, const Color& color) {}
    //         void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color) {}
    //         void drawRaster(int x, int y, const Raster& raster, double alpha)
    //         {
    //             m_actualDrawable.drawRaster(x, y, raster, alpha);
    //         }
    //         void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent()) {}
    //         const Raster& getRaster() const { return m_actualDrawable.getRaster(); }

    //     private:
    //         Drawable& m_actualDrawable;
    // };
}

#endif /* DRAWABLE */
