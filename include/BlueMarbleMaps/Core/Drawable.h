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
            virtual void drawCircle(int x, int y, double radius, const Color& color) = 0;
            virtual void drawLine(const std::vector<Point>& points, const Color& color, double width=1.0) = 0;
            virtual void drawPolygon(const std::vector<Point>& points, const Color& color) = 0;
            virtual void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color) = 0;
            virtual void drawRect(const Rectangle& rect, const Color& color) = 0; // Utility method, should call the above
            virtual void drawRaster(int x, int y, const Raster& raster, double alpha) = 0;
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
