#include "SoftwareDrawable.h"
#include "SoftwareDrawableImpl.h"


namespace BlueMarble
{
    // TODO: add declaration of Impl
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

    SoftwareDrawable::SoftwareDrawable(int width, int height, int colorDepth)
        : m_impl(new Impl(width, height, colorDepth))
    {}

    // Nedded for incomplete type error?
    // Drawable::Drawable()
    // {}

    const Transform &SoftwareDrawable::getTransform()
    {
        return m_impl->getTransform();
    }

    void SoftwareDrawable::setTransform(const Transform &transform)
    {
        m_impl->setTransform(transform);
    }

    void SoftwareDrawable::resize(int width, int height)
    {
        m_impl->resize(width, height);
    }

    int SoftwareDrawable::width() const
    {
        return m_impl->width();
    }

    int SoftwareDrawable::height() const
    {
        return m_impl->height();
    }

    const Color& SoftwareDrawable::backgroundColor()
    {
        return m_impl->backgroundColor();
    }

    void SoftwareDrawable::backgroundColor(const Color &color)
    {
        m_impl->backgroundColor(color);
    }

    void SoftwareDrawable::fill(int val)
    {
        m_impl->fill(val);
    }

    void SoftwareDrawable::drawCircle(int x, int y, double radius, const Color& color)
    {
        m_impl->drawCircle(x, y, radius, color);
    }

    void SoftwareDrawable::drawLine(const std::vector<Point>& points, const Color& color, double width)
    {
        m_impl->drawLine(points, color, width);
    }

    void SoftwareDrawable::drawPolygon(const std::vector<Point>& points, const Color& color)
    {
        m_impl->drawPolygon(points, color);
    }

    void SoftwareDrawable::drawRect(const Rectangle& rect, const Color& color)
    {
        drawRect(rect.minCorner(), rect.maxCorner(), color);
    }

    void SoftwareDrawable::drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
    {
        m_impl->drawRect(topLeft, bottomRight, color);
    }

    void SoftwareDrawable::drawRaster(int x, int y, const Raster& raster, double alpha)
    {
        m_impl->drawRaster(x, y, raster, alpha);
    }

    void SoftwareDrawable::drawRaster(const RasterGeometryPtr& raster, double alpha)
    {
        m_impl->drawRaster(raster, alpha);
    }

    void SoftwareDrawable::drawText(int x, int y, const std::string &text, const Color &color, int fontSize, const Color& backgroundColor)
    {
        m_impl->drawText(x, y, text, color, fontSize, backgroundColor);
    }

    void SoftwareDrawable::swapBuffers()
    {
        m_impl->swapBuffers();
    }

    RendererImplementation SoftwareDrawable::renderer()
    {
        return m_impl->renderer();
    }

    Color SoftwareDrawable::readPixel(int x, int y)
    {
        return m_impl->readPixel(x, y);
    }

    void SoftwareDrawable::setPixel(int x, int y, const Color &color)
    {
        m_impl->setPixel(x, y, color);
    }

    void SoftwareWindowDrawable::setWindow(void* window)
    {
        m_impl->setWindow(window);
    }
}