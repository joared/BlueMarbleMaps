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

    class Geometry
    {
        public:
            virtual GeometryType type() = 0;
            virtual Rectangle bounds() = 0;
            virtual Point center() = 0;
            virtual bool isInside(const Rectangle& bounds) const = 0;
    };
    typedef std::shared_ptr<Geometry> GeometryPtr;


    class PointGeometry : public Geometry
    {
        public:
            PointGeometry();
            PointGeometry(const Point& point);

            GeometryType type() override final { return GeometryType::Point; };
            Rectangle bounds() override final { return Rectangle::undefined(); };
            Point center() override final { return m_point; };
            bool isInside(const Rectangle& bounds) const override final { return bounds.isInside(m_point); };

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

            GeometryType type() override final { return GeometryType::Line; };
            Rectangle bounds() override final { return Rectangle::fromPoints(m_points); };
            Point center() override final { return Utils::centroid(m_points); };
            bool isInside(const Rectangle& bounds) const override final { return bounds.isInside(m_points); }; // FIXME: points might not be inside

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
            PolygonGeometry(const std::vector<Point>& points);
            PolygonGeometry& operator=(const PolygonGeometry& other);

            GeometryType type() override final { return GeometryType::Polygon; };
            Rectangle bounds() override final { return Rectangle::fromPoints(m_points); };//{ return Rectangle::fromPoints(m_points); }; //{ return m_cachedBounds.get(); };
            Point center() override final { return Utils::centroid(m_points); };
            bool isInside(const Rectangle& bounds) const override final 
            { 
                if (Utils::pointInsidePolygon(bounds.center(), m_points))
                    return true;
                return bounds.isInside(m_points); 
            };

            std::vector<Point>& points() { return m_points; }
        private:
            std::vector<Point> m_points;
            Utils::CachableVariable<Rectangle> m_cachedBounds; // TODO: No obvious gain, remove?
    };
    typedef std::shared_ptr<PolygonGeometry> PolygonGeometryPtr;


    class MultiPolygonGeometry : public Geometry
    {
        public:
            MultiPolygonGeometry();
            MultiPolygonGeometry(const std::vector<PolygonGeometry>& polygons);

            GeometryType type() override final { return GeometryType::MultiPolygon; };
            Rectangle bounds() override final { return Rectangle(); };
            Point center() override final { return Point(); };
            bool isInside(const Rectangle& bounds) const override final { return false; }; // TODO

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

            GeometryType type() override final { return GeometryType::Raster; };
            Rectangle bounds() override final { return Rectangle(); };
            Point center() override final { return Point(); };
            bool isInside(const Rectangle& bounds) const override final { return false; }; // TODO
            
            const Point& offset() { return m_offset; }
            Raster& raster() { return m_raster; }
        private:
            Raster m_raster;
            Point  m_offset; // TODO: remove probably, temporary fix
    };
    typedef std::shared_ptr<RasterGeometry> RasterGeometryPtr;

}

#endif /* BLUEMARBLE_GEOMETRY */
