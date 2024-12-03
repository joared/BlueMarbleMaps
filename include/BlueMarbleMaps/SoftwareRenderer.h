#ifndef BLUEMARBLE_SOFTWARERENDERER
#define BLUEMARBLE_SOFTWARERENDERER

#include "Renderer.h"

namespace BlueMarble
{
    class SoftwareRenderer : public Renderer
    {
        public:
            SoftwareRenderer();
            void drawCircle(int x, int y, double radius, const Color& color) override final;
            void drawLine(const std::vector<Point>& points, const Color& color, double width=1.0) override final;
            void drawPolygon(const std::vector<Point>& points, const Color& color) override final;
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color) override final;
            void drawRaster(int x, int y, const Raster& raster, double alpha) override final;
            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent()) override final;

            void setFrameBuffer(const unsigned char* data, int width, int height, int channels);
        private:
            const unsigned char* m_data;
            int m_width;
            int m_height;
            int m_channels;
    };
    typedef std::shared_ptr<SoftwareRenderer> SoftwareRendererPtr;
};

#endif /* BLUEMARBLE_SOFTWARERENDERER */
