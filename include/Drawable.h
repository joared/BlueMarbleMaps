#ifndef DRAWABLE
#define DRAWABLE

#include "Core.h"
#include "Raster.h"

#include <string>
#include <vector>

namespace BlueMarble
{
    class Color
    {
        public:
            inline static Color transparent() { return Color(0, 0, 0, 0); }
            inline static Color red(double a=1.0) { return Color(255, 0, 0, a); }
            inline static Color green(double a=1.0) { return Color(0, 255, 0, a); }
            inline static Color blue(double a=1.0) { return Color(0, 0, 255, a); }
            inline static Color white(double a=1.0) { return Color(255, 255, 255, a); }
            inline static Color black(double a=1.0) { return Color(0, 0, 0, a); }

            inline Color(int r, int g, int b)
                : m_r(r)
                , m_g(g)
                , m_b(b)
                , m_alpha(1.0)
            {}

            inline Color(int r, int g, int b, double alpha)
                : m_r(r)
                , m_g(g)
                , m_b(b)
                , m_alpha(alpha)
            {}

            inline int r() const { return m_r; }
            inline int b() const { return m_b; }
            inline int g() const { return m_g; }
            inline double a() const { return m_alpha; }

            inline std::string toString() const 
            { 
                std::string str = "Color (";
                str += "r:" + std::to_string(r());
                str += ", g:" + std::to_string(g());
                str += ", b:" + std::to_string(b());
                str += ", alpha:" + std::to_string(a());
                str += ")";

                return str;
            }

        private:
            int m_r;
            int m_g;
            int m_b;
            double m_alpha;
    };

    
    class Drawable
    {
        public:
            Drawable(int width, int height);
            Drawable(const Drawable& drawable) = delete;
            void resize(int width, int height);
            int width() const;
            int height() const;
            void fill(int val);
            void drawCircle(int x, int y, double radius, const Color& color);
            void drawLine(const std::vector<Point>& points, const Color& color, double width=1.0);
            void drawPolygon(const std::vector<Point>& points, const Color& color);
            void drawRect(const Rectangle& rect, const Color& color);
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
            void drawRaster(int x, int y, const Raster& raster, double alpha);
            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent());
            const Raster& getRaster() const;
        private:
            class Impl; // Forward declaration
            Impl* m_impl; // Using "pimpl" design pattern for fun
    };
}

#endif /* DRAWABLE */
