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

            void drawRaster(const RasterGeometryPtr& geometry, double alpha)
            {
                auto center = m_transform.translation();
                double scale = m_transform.scale();
                double rotaiton = m_transform.rotation();
                

                Raster& raster = geometry->raster();
                
                int screenWidth = geometry->bounds().width()*scale;
                int screenHeight = geometry->bounds().height()*scale;
                auto minCorner = geometry->bounds().minCorner();

                auto screenC = screenCenter();
                auto delta = minCorner - center;
                int x = delta.x()*scale + screenC.x();
                int y = delta.y()*scale + screenC.y();

                std::cout << "Before: " << raster.width() << ", " << raster.height() << "\n";
                raster.resize(screenWidth, screenHeight, Raster::ResizeInterpolation::NearestNeighbor);
                std::cout << "After: " << raster.width() << ", " << raster.height() << "\n";

                std::cout << "Offset: " << Point(x,y).toString() << "\n";
                m_renderer->drawRaster(x, y, raster, alpha);


                // // Copied from Map::mapToScreen()
                // auto screenC = screenCenter();
                // //auto delta = Utils::rotatePointDegrees(raster->bounds().center(), -rotaiton, center) - center;
                // auto deltaMinCorner = geometry->bounds().minCorner() - center;
                // double xMinCorner = deltaMinCorner.x()*scale + screenC.x();
                // double yMinCorner = deltaMinCorner.y()*scale + screenC.y();

                // auto deltaMaxCorner = geometry->bounds().maxCorner() - center;
                // double xMaxCorner = deltaMaxCorner.x()*scale + screenC.x();
                // double yMaxCorner = deltaMaxCorner.y()*scale + screenC.y();

                // auto screenBounds = Rectangle(xMinCorner, yMinCorner, xMaxCorner, yMaxCorner);
                
                // Raster& rasterOrig = geometry->raster();

                // Raster raster;
                // int screenWidth;
                // int screenHeight;
                // // Crop raster if needed for efficiency
                // if (screenBounds.xMin() < 0 || screenBounds.yMin() < 0 || 
                //     screenBounds.xMax() >= width() || screenBounds.yMax() >= height())
                // {
                    
                //     // int x0 = std::max((int)round(screenBounds.xMin()),0);
                //     // int y0 = std::max((int)round(screenBounds.yMin()), 0);
                //     // int x1 = std::min((int)round(screenBounds.xMax()),width()-1);
                //     // int y1 = std::min((int)round(screenBounds.yMax()), height()-1);

                //     // // Copied from Map::screenToMap
                //     // auto sCenter = screenCenter();
                //     // double x0Map = ((double)x0 - sCenter.x()) / scale + center.x();
                //     // double y0Map = ((double)y0 - sCenter.y()) / scale + center.y();
                //     // double x1Map = ((double)x1 - sCenter.x()) / scale + center.x();
                //     // double y1Map = ((double)y1 - sCenter.y()) / scale + center.y();

                //     // // Convert to cell index
                //     // int x0MapIdx = (int)round(x0Map/geometry->cellWidth());
                //     // int y0MapIdx = (int)round(y0Map/geometry->cellHeight());
                //     // int x1MapIdx = (int)round(x1Map/geometry->cellWidth());
                //     // int y1MapIdx = (int)round(y1Map/geometry->cellHeight());

                //     // x0MapIdx = std::max((int)x0MapIdx,0);
                //     // y0MapIdx = std::max((int)y0MapIdx, 0);
                //     // x1MapIdx = std::min((int)x1MapIdx, rasterOrig.width()-1);
                //     // y1MapIdx = std::min((int)y1MapIdx, rasterOrig.height()-1);

                //     // std::cout << "Cellwidth: " << geometry->cellWidth() << "\n";
                //     // std::cout << "Cellheight: " << geometry->cellHeight() << "\n";

                //     // std::cout << "x0: " << x0Map << "\n";
                //     // std::cout << "y0: " << y0Map << "\n";
                //     // std::cout << "x1: " << x1Map << "\n";
                //     // std::cout << "y1: " << y1Map << "\n";

                //     // raster = rasterOrig.getCrop(x0MapIdx, y0MapIdx, x1MapIdx, y1MapIdx);
                //     // //screenBounds = Rectangle(x0, y0, x1, y1);
                    
                //     // // Again, use new raster to convert to screen bounds
                    
                //     // auto newBounds = Rectangle(-x0Map + 2.0*x0MapIdx, 
                //     //                            -y0Map + 2.0*y0MapIdx, 
                //     //                            -x1Map + 2.0*x1MapIdx, 
                //     //                            -y1Map + 2.0*y1MapIdx);
                //     // auto deltaMinCorner = newBounds.minCorner() - center;
                //     // double xMinCorner = deltaMinCorner.x()*scale + screenC.x();
                //     // double yMinCorner = deltaMinCorner.y()*scale + screenC.y();

                //     // auto deltaMaxCorner = newBounds.maxCorner() - center;
                //     // double xMaxCorner = deltaMaxCorner.x()*scale + screenC.x();
                //     // double yMaxCorner = deltaMaxCorner.y()*scale + screenC.y();

                //     // screenBounds = Rectangle(xMinCorner, yMinCorner, xMaxCorner, yMaxCorner);
                    
                //     // screenWidth = Rectangle(x0, y0, x1, y1).width();
                //     // screenHeight = Rectangle(x0, y0, x1, y1).height();
                // }
                // else
                // {
                //     // Copy orignal raster
                //     screenWidth = screenBounds.width();
                //     screenHeight = screenBounds.height();
                //     raster = rasterOrig;
                // }

                // std::cout << "Before: " << raster.width() << ", " << raster.height() << "\n";
                // raster.resize(screenWidth, screenHeight, Raster::ResizeInterpolation::NearestNeighbor);
                // std::cout << "After: " << raster.width() << ", " << raster.height() << "\n";

                // auto offset = screenBounds.minCorner();
                // m_renderer->drawRaster(offset.x(), offset.y(), raster, alpha);
            }

            void drawRaster(int x, int y, const Raster& raster, double alpha)
            {
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