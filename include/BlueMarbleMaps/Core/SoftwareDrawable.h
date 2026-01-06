#ifndef BLUEMARBLE_SOFTWAREDRAWABLE
#define BLUEMARBLE_SOFTWAREDRAWABLE

#include "Drawable.h"
#include <exception>

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
            void setProjectionMatrix(const glm::dmat4& proj) override final {};
            void setViewMatrix(const glm::dmat4& viewMatrix) override final {};
            void setRenderOrigin(const Point& origin) override final {};
            void beginBatches() override final;
            void endBatches() override final;
            void resize(int width, int height)  override final;
            void drawArc(double cx, double cy, double rx, double ry, double theta, const Pen& pen, const Brush& brush) override final;
            void drawCircle(double x, double y, double radius, const Pen& pen, const Brush& brush);
            void drawLine(const LineGeometryPtr& geometry, const Pen& pen) override final;
            void drawPolygon(const PolygonGeometryPtr& geometry, const Pen& pen, const Brush& brush) override final;
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color) override final;
            void drawRect(const Rectangle& rect, const Color& color) override final; // Utility method, calls the above
            void drawRaster(const RasterGeometryPtr& raster, const Brush& brush) override final;
            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent()) override final;
            Color readPixel(int x, int y) override final;
            void setPixel(int x, int y, const Color& color) override final;
            void clearBuffer() override final;
            void swapBuffers() override final;
            virtual Raster getRaster() override final { throw std::runtime_error("SoftwareDrawable::getRaster() Not implemented"); };
            RendererImplementation renderer() override final;
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
