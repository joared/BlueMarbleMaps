#include "Drawable.h"

#include <CImg.h>

namespace BlueMarble
{
    class Drawable::Impl
    {
        public:
            Impl(int width, int height)
                : m_raster(width, height)
                , m_backGroundColor(Color::blue(0.5))
            {

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
                auto& img = rasterToCImg(m_raster);
                unsigned char c[] = {color.r(), color.g(), color.b()};
                img.draw_circle(x, y, radius, c, color.a());
            }

            void drawLine(const std::vector<Point>& points, const Color& color, double width)
            {
                auto img = static_cast<cimg_library::CImg<unsigned char>*>(m_raster.data());

                unsigned char c[] = {color.r(), color.g(), color.b()};
                int size = points.size();
                auto line = points;
                for (int i(0); i < size-1; ++i)
                {
                    auto& p1 = line[i];
                    auto& p2 = line[i+1];
                    int x0 = std::round(p1.x());
                    int y0 = std::round(p1.y());
                    int x1 = std::round(p2.x());
                    int y1 = std::round(p2.y());
                    if (width <= 1.0)
                        img->draw_line(x0, y0, x1, y1, c, color.a());
                    else
                    {
                        // Draw the line as a polygon
                        auto start = Point(x0, y0);
                        auto end = Point(x1, y1);
                        auto v = (end-start).norm();    // unit vector parallell to line
                        auto v2 = Point(-v.y(), v.x()); // unit vector norm to the line
                        
                        std::vector<Point> polygon;
                        polygon.push_back(start + v2*width*0.5);
                        polygon.push_back(start - v2*width*0.5);
                        polygon.push_back(end - v2*width*0.5);
                        polygon.push_back(end + v2*width*0.5);
                        drawPolygon(polygon, color);
                    }
                }
            }

            void drawPolygon(const std::vector<Point>& points, const Color& color)
            {
                assert(points.size() > 2);

                auto iterator = points.begin();
                cimg_library::CImg<int> pointsCImg(points.size(),2);
                cimg_forX(pointsCImg,i) 
                { 
                    auto& p = *(iterator++);
                    pointsCImg(i,0) = p.x(); 
                    pointsCImg(i,1) = p.y(); 
                }
                
                auto& img = rasterToCImg(m_raster);
                unsigned char c[] = {color.r(), color.g(), color.b()};
                img.draw_polygon(pointsCImg, c, color.a()); // .display();
            }

            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
            {
                auto topLeftRounded = topLeft.round();
                auto bottomRightRounded = bottomRight.round();
                auto& img = rasterToCImg(m_raster);
                unsigned char c[] = {color.r(), color.g(), color.b()};

                img.draw_rectangle(topLeftRounded.x(), 
                                   topLeftRounded.y(), 
                                   bottomRightRounded.x(), 
                                   bottomRightRounded.y(), 
                                   c, 
                                   (float)color.a());
            }

            void drawRaster(int x, int y, const Raster& raster, double alpha)
            {
                auto& img = rasterToCImg(m_raster);
                auto& rasterImg = rasterToCImg(raster);
                img.draw_image(x, y, 0, 0, rasterImg, alpha);
            }

            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor)
            {
                auto& img = rasterToCImg(m_raster);
                unsigned char c[] = {color.r(), color.g(), color.b()};
                if (bcolor.a() > 0)
                {
                    // With background color
                    unsigned char backc[] = {bcolor.r(), bcolor.g(), bcolor.b()};
                    img.draw_text(x, y, text.c_str(), backc, backc, bcolor.a(), fontSize);
                    img.draw_text(x, y, text.c_str(), c, 0, color.a(), fontSize);
                }
                else
                {
                    // Without background color
                    img.draw_text(x, y, text.c_str(), c, 0, color.a(), fontSize);
                }
            }

            Raster& getRaster()
            {
                return m_raster;
            }

        private:
            cimg_library::CImg<unsigned char>& rasterToCImg(const Raster& raster)
            {
                return *static_cast<cimg_library::CImg<unsigned char>*>(raster.data());
            }
            Raster m_raster;
            Color  m_backGroundColor;

    };
}