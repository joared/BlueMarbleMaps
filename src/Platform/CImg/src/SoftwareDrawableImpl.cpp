#include "SoftwareDrawableImpl.h"

#ifndef BLUEMARBLE_USE_CIMG_RASTER_IMPL
// If opengl compatible raster implementation is used,
// we need ImageDataOperations to convert image data in e.g.
// drawRaster.
#include "BlueMarbleMaps/Utility/ImageDataOperations.h"
#define OPENGL_TO_CIMG interleavedToPlanarFlipY
#endif
 

using namespace BlueMarble;

SoftwareDrawable::Impl::Impl(int width, int height, int channels)
    : m_img(width, height, 1, channels, 0)
    , m_backGroundColor(Color::blue(0.0))
    , m_disp(nullptr)
    , m_transform()
{
}

const Transform& SoftwareDrawable::Impl::getTransform()
{
    return m_transform;
}

void SoftwareDrawable::Impl::setTransform(const Transform &transform)
{
    m_transform = transform;
}

void SoftwareDrawable::Impl::resize(int width, int height)
{
    m_img.resize(width, height);
}

int SoftwareDrawable::Impl::width() const
{
    return m_img.width();
}

int SoftwareDrawable::Impl::height() const
{
    return m_img.height();
}

const Color& SoftwareDrawable::Impl::backgroundColor() const
{
    return m_backGroundColor;
}

void SoftwareDrawable::Impl::backgroundColor(const Color& color)
{
    m_backGroundColor = color;
}

void SoftwareDrawable::Impl::fill(int val)
{
    m_img.fill(val);
}

void SoftwareDrawable::Impl::drawCircle(int x, int y, double radius, const Color& color)
{
    // m_renderer->drawCircle(x, y,radius,color);
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    m_img.draw_circle(x, y, radius, c, color.a());
}

void SoftwareDrawable::Impl::drawLine(const LineGeometryPtr& geometry, const Color& color, double width)
{
    // m_renderer->drawLine(points, color, width);
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    int size = geometry->points().size();
    auto line = geometry->points();
    for (int i(0); i < size-1; ++i)
    {
        auto& p1 = line[i];
        auto& p2 = line[i+1];
        int x0 = std::round(p1.x());
        int y0 = std::round(p1.y());
        int x1 = std::round(p2.x());
        int y1 = std::round(p2.y());
        if (width <= 1.0)
            m_img.draw_line(x0, y0, x1, y1, c, color.a());
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
            PolygonGeometryPtr polygonPtr = std::make_shared<PolygonGeometry>(PolygonGeometry(polygon));
            drawPolygon(polygonPtr, color);
        }
    }
}

void SoftwareDrawable::Impl::drawPolygon(const PolygonGeometryPtr& geometry, const Color& color)
{
    // m_renderer->drawPolygon(points,color);
    assert(geometry->outerRing().size() > 2);
    auto iterator = geometry->outerRing().begin();
    cimg_library::CImg<int> pointsCImg(geometry->outerRing().size(), 2);
    cimg_forX(pointsCImg,i) 
    { 
        auto& p = *(iterator++);
        pointsCImg(i,0) = p.x(); 
        pointsCImg(i,1) = p.y(); 
    }
    
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    m_img.draw_polygon(pointsCImg, c, color.a()); // .display();
}

void SoftwareDrawable::Impl::drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
{
    // m_renderer->drawRect(topLeft, bottomRight, color);
    auto topLeftRounded = topLeft.round();
    auto bottomRightRounded = bottomRight.round();
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    m_img.draw_rectangle(topLeftRounded.x(), 
                            topLeftRounded.y(), 
                            bottomRightRounded.x(), 
                            bottomRightRounded.y(), 
                            c, 
                            (float)color.a());
}

void SoftwareDrawable::Impl::drawRaster(const RasterGeometryPtr& geometry, double alpha)
{
    auto center = m_transform.translation();
    double scale = m_transform.scale();
    double rotation = m_transform.rotation();

    // TODO: This is the entire screen and might not be the update area that the view provided during the update.
    // This will result in raster features that have info outside the update area provided by the view will be visible
    Rectangle updateArea
    (
        center.x() - (width() / scale)*0.5,
        center.y() - (height() / scale)*0.5,
        center.x() + (width() / scale)*0.5,
        center.y() + (height() / scale)*0.5
    );

    auto subGeometry = geometry->getSubRasterGeometry(updateArea);

    Raster& subRaster = subGeometry->raster();

    int screenWidth = subGeometry->bounds().width()*scale;
    int screenHeight = subGeometry->bounds().height()*scale;

    if (screenWidth == 0 || screenHeight == 0)
    {
        std::cout << "SoftwareDrawable::Impl::drawRaster - Raster resize dimensions are too small, ignoring draw (" << screenWidth << ", " << screenHeight << ")\n";
        return;
    }

    auto minCorner = subGeometry->bounds().minCorner();

    auto screenC = screenCenter();
    auto delta = minCorner - center;
    int x = delta.x()*scale + screenC.x();
    int y = delta.y()*scale + screenC.y();
    
    subRaster.resize(screenWidth, screenHeight, Raster::ResizeInterpolation::NearestNeighbor);

#ifdef BLUEMARBLE_USE_CIMG_RASTER_IMPL
    // CImg raster implementation
    auto rasterImg = cimg_library::CImg<unsigned char>(raster.data(), raster.width(), raster.height(), 1, raster.channels(), true);
#else
    // stb_image Raster implementation, need to convert
    auto rasterImg = cimg_library::CImg<unsigned char>(subRaster.data(), subRaster.width(), subRaster.height(), 1, subRaster.channels());
    OPENGL_TO_CIMG(rasterImg.data(), subRaster.data(), subRaster.width(), subRaster.height(), subRaster.channels());
#endif

    if (rasterImg.spectrum() == 4)
    {
        //img.draw_image(x, y, rasterImg.get_shared_channels(0,2), rasterImg.get_shared_channel(3), 1.0, 255);
        //img.draw_image(x, y, rasterImg, rasterImg.get_shared_channel(3), 1.0, 255);
        m_img.draw_image(x, y, rasterImg, rasterImg.get_shared_channel(3), alpha, 255);
    }
    else
    {
        m_img.draw_image(x, y, 0, 0, rasterImg, alpha);
    }
}

void SoftwareDrawable::Impl::drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& bcolor)
{
    // m_renderer->drawText(x, y, text, color, fontSize, bcolor);
    // TODO: fix opacity/alpha stuff
    unsigned char c[] = {color.r(), color.g(), color.b(), (unsigned char)(color.a()*255)};
    if (bcolor.a() > 0)
    {
        // With background color
        unsigned char backc[] = {bcolor.r(), bcolor.g(), bcolor.b(), (unsigned char)(bcolor.a()*255)};
        m_img.draw_text(x, y, text.c_str(), backc, backc, bcolor.a(), fontSize);
        m_img.draw_text(x, y, text.c_str(), c, 0, color.a(), fontSize);
    }
    else
    {
        // Without background color
        m_img.draw_text(x, y, text.c_str(), c, 0, color.a(), fontSize);
    }
}

void SoftwareDrawable::Impl::swapBuffers()
{
    // Update the image and save new black draw image
    auto drawImg = cimg_library::CImg<unsigned char>(m_img.data(), m_img.width(), m_img.height(), 1, m_img.spectrum(), true);

    if (m_transform.rotation() != 0)
    {
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
        return;
    }

    m_disp->display(drawImg);
}

void SoftwareDrawable::Impl::setWindow(void* window)
{
    #ifdef BLUEMARBLE_USE_CIMG_WINDOW
    m_disp = static_cast<cimg_library::CImgDisplay*>(window);
    #else
    // GLFW
    m_disp = static_cast<cimg_library::CImgDisplay*>(window);
    #endif
}

RendererImplementation SoftwareDrawable::Impl::renderer()
{
    return RendererImplementation::Software;
}

Color SoftwareDrawable::Impl::readPixel(int x, int y)
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

void SoftwareDrawable::Impl::setPixel(int x, int y, const Color& color)
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

Point SoftwareDrawable::Impl::screenCenter()
{
    return Point((width()-1)*0.5, (height()-1)*0.5);
}
