#ifndef DRAWABLE
#define DRAWABLE

#include "Core.h"
#include "Raster.h"
#include "Utils.h"
#include "Color.h"
#include "Renderer.h"
#include "Transform.h"

#include <string>
#include <vector>
#include <memory>

namespace BlueMarble
{    
    class Drawable
    {
        public:
            Drawable(int width, int height, int colorDepth=4);
            Drawable(const Drawable& drawable) = delete;
            virtual ~Drawable() = default;
            // Properties
            int width() const;
            int height() const;
            const Color& backgroundColor();
            void backgroundColor(const Color& color);
            
            // Methods
            const Transform& getTransform();
            void setTransform(const Transform& transform);
            void resize(int width, int height);
            void fill(int val);
            void drawCircle(int x, int y, double radius, const Color& color);
            void drawLine(const std::vector<Point>& points, const Color& color, double width=1.0);
            void drawPolygon(const std::vector<Point>& points, const Color& color);
            void drawRect(const Rectangle& rect, const Color& color); // Utility method
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
            void drawRaster(int x, int y, const Raster& raster, double alpha);
            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent());
            Raster& getRaster();
            void swapBuffers();
            RendererImplementation renderer();

            Color readPixel(int x, int y);
            void setPixel(int x, int y, const Color& color);
        protected:
            class Impl;   // Forward declaration
            Impl* m_impl; // Using "pimpl" design pattern for fun
    };
    typedef std::shared_ptr<Drawable> DrawablePtr;

    class BitmapDrawable : public Drawable
    {
        public:
            using Drawable::Drawable;
    };
    typedef std::shared_ptr<BitmapDrawable> BitmapDrawablePtr;

    class WindowDrawable : public Drawable
    {
        public:
            using Drawable::Drawable;
            void setWindow(void* window);
    };
    typedef std::shared_ptr<WindowDrawable> WindowDrawablePtr;
}

#endif /* DRAWABLE */
