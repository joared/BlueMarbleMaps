#include "Drawable.h"
#include "SoftwareRenderer.h"
#include <CImg.h>

namespace BlueMarble
{
    class Drawable::Impl
    {
        public:
            Impl(int width, int height, int colorDepth)
                : m_raster(width, height, colorDepth)
                , m_backGroundColor(Color::blue(0.0))
                , m_disp(nullptr)
                , m_renderer(nullptr)
            {
                //m_disp.resize(width, height, true);
                auto data = m_raster.data();
                m_renderer = std::make_shared<SoftwareRenderer>();
                m_renderer->setFrameBuffer(data, width, height, colorDepth);
            }

            void resize(int width, int height)
            {
                m_raster.resize(width, height);
                auto data = m_raster.data();
                int channels = m_raster.colorDepth();
                m_renderer->setFrameBuffer(data, width, height, channels);
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
                //m_raster.drawCircle(x, y,radius,color);
                m_renderer->drawCircle(x, y,radius,color);
            }

            void drawLine(const std::vector<Point>& points, const Color& color, double width)
            {
                //m_raster.drawLine(points, color, width);
                m_renderer->drawLine(points, color, width);
            }

            void drawPolygon(const std::vector<Point>& points, const Color& color)
            {
                //m_raster.drawPolygon(points,color);
                m_renderer->drawPolygon(points,color);
            }

            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
            {
                //m_raster.drawRect(topLeft, bottomRight, color);
                m_renderer->drawRect(topLeft, bottomRight, color);
            }

            void drawRaster(int x, int y, const Raster& raster, double alpha)
            {
                //m_raster.drawRaster(x,y,raster,alpha);
                m_renderer->drawRaster(x, y, raster, alpha);
            }

            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor)
            {
                //m_raster.drawText(x,y,text,color,fontSize,bcolor);
                m_renderer->drawText(x, y, text, color, fontSize, bcolor);
            }

            Raster& getRaster()
            {
                return m_raster;
            }

            void swapBuffers()
            {
                // Update the image and save new black draw image
                auto drawImg = cimg_library::CImg<unsigned char>(m_raster.data(), m_raster.width(), m_raster.height(), 1, m_raster.colorDepth(), true);
                m_disp->display(drawImg);
                // Reset draw image
                //fill(150); // TODO: fill with drawable.backGroundColor()
            }

            void setWindow(void* window)
            {
                m_disp = static_cast<cimg_library::CImgDisplay*>(window);
            }

            RendererImplementation renderer()
            {
                return RendererImplementation::Software;
            }

            Color readPixel(int x, int y)
            {
                if (x < 0 || y < 0 || x >= m_raster.width() || y >= m_raster.height())
                {
                    std::cout << "Warning: Trying to read pixel outside buffer: " << x << ", " << y << "\n";
                    return Color::black();
                }
                auto img = cimg_library::CImg<unsigned char>(m_raster.data(), m_raster.width(), m_raster.height(), 1, m_raster.colorDepth(), true);
                auto r = img(x, y, 1, 0);
                auto g = img(x, y, 1, 1);
                auto b = img(x, y, 1, 2);
                auto a = img(x, y, 1, 3);

                return Color(r,g,b,(double)a/255.0);
            }

        private:
            Raster m_raster;
            Color  m_backGroundColor;
            cimg_library::CImgDisplay* m_disp;
            SoftwareRendererPtr m_renderer;

    };
}