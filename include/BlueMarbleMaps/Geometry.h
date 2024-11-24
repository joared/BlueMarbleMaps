#ifndef BLUEMARBLE_GEOMETRY
#define BLUEMARBLE_GEOMETRY

#include "Core.h"
#include "Raster.h"
#include "Utils.h"

#include <memory>
#include <vector>

namespace BlueMarble
{

    enum class GeometryType
    {
        Point,
        Line,
        Polygon,
        MultiPolygon,
        Raster
    };

    inline std::string typeToString(GeometryType type)
    {
        switch (type)
        {
        case GeometryType::Point:
            return "Point";
        case GeometryType::Line:
            return "Line";
        case GeometryType::Polygon:
            return "Polygon";
        case GeometryType::MultiPolygon:
            return "MultiPolygon";
        case GeometryType::Raster:
            return "Raster";
        default:
            std::cout << "typeToString(GeometryType) Unhandled GeometryType: " << (int)type << "\n";
            return "Unknown geometry type: " + std::to_string((int)type);
        }
    }

    class Geometry; // Forward declaration
    typedef std::shared_ptr<Geometry> GeometryPtr;
    class Geometry
    {
        public:
            virtual GeometryPtr clone() = 0;
            virtual GeometryType type() = 0;
            virtual Rectangle bounds() = 0;
            virtual Point center() = 0;
            virtual void move(const Point& delta) = 0;
            virtual void moveTo(const Point& point) = 0;
            virtual bool isInside(const Rectangle& bounds) const = 0;
            virtual bool isStrictlyInside(const Rectangle& bounds) const = 0;
        protected:
            virtual ~Geometry() = default; // { std::cout << "~Geometry()\n"; };
    };


    class PointGeometry : public Geometry
    {
        public:
            PointGeometry();
            PointGeometry(const Point& point);
            GeometryPtr clone() override final { return std::make_shared<PointGeometry>(*this); };
            GeometryType type() override final { return GeometryType::Point; };
            Rectangle bounds() override final { return Rectangle::undefined(); };
            Point center() override final { return m_point; };
            void move(const Point& delta) override final { m_point += delta; };
            void moveTo(const Point& point) override final { m_point = point; };
            bool isInside(const Rectangle& bounds) const override final { return bounds.isInside(m_point); };
            bool isStrictlyInside(const Rectangle& bounds) const override final { return isInside(bounds); };

            Point& point() { return m_point; }

        private:
            Point m_point;
    };
    typedef std::shared_ptr<PointGeometry> PointGeometryPtr;


    class LineGeometry : public Geometry
    {
        public:
            LineGeometry();
            LineGeometry(const std::vector<Point>& points);
            GeometryPtr clone() override final { return std::make_shared<LineGeometry>(*this); };
            GeometryType type() override final { return GeometryType::Line; };
            Rectangle bounds() override final { return Rectangle::fromPoints(m_points); };
            Point center() override final { return m_points[m_points.size()%2]; }; // FIXME
            void move(const Point& delta) override final;
            void moveTo(const Point& point) override final;
            bool isInside(const Rectangle& bounds) const override final { return bounds.isInside(m_points); }; // FIXME: points might not be inside
            bool isStrictlyInside(const Rectangle& bounds) const override final { return bounds.allInside(m_points); };

            std::vector<Point>& points() { return m_points; }
        private:
            std::vector<Point> m_points;
    };
    typedef std::shared_ptr<LineGeometry> LineGeometryPtr;


    class PolygonGeometry : public Geometry
    {
        public:
            PolygonGeometry();
            PolygonGeometry(const PolygonGeometry& other);
            PolygonGeometry(const std::vector<Point>& outerRing);
            PolygonGeometry(const std::vector<std::vector<Point>>& rings);
            PolygonGeometry& operator=(const PolygonGeometry& other);

            GeometryPtr clone() override final { return std::make_shared<PolygonGeometry>(*this); };
            GeometryType type() override final { return GeometryType::Polygon; };
            Rectangle bounds() override final { return Rectangle::fromPoints(outerRing()); }; //{ return m_cachedBounds.get(); };
            Point center() override final { return Utils::centroid(outerRing()); };
            void move(const Point& delta) override final;
            void moveTo(const Point& point) override final;
            bool isInside(const Rectangle& bounds) const override final 
            { 
                // TODO: if the bounds overlap with a polygon but in between nodes, this doesnt work
                
                // If any point on the outer ring is inside of the bounds
                // this Polygon is inside the bounds
                if (bounds.isInside(outerRing()))
                    return true;

                // If the center of the bounds is not within the outer ring,
                // this Polygon is can be inside the bounds since the above 
                // was false.
                if (!Utils::pointInsidePolygon(bounds.center(), outerRing()))
                    return false;
                
                // Check whether the bounds are completely inside any of the inner rings.
                // If it is, this Polygon is not inside the bounds
                for (size_t i=1; i<m_rings.size(); i++)
                {
                    if (Utils::pointInsidePolygon(bounds.center(), m_rings[i])
                        && !bounds.isInside(m_rings[i]))
                        return false;
                }

                return true;
            };
            bool isStrictlyInside(const Rectangle& bounds) const override final { return bounds.allInside(outerRing()); };

            std::vector<Point>& outerRing() { return m_rings[0]; }
            const std::vector<Point>& outerRing() const { assert(m_rings.size() > 0); return m_rings[0]; }
            std::vector<std::vector<Point>>& rings() { return m_rings; };
        private:
            std::vector<std::vector<Point>>     m_rings; // All rings, including outer
            Utils::CachableVariable<Rectangle>  m_cachedBounds; // TODO: No obvious gain, remove?
    };
    typedef std::shared_ptr<PolygonGeometry> PolygonGeometryPtr;


    class MultiPolygonGeometry : public Geometry
    {
        public:
            MultiPolygonGeometry();
            MultiPolygonGeometry(const std::vector<PolygonGeometry>& polygons);

            GeometryPtr clone() override final { return std::make_shared<MultiPolygonGeometry>(*this); };
            GeometryType type() override final { return GeometryType::MultiPolygon; };
            Rectangle bounds() override final { return Rectangle(); };
            Point center() override final { return Point(); };
            void move(const Point& delta) override final;
            void moveTo(const Point& point) override final;
            bool isInside(const Rectangle& bounds) const override final 
            {  
                for (auto& p : m_polygons)
                {
                    if(p.isInside(bounds))
                        return true;
                }
                return false; 
            }; // TODO
            bool isStrictlyInside(const Rectangle& bounds) const override final 
            { 
                for (auto& p : m_polygons)
                {
                    if(!p.isStrictlyInside(bounds))
                        return false;
                }
                return true; 
            }; // TODO

            std::vector<PolygonGeometry>& polygons() { return m_polygons; }
        private:
            std::vector<PolygonGeometry> m_polygons;
    };
    typedef std::shared_ptr<MultiPolygonGeometry> MultiPolygonGeometryPtr;


    class RasterGeometry : public Geometry
    {
        public:
            RasterGeometry();
            RasterGeometry(const Raster& raster, const Point& offset = Point(0,0));
            GeometryPtr clone() override final { return std::make_shared<RasterGeometry>(*this); };
            GeometryType type() override final { return GeometryType::Raster; };
            Rectangle bounds() override final { return Rectangle(); };
            Point center() override final { return Point(); };
            void move(const Point& delta) override final { m_offset += delta; }; // Not tested
            void moveTo(const Point& point) override final { m_offset = point; }; // Not tested
            bool isInside(const Rectangle& /*bounds*/) const override final { return false; }; // TODO
            bool isStrictlyInside(const Rectangle& /*bounds*/) const override final { return false; }; // TODO

            const Point& offset() { return m_offset; }
            Raster& raster() { return m_raster; }
        private:
            Raster m_raster;
            Point  m_offset; // TODO: remove probably, temporary fix
    };
    typedef std::shared_ptr<RasterGeometry> RasterGeometryPtr;


    void convertGeometry(GeometryPtr from, GeometryType toType);

}

#endif /* BLUEMARBLE_GEOMETRY */