#ifndef DRAWABLE
#define DRAWABLE

#include "Core.h"
#include "Raster.h"
#include "Utils.h"

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

            // inline Color(int r, int g, int b)
            //     : m_r(r)
            //     , m_g(g)
            //     , m_b(b)
            //     , m_alpha(1.0)
            // {}

            inline Color(int r=0, int g=0, int b=0, double alpha=1.0)
                : m_r(Utils::clampValue(r, 0, 255))
                , m_g(Utils::clampValue(g, 0, 255))
                , m_b(Utils::clampValue(b, 0, 255))
                , m_alpha(Utils::clampValue(alpha, 0, 1.0))
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
            
            // Properties
            int width() const;
            int height() const;
            const Color& backgroundColor();
            void backgroundColor(const Color& color);
            
            // Methods
            void resize(int width, int height);
            void fill(int val);
            void drawCircle(int x, int y, double radius, const Color& color);
            void drawLine(const std::vector<Point>& points, const Color& color, double width=1.0);
            void drawPolygon(const std::vector<Point>& points, const Color& color);
            void drawRect(const Rectangle& rect, const Color& color);
            void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
            void drawRaster(int x, int y, const Raster& raster, double alpha);
            void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent());
            Raster& getRaster();
        private:
            class Impl;   // Forward declaration
            Impl* m_impl; // Using "pimpl" design pattern for fun
    };

    // TODO: It could be useful to test performance where rendering is turned off.
    // Adding an IDrawable interface and implementing a DummyDrawable that does not 
    // draw anything could be a solution.
    // class DummyDrawable : public Drawable
    // {
    //     public:
    //         DummyDrawable(Drawable& actualDrawable)
    //             : Drawable(1, 1)
    //             , m_actualDrawable(actualDrawable)
    //         {

    //         }
    //         void resize(int width, int height)
    //         {
    //             m_actualDrawable.resize(width, height);
    //         }
    //         int width() const { return m_actualDrawable.width(); };
    //         int height() const  { return m_actualDrawable.height(); };
    //         void fill(int val)  { m_actualDrawable.fill(val); };
    //         void drawCircle(int x, int y, double radius, const Color& color) {}
    //         void drawLine(const std::vector<Point>& points, const Color& color, double width=1.0)  {}
    //         void drawPolygon(const std::vector<Point>& points, const Color& color)  {}
    //         void drawRect(const Rectangle& rect, const Color& color) {}
    //         void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color) {}
    //         void drawRaster(int x, int y, const Raster& raster, double alpha)
    //         {
    //             m_actualDrawable.drawRaster(x, y, raster, alpha);
    //         }
    //         void drawText(int x, int y, const std::string& text, const Color& color, int fontSize=20, const Color& backgroundColor=Color::transparent()) {}
    //         const Raster& getRaster() const { return m_actualDrawable.getRaster(); }

    //     private:
    //         Drawable& m_actualDrawable;
    // };
}

#endif /* DRAWABLE */
