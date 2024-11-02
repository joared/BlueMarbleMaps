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
                m_raster.drawCircle(x, y, radius, color);
            }

            void drawLine(const std::vector<Point>& points, const Color& color, double width)
            {
                m_raster.drawLine(points,color,width);
            }

            void drawPolygon(const std::vector<Point>& points, const Color& color)
            {
                m_raster.drawPolygon(points,color);
            }

            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
            {
                auto topLeftRounded = topLeft.round();
                auto bottomRightRounded = bottomRight.round();
                auto& img = rasterToCImg(m_raster);
                unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};

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

                if (rasterImg.spectrum() == 4)
                {
                    //img.draw_image(x, y, rasterImg.get_shared_channels(0,2), rasterImg.get_shared_channel(3), 1.0, 255);
                    img.draw_image(x, y, rasterImg, rasterImg.get_shared_channel(3), 1.0, 255);
                }
                else
                {
                    img.draw_image(x, y, 0, 0, rasterImg, alpha);
                }
            }

            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor)
            {
                // TODO: fix opacity/alpha stuff
                auto& img = rasterToCImg(m_raster);
                unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
                if (bcolor.a() > 0)
                {
                    // With background color
                    unsigned char backc[] = {bcolor.r(), bcolor.g(), bcolor.b(), (unsigned char)(bcolor.a()*255)};

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