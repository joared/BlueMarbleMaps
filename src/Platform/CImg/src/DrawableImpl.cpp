#include "Drawable.h"
#include "SoftwareRenderer.h"
#include <CImg.h>

namespace BlueMarble
{
    class Drawable::Impl
    {
        public:
            Impl(int width, int height, int channels)
                : m_img(width, height, 1, channels, 0)
                , m_backGroundColor(Color::blue(0.0))
                , m_disp(nullptr)
                , m_renderer(nullptr)
                , m_transform(Point(), 0, 0)
            {
                //m_disp.resize(width, height, true);
                auto data = m_img.data();
                m_renderer = std::make_shared<SoftwareRenderer>();
                m_renderer->setFrameBuffer(data, width, height, channels);
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
                m_img.resize(width, height);
                auto data = m_img.data();
                int channels = m_img.spectrum();
                m_renderer->setFrameBuffer(data, width, height, channels);
            }

            int width() const
            {
                return m_img.width();
            }

            int height() const
            {
                return m_img.height();
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
                m_img.fill(val);
            }

            void drawCircle(int x, int y, double radius, const Color& color)
            {
                m_renderer->drawCircle(x, y,radius,color);
            }

            void drawLine(const std::vector<Point>& points, const Color& color, double width)
            {
                m_renderer->drawLine(points, color, width);
            }

            void drawPolygon(const std::vector<Point>& points, const Color& color)
            {
                m_renderer->drawPolygon(points,color);
            }

            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
            {
                m_renderer->drawRect(topLeft, bottomRight, color);
            }

            void drawRaster(const RasterGeometryPtr& geometry, double alpha)
            {
                auto center = m_transform.translation();
                double scale = m_transform.scale();
                double rotation = m_transform.rotation();
                
                Raster& raster = geometry->raster();
                
                int screenWidth = geometry->bounds().width()*scale;
                int screenHeight = geometry->bounds().height()*scale;
                auto minCorner = geometry->bounds().minCorner();

                auto screenC = screenCenter();
                auto delta = minCorner - center;
                int x = delta.x()*scale + screenC.x();
                int y = delta.y()*scale + screenC.y();
                
                raster.resize(screenWidth, screenHeight, Raster::ResizeInterpolation::NearestNeighbor);

                m_renderer->drawRaster(x, y, raster, alpha);
            }

            void drawRaster(int x, int y, const Raster& raster, double alpha)
            {
                m_renderer->drawRaster(x, y, raster, alpha);
            }

            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor)
            {
                m_renderer->drawText(x, y, text, color, fontSize, bcolor);
            }

            void swapBuffers()
            {
                // Update the image and save new black draw image
                auto drawImg = cimg_library::CImg<unsigned char>(m_img.data(), m_img.width(), m_img.height(), 1, m_img.spectrum(), true);

                cimg_library::CImg<unsigned char> fixed_size_canvas(drawImg.width(), drawImg.height(), 1, drawImg.spectrum(), 0);

                // Rotate the image by 45 degrees, centered at the middle of the canvas
                cimg_library::CImg<unsigned char> rotated = drawImg.get_rotate(m_transform.rotation(), drawImg.width() / 2.0f, drawImg.height() / 2.0f);
                //cimg_library::CImg<unsigned char> rotated = drawImg.get_rotate(m_transform.rotation(), 0, 0);

                auto center1 = Point(drawImg.width() / 2.0, drawImg.height() / 2.0);
                auto center2 = Point(rotated.width() / 2.0, rotated.height() / 2.0);
                auto offset = center1 - center2;
                // Draw the rotated image onto the fixed-size canvas
                fixed_size_canvas.draw_image(offset.x(), offset.y(), 0, 0, rotated, 1.0f);

                m_disp->display(fixed_size_canvas);
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
                if (x < 0 || y < 0 || x >= m_img.width() || y >= m_img.height())
                {
                    std::cout << "Warning: Trying to read pixel outside buffer: " << x << ", " << y << "\n";
                    return Color::black();
                }
                auto img = cimg_library::CImg<unsigned char>(m_img.data(), m_img.width(), m_img.height(), 1, m_img.spectrum(), true);
                unsigned char r = img(x, y, 0, 0);
                unsigned char g = img(x, y, 0, 1);
                unsigned char b = img(x, y, 0, 2);
                unsigned char a = img(x, y, 0, 3);

                return Color(r,g,b,(double)a/255.0);
            }

            void setPixel(int x, int y, const Color& color)
            {
                if (x < 0 || y < 0 || x >= m_img.width() || y >= m_img.height())
                {
                    std::cout << "Warning: Trying to set pixel outside buffer: " << x << ", " << y << "\n";
                    return;
                }
                auto img = cimg_library::CImg<unsigned char>(m_img.data(), m_img.width(), m_img.height(), 1, m_img.spectrum(), true);
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
            cimg_library::CImg<unsigned char> m_img;
            Color  m_backGroundColor;
            cimg_library::CImgDisplay* m_disp;
            SoftwareRendererPtr m_renderer;
    };
}