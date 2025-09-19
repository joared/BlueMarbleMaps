#ifndef BLUEMARBLE_SOFTWAREDRAWABLE
#define BLUEMARBLE_SOFTWAREDRAWABLE

#include "Drawable.h"
#include "Brush.h"
#include "Pen.h"

namespace BlueMarble
{    
    class SoftwareDrawable : public virtual Drawable
    {
        public:
            SoftwareDrawable(int width, int height, int colorDepth=4);
            SoftwareDrawable(const Drawable& drawable) = delete;
            ~SoftwareDrawable() = default;
            // Properties
            int width() const;
            int height() const;
            const Color& backgroundColor();
            virtual void backgroundColor(const Color& color);
            
            // Methods
            const Transform& getTransform();
            void setTransform(const Transform& transform);
            void resize(int width, int height);
            void fill(int val);
            void drawCircle(int x, int y, double radius, const Color& color);
            void drawLine(const LineGeometryPtr& geometry, const Pen& pen);
            void drawPolygon(const PolygonGeometryPtr& geometry, const Brush& brush);
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
            void drawRect(const Rectangle& rect, const Color& color); // Utility method, calls the above
            void drawRaster(const RasterGeometryPtr& raster, const Brush& brush);
            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent());
            Color readPixel(int x, int y);
            void setPixel(int x, int y, const Color& color);
            void swapBuffers();
            RendererImplementation renderer();
        protected:
            class Impl;
            Impl* m_impl;

    };
    typedef std::shared_ptr<SoftwareDrawable> SoftwareDrawablePtr;

    class SoftwareBitmapDrawable 
        : public virtual SoftwareDrawable
        , public virtual BitmapDrawable
    {
        public:
            using SoftwareDrawable::SoftwareDrawable;
    };
    typedef std::shared_ptr<SoftwareBitmapDrawable> SoftwareBitmapDrawablePtr;

    class SoftwareWindowDrawable
        : public virtual SoftwareDrawable
        , public virtual WindowDrawable
    {
        public:
            using SoftwareDrawable::SoftwareDrawable;
            void setWindow(void* window);
    };
    typedef std::shared_ptr<SoftwareWindowDrawable> SoftwareWindowDrawablePtr;
}

#endif /* BLUEMARBLE_SOFTWAREDRAWABLE */
