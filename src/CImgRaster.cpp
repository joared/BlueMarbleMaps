#include "Raster.h"
// TODO: use pimpl?

using namespace BlueMarble;


Raster::Raster()
    : m_img(1, 1, 1, 3, 0)
{
    std::cout << "Raster::Raster() Warning: Raster not initialized\n";
}

BlueMarble::Raster::Raster(const Raster &raster)
    : m_img(raster.m_img)
{
}

Raster::Raster(int width, int height, int depth, int fill)
    : m_img(width, height, 1, depth, fill)
{

}

Raster::Raster(void* data)
    : m_img(*static_cast<cimg_library::CImg<unsigned char>*>(data))
{
}

Raster::Raster(const std::string& filePath)
    : m_img(cimg_library::CImg<unsigned char>(filePath.c_str()))
{
}


int Raster::width() const
{
    return m_img.width();
}

void Raster::drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
{
    auto topLeftRounded = topLeft.round();
    auto bottomRightRounded = bottomRight.round();
    auto& img = m_img;
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    img.draw_rectangle(topLeftRounded.x(), 
                       topLeftRounded.y(), 
                       bottomRightRounded.x(), 
                       bottomRightRounded.y(), 
                       c, 
                       (float)color.a());
}

void Raster::drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor)
{
    // TODO: fix opacity/alpha stuff
    auto& img = m_img;
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

void Raster::drawRaster(int x, int y, const Raster& raster, double alpha)
{
    auto& img = m_img;
    auto& rasterImg = *static_cast<cimg_library::CImg<unsigned char>*>(raster.data());
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

void Raster::drawLine(const std::vector<Point>& points, const Color& color, double width)
{
    auto& img = m_img;

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

void Raster::drawPolygon(const std::vector<Point>& points, const Color& color)
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
    
    auto& img = m_img;
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    img.draw_polygon(pointsCImg, c, color.a()); // .display();
}

int Raster::height() const
{
    return m_img.height();
}

int BlueMarble::Raster::colorDepth() const
{
    return m_img.spectrum();
}

void Raster::resize(int width, int height, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    // interpolationType = interpolationType == 0 ? -1 : interpolationType; // -1 does not work as I expect
    m_img.resize(width, height, -100, -100, interpolationType);
}
void Raster::resize(float scaleRatio, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    // interpolationType = interpolationType == 0 ? -1 : interpolationType; // -1 does not work as I expect
    m_img.resize(width()*scaleRatio, height()*scaleRatio, -100, -100, interpolationType);
}
void Raster::rotate(double angle, int cx, int cy, ResizeInterpolation interpolation)
{
    int interpolationType = (int)interpolation;
    m_img.rotate(angle, cx, cy, interpolationType, 0);
}

void Raster::fill(int val)
{
    m_img.fill(val);
}

void Raster::blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian)
{
    m_img.blur(sigmaX, sigmaY, sigmaZ, isGaussian);
}

Raster Raster::getCrop(int x0, int y0, int x1, int y1)
{
    auto crop = m_img.get_crop(x0, y0, x1, y1);

    return Raster((void*)&crop);
}

void Raster::drawCircle(int x, int y, double radius, const Color& color)
{
    auto& img = m_img;
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    img.draw_circle(x, y, radius, c, color.a());
}

void* Raster::data() const
{
    return (void*)&m_img;
}

