#include "SoftwareRenderer.h"

using namespace BlueMarble;


SoftwareRenderer::SoftwareRenderer()
    : m_data(nullptr)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
{
}

void SoftwareRenderer::drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
{
    auto topLeftRounded = topLeft.round();
    auto bottomRightRounded = bottomRight.round();
    auto img = cimg_library::CImg<unsigned char>(m_data, m_width, m_height, 1, m_channels, true);
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    img.draw_rectangle(topLeftRounded.x(), 
                       topLeftRounded.y(), 
                       bottomRightRounded.x(), 
                       bottomRightRounded.y(), 
                       c, 
                       (float)color.a());
}

void SoftwareRenderer::drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor)
{
    // TODO: fix opacity/alpha stuff
    auto img = cimg_library::CImg<unsigned char>(m_data, m_width, m_height, 1, m_channels, true);
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

void SoftwareRenderer::setFrameBuffer(const unsigned char *data, int width, int height, int channels)
{
    m_data = data;
    m_width = width;
    m_height = height;
    m_channels = channels;
}

void SoftwareRenderer::drawRaster(int x, int y, const Raster& raster, double alpha)
{
    auto img = cimg_library::CImg<unsigned char>(m_data, m_width, m_height, 1, m_channels, true);
    auto rasterImg = cimg_library::CImg<unsigned char>(raster.data(), raster.width(), raster.height(), 1, raster.colorDepth(), true);
    if (rasterImg.spectrum() == 4)
    {
        //img.draw_image(x, y, rasterImg.get_shared_channels(0,2), rasterImg.get_shared_channel(3), 1.0, 255);
        //img.draw_image(x, y, rasterImg, rasterImg.get_shared_channel(3), 1.0, 255);
        img.draw_image(x, y, rasterImg, rasterImg.get_shared_channel(3), alpha, 255);
    }
    else
    {
        img.draw_image(x, y, 0, 0, rasterImg, alpha);
    }
    //std::memcpy(m_data, img.data(), m_width*m_height*m_channels);
}

void SoftwareRenderer::drawLine(const std::vector<Point>& points, const Color& color, double width)
{
    auto img = cimg_library::CImg<unsigned char>(m_data, m_width, m_height, 1, m_channels, true);
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
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
            img.draw_line(x0, y0, x1, y1, c, color.a());
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

void SoftwareRenderer::drawPolygon(const std::vector<Point>& points, const Color& color)
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
    
    auto img = cimg_library::CImg<unsigned char>(m_data, m_width, m_height, 1, m_channels, true);
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    img.draw_polygon(pointsCImg, c, color.a()); // .display();
}

// void SoftwareRenderer::blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian)
// {
//     auto img = cimg_library::CImg<unsigned char>(m_data, m_width, m_height, 1, m_channels);
//     img.blur(sigmaX, sigmaY, sigmaZ, isGaussian);
// }

// Raster SoftwareRenderer::getCrop(int x0, int y0, int x1, int y1)
// {
//     auto img = cimg_library::CImg<unsigned char>(m_data, m_width, m_height, 1, m_channels);
//     auto crop = img.get_crop(x0, y0, x1, y1);

//     return Raster((void*)&crop);
// }

void SoftwareRenderer::drawCircle(int x, int y, double radius, const Color& color)
{
    auto img = cimg_library::CImg<unsigned char>(m_data, m_width, m_height, 1, m_channels, true);
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    img.draw_circle(x, y, radius, c, color.a());
}
