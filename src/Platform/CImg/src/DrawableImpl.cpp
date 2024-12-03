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
                , m_transform(Point(), 0, 0)
            {
                //m_disp.resize(width, height, true);
                auto data = m_raster.data();
                m_renderer = std::make_shared<SoftwareRenderer>();
                m_renderer->setFrameBuffer(data, width, height, colorDepth);
            }

            const Transform& getTransform()
            {
                return m_transform;
            }

            void setTransform(const Transform &transform)
            {
                m_transform = transform;
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
                //auto rotated = Utils::rotatePoints(points, m_transform.rotation(), screenCenter());
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

                cimg_library::CImg<unsigned char> fixed_size_canvas(drawImg.width(), drawImg.height(), 1, drawImg.spectrum(), 0);

                // Rotate the image by 45 degrees, centered at the middle of the canvas
                cimg_library::CImg<unsigned char> rotated = drawImg.get_rotate(m_transform.rotation(), drawImg.width() / 2.0f, drawImg.height() / 2.0f);
                //cimg_library::CImg<unsigned char> rotated = drawImg.get_rotate(m_transform.rotation(), 0, 0);

                std::cout << "Size before: " << drawImg.width() << ",  " << drawImg.width() << "\n";
                std::cout << "Size after: " << rotated.width() << ",  " << rotated.width() << "\n";

                // Rectangle screen(0,0,drawImg.width(), drawImg.height());
                // auto screenRotated = screen.rotate(-m_transform.rotation());
                // auto offset = screenRotated.minCorner();
                auto center1 = Point(drawImg.width() / 2.0, drawImg.height() / 2.0);
                auto center2 = Point(rotated.width() / 2.0, rotated.height() / 2.0);
                auto offset = center1 - center2;
                // Draw the rotated image onto the fixed-size canvas
                fixed_size_canvas.draw_image(offset.x(), offset.y(), 0, 0, rotated, 1.0f);

                m_disp->display(fixed_size_canvas);
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
                unsigned char r = img(x, y, 0, 0);
                unsigned char g = img(x, y, 0, 1);
                unsigned char b = img(x, y, 0, 2);
                unsigned char a = img(x, y, 0, 3);

                return Color(r,g,b,(double)a/255.0);
            }

            void setPixel(int x, int y, const Color& color)
            {
                if (x < 0 || y < 0 || x >= m_raster.width() || y >= m_raster.height())
                {
                    std::cout << "Warning: Trying to set pixel outside buffer: " << x << ", " << y << "\n";
                    return;
                }
                auto img = cimg_library::CImg<unsigned char>(m_raster.data(), m_raster.width(), m_raster.height(), 1, m_raster.colorDepth(), true);
                img(x, y, 0, 0) = (unsigned char)color.r();
                img(x, y, 0, 1) = (unsigned char)color.g();
                img(x, y, 0, 2) = (unsigned char)color.b();
                img(x, y, 0, 3) = (unsigned char)(color.a()*255.0);
            }

        private:
            Point screenCenter()
            {
                return Point(width() / 2.0, height() / 2.0);
            }
            Transform m_transform;
            Raster m_raster;
            Color  m_backGroundColor;
            cimg_library::CImgDisplay* m_disp;
            SoftwareRendererPtr m_renderer;

    };
}