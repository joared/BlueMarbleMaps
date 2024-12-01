#include "Drawable.h"

#include <CImg.h>

namespace BlueMarble
{
    class Drawable::Impl
    {
        public:
            Impl(int width, int height, int colorDepth)
                : m_raster(width, height, colorDepth)
                , m_backGroundColor(Color::blue(0.5))
                , m_disp(nullptr)
            {
                //m_disp.resize(width, height, true);
            }

            void resize(int width, int height)
            {
                m_raster.resize(width, height);
            }

            int width() const
            {
                return m_raster.width();
            }

            int height() const
            {
                return m_raster.height();
            }

            const Color& backgroundColor() const
            {
                return m_backGroundColor;
            }

            void backgroundColor(const Color& color)
            {
                m_backGroundColor = color;
            }

            void fill(int val)
            {
                m_raster.fill(val);
            }

            void drawCircle(int x, int y, double radius, const Color& color)
            {
                m_raster.drawCircle(x, y,radius,color);
            }

            void drawLine(const std::vector<Point>& points, const Color& color, double width)
            {
                m_raster.drawLine(points, color, width);
            }

            void drawPolygon(const std::vector<Point>& points, const Color& color)
            {
                m_raster.drawPolygon(points,color);
            }

            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
            {
                m_raster.drawRect(topLeft, bottomRight, color);
            }

            void drawRaster(int x, int y, const Raster& raster, double alpha)
            {
                m_raster.drawRaster(x,y,raster,alpha);
            }

            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor)
            {
                m_raster.drawText(x,y,text,color,fontSize,bcolor);
            }

            Raster& getRaster()
            {
                return m_raster;
            }

            void swapBuffers()
            {
                // Update the image and save new black draw image
                const auto& drawImg = *(cimg_library::CImg<unsigned char>*)getRaster().data();
                m_disp->display(drawImg);
                // Reset draw image
                fill(150); // TODO: fill with drawable.backGroundColor()
            }

            void setWindow(void* window)
            {
                m_disp = static_cast<cimg_library::CImgDisplay*>(window);
            }

            void* getDisplay()
            {
                return &m_disp;
            }

        private:
            Raster m_raster;
            Color  m_backGroundColor;
            cimg_library::CImgDisplay* m_disp;

    };
}