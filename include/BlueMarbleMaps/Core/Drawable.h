#ifndef DRAWABLE
#define DRAWABLE

#include "BlueMarbleMaps/Core/Raster.h"
#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Color.h"
#include "BlueMarbleMaps/Core/Renderer.h"
#include "BlueMarbleMaps/Core/Transform.h"
#include "BlueMarbleMaps/Core/Geometry.h"
#include "BlueMarbleMaps/Utility/Utils.h"

#include <string>
#include <vector>
#include <memory>

namespace BlueMarble
{    
    class Pen
    {
        public:
            static Pen transparent() { return Pen(); }
            Pen(bool antiAlias=false, const Color& color=Color::transparent(), double width=1.0) 
                : m_antiAlias(antiAlias)
                , m_color(color)
                , m_width(width)
            {}
            const Color& getColor() const { return m_color; };
            void setColor(const Color& color) { m_color = color; };
            bool getAntiAlias() const { return m_antiAlias; };
            void setAntiAlias(bool antiAlias) { m_antiAlias = antiAlias; };
            double getWidth() const { return m_width; }
            void setWidth(double width) { m_width = width; }
        private:
            bool m_antiAlias;
            Color m_color;
            double m_width;
    };

    class Brush
    {
        public:
            static Brush transparent() { return Brush(); }
            const Color& getColor() const { return m_color; };
            void setColor(const Color& color) { m_color = color; };
            bool getAntiAlias() const { return m_antiAlias; };
            void setAntiAlias(bool antiAlias) { m_antiAlias = antiAlias; };
        private:
            bool m_antiAlias = false;
            Color m_color = Color::transparent();
    };

    class Drawable
    {
        public:
            //Drawable(const Drawable& drawable) = delete;
            virtual ~Drawable() = default;
            // Properties
            virtual int width() const = 0;
            virtual int height() const = 0;
            virtual const Color& backgroundColor() = 0;
            virtual void backgroundColor(const Color& color) = 0;
            
            // Methods
            virtual const Transform& getTransform() = 0;
            virtual void setTransform(const Transform& transform) = 0;
            virtual void resize(int width, int height) = 0;
            virtual void fill(int val) = 0;
            virtual void drawArc(double cx, double cy, double rx, double ry, double theta, const Pen& pen, const Brush& brush) = 0;
            virtual void drawCircle(double x, double y, double radius, const Pen& pen, const Brush& brush) = 0;
            virtual void drawLine(const LineGeometryPtr& geometry, const Pen& pen) = 0;
            virtual void drawPolygon(const PolygonGeometryPtr& geometry, const Pen& pen, const Brush& brush) = 0;
            virtual void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color) = 0;
            virtual void drawRect(const Rectangle& rect, const Color& color) = 0; // Utility method, calls the above
            virtual void drawRaster(const RasterGeometryPtr& raster, double alpha) = 0;
            virtual void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent()) = 0;
            virtual Color readPixel(int x, int y) = 0;
            virtual void setPixel(int x, int y, const Color& color) = 0;
            virtual void swapBuffers() = 0;
            virtual RendererImplementation renderer() = 0;
    };
    typedef std::shared_ptr<Drawable> DrawablePtr;

    class BitmapDrawable : public virtual Drawable
    {

    };
    typedef std::shared_ptr<BitmapDrawable> BitmapDrawablePtr;

    class WindowDrawable : public virtual Drawable
    {
        public:
            virtual void setWindow(void* window) = 0;
    };
    typedef std::shared_ptr<WindowDrawable> WindowDrawablePtr;
}

#endif /* DRAWABLE */
