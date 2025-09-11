#ifndef BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL
    #error "CImg SoftwareDrawableImpl.cpp has not been compiled. Define BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL to compile it!";
#endif

#include "BlueMarbleMaps/Core/SoftwareDrawable.h"
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
            void drawArc(float cx, float cy, float rx, float ry, double theta, const Pen& pen, const Brush& brush);
            void drawCircle(int x, int y, double radius, const Pen& pen, const Brush& brush);
            void drawLine(const LineGeometryPtr& points, const Pen& pen);
            void drawPolygon(const PolygonGeometryPtr& points, const Pen& pen, const Brush& brush);
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
            void drawRaster(const RasterGeometryPtr& geometry, double alpha);
            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor);
            void clearBuffer();
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