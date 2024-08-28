#include "Drawable.h"
#include "CImgDrawableImpl.cpp" // TODO: add macro ifdef


namespace BlueMarble
{
    // class Drawable::Impl
    // {
    //     public:
    //         void resize(int width, int height);
    //         int width();
    //         int height();
    //         void drawCircle(int x, int y, double radius, const Color& color);
    //         void drawLine(const std::vector<Point>& points, const Color& color);
    //         void drawPolygon(const std::vector<Point>& points, const Color& color);
    //         void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
    //         void drawRaster(int x, int y, const Raster& raster);
    //         void* getImage();
    // };

    Drawable::Drawable(int width, int height)
        : m_impl(new Impl(width, height))
    {}

    // Nedded for incomplete type error?
    // Drawable::Drawable()
    // {}

    void Drawable::resize(int width, int height)
    {
        m_impl->resize(width, height);
    }

    int Drawable::width() const
    {
        return m_impl->width();
    }

    int Drawable::height() const
    {
        return m_impl->height();
    }

    void Drawable::fill(int val)
    {
        m_impl->fill(val);
    }

    void Drawable::drawCircle(int x, int y, double radius, const Color& color)
    {
        return m_impl->drawCircle(x, y, radius, color);
    }

    void Drawable::drawLine(const std::vector<Point>& points, const Color& color, double width)
    {
        return m_impl->drawLine(points, color, width);
    }

    void Drawable::drawPolygon(const std::vector<Point>& points, const Color& color)
    {
        m_impl->drawPolygon(points, color);
    }

    void Drawable::drawRect(const Rectangle& rect, const Color& color)
    {
        drawRect(rect.minCorner(), rect.maxCorner(), color);
    }

    void Drawable::drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
    {
        m_impl->drawRect(topLeft, bottomRight, color);
    }

    void Drawable::drawRaster(int x, int y, const Raster& raster, double alpha)
    {
        m_impl->drawRaster(x, y, raster, alpha);
    }

    void Drawable::drawText(int x, int y, const std::string &text, const Color &color, int fontSize, const Color& backgroundColor)
    {
        m_impl->drawText(x, y, text, color, fontSize, backgroundColor);
    }

    const Raster& Drawable::getRaster() const
    {
        return m_impl->getRaster();
    }
}