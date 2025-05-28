#ifndef BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL
    #error "CImg SoftwareDrawableImpl.cpp has not been compiled. Define BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL to compile it!";
#endif

#include "Core/SoftwareDrawable.h"
#include "CImg.h"

namespace BlueMarble
{
    class SoftwareDrawable::Impl
    {
        public:
            Impl(int width, int height, int channels);
            const Transform& getTransform();
            void setTransform(const Transform &transform);
            void resize(int width, int height);
            int width() const;
            int height() const;
            const Color& backgroundColor() const;
            void backgroundColor(const Color& color);
            void fill(int val);
            void drawCircle(int x, int y, double radius, const Color& color);
            void drawLine(const LineGeometryPtr& points, const Color& color, double width);
            void drawPolygon(const PolygonGeometryPtr& points, const Color& color);
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
            void drawRaster(const RasterGeometryPtr& geometry, const Brush& brush);
            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor);
            void swapBuffers();
            void setWindow(void* window);
            RendererImplementation renderer();
            Color readPixel(int x, int y);
            void setPixel(int x, int y, const Color& color);
        private:
            Point screenCenter();

            Transform m_transform;
            cimg_library::CImg<unsigned char> m_img;
            Color  m_backGroundColor;
            cimg_library::CImgDisplay* m_disp;
    };
}