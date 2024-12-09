#ifndef BLUEMARBLE_CORE
#define BLUEMARBLE_CORE

#include <chrono>
#include <cmath>
#include <vector>
#include <cassert>
#include <limits>
#include <string>


namespace BlueMarble
{
    #define BIT(x) (1 << x)

    class Point
    {
        public:
            inline static Point undefined() { Point p; p.m_isUndefined = true; return p; }
            inline Point(double x=0, double y=0) : m_x(x), m_y(y), m_isUndefined(false) {}
            inline bool isUndefined() const { return m_isUndefined; }
            inline  double x() const { return m_x; }
            inline  double y() const { return m_y; }

            inline Point round() const
            {
                return Point
                (
                    std::round(m_x),
                    std::round(m_y)
                );
            }


            inline double length()
            {
                return std::sqrt(m_x*m_x + m_y*m_y);
            }

            inline double distanceTo(const Point& other)
            {
                return (*this-other).length();
            }

            inline Point norm()
            {
                double l = length();
                return Point(m_x / l, m_y / l);
            }

            inline double dotProduct(const Point& point)
            {
                return m_x*point.x() + m_y*point.y();
            }

            inline Point project(const Point& point)
            {
                auto n = norm();
                return n*n.dotProduct(point);
            }


            inline Point operator+(const Point& other) const
            {
                return Point(m_x + other.x(), m_y+other.y());
            }

            inline void operator+=(const Point& other)
            {
                m_x += other.x();
                m_y += other.y();
            }

            inline Point operator-(const Point& other) const
            {
                return Point(m_x - other.x(), m_y-other.y());
            }

            inline Point operator*(const double& other) const
            {
                return Point(m_x*other, m_y*other);
            }

            inline std::string toString()
            {
                return "Point(" + std::to_string(m_x) + ", " + std::to_string(m_y);
            }

        private:
            double  m_x;
            double  m_y;
            bool    m_isUndefined;
    };

    // Uggly forward declarations
    namespace Utils
    {
        std::vector<Point> rotatePoints(const std::vector<Point>& input, double angle, Point fixPoint);
    }

    class Rectangle
    {
        public:
            static Rectangle undefined()
            {
                auto rect = Rectangle(0, 0, 0, 0);
                rect.m_isUndefined = true;
                return rect;
            }

            static Rectangle infinite()
            {
                double inf = std::numeric_limits<double>::infinity();
                return Rectangle(-inf, -inf, inf, inf);
            }


            static Rectangle fromPoints(const std::vector<Point>& points)
            {
                assert(points.size() > 1); // FIXME: valid even if only 1 point?
                
                auto inf = std::numeric_limits<double>::infinity();
                double xMin = inf; 
                double yMin = inf; 
                double xMax = -inf; 
                double yMax = -inf;
                for (auto& p : points)
                {
                    xMin = std::min(xMin, p.x());
                    yMin = std::min(yMin, p.y());
                    xMax = std::max(xMax, p.x());
                    yMax = std::max(yMax, p.y());
                }

                return Rectangle(xMin, yMin, xMax, yMax);
            }

            static Rectangle mergeBounds(const std::vector<Rectangle>& boundsList)
            {
                if (boundsList.size() == 0)
                {
                    return undefined();
                }
                
                auto inf = std::numeric_limits<double>::infinity();
                double xMin = inf; 
                double yMin = inf; 
                double xMax = -inf; 
                double yMax = -inf;
                for (auto& b : boundsList)
                {
                    if (b.isUndefined())
                        continue;
                    xMin = std::min(xMin, b.xMin());
                    yMin = std::min(yMin, b.yMin());
                    xMax = std::max(xMax, b.xMax());
                    yMax = std::max(yMax, b.yMax());
                }

                return Rectangle(xMin, yMin, xMax, yMax);
            }

            inline Rectangle()
                : Rectangle(undefined())
            {
            }

            inline Rectangle(double xMin, double yMin, double xMax, double yMax) 
                : m_xMin(std::min(xMin, xMax))
                , m_yMin(std::min(yMin, yMax))
                , m_xMax(std::max(xMin, xMax))
                , m_yMax(std::max(yMin, yMax))
                , m_isUndefined(false)
            {}

            inline Rectangle(const Point& center, double width, double height) 
                : m_xMin(center.x() - width*0.5)
                , m_yMin(center.y() - height*0.5)
                , m_xMax(center.x() + width*0.5)
                , m_yMax(center.y() + height*0.5)
                , m_isUndefined(false)
            {}

            inline bool isUndefined() const { return m_isUndefined; }

            inline bool isInside(const Point& point) const
            {
                return m_xMin <= point.x() && m_yMin <= point.y() && m_xMax >= point.x() && m_yMax >= point.y();
            }

            // TODO: rename to anyInside/anyPointsInside?
            inline bool isInside(const std::vector<Point>& points) const
            {
                for (auto& p : points)
                {
                    if(isInside(p))
                        return true;
                }

                return false;
            }


            // Checks whether other rectangle is inside this
            inline bool isInside(const Rectangle& other) const
            {
                return m_xMin <= other.xMin() && m_yMin <= other.yMin() && m_xMax >= other.xMax() && m_yMax >= other.yMax();
            }


            inline bool allInside(const std::vector<Point>& points) const
            {
                for (auto& p : points)
                {
                    if(!isInside(p))
                        return false;
                }

                return true;
            }

            // Checks whether the rectangles overlap: https://www.geeksforgeeks.org/find-two-rectangles-overlap/
            inline bool overlap(const Rectangle& other) const
            {
                auto l1 = Point(m_xMin, m_yMax);
                auto r1 = Point(m_xMax, m_yMin);
                auto l2 = Point(other.m_xMin, other.m_yMax);
                auto r2 = Point(other.m_xMax, other.m_yMin);

                if (l1.x() > r2.x() || l2.x() > r1.x())
                    return false;

                // If one rectangle is above the other
                if (r1.y() > l2.y() || r2.y() > l1.y())
                    return false;

                return true;
            }

            inline void offset(double offsetX, double offsetY)
            {
                m_xMin += offsetX;
                m_xMax += offsetX;
                m_yMin += offsetY;
                m_yMax += offsetY;
            }

            inline void extend(double x, double y)
            {
                m_xMin -= x;
                m_xMax += x;
                m_yMin -= y;
                m_yMax += y;
            }

            inline void scale(double scale)
            {
                auto c = center();
                double newWidth = width()*scale;
                double newHeight = height()*scale;
                m_xMin = c.x() - newWidth*0.5;
                m_xMax = c.x() + newWidth*0.5;
                m_yMin = c.y() - newHeight*0.5;
                m_yMax = c.y() + newHeight*0.5;
            }

            inline Rectangle rotate(double angle)
            {
                auto points = corners();
                auto rotated = Utils::rotatePoints(points, angle, center());
                return fromPoints(rotated);
            }

            inline Point minCorner() const
            {
                return Point(xMin(), yMin());
            }

            inline Point maxCorner() const
            {
                return Point(xMax(), yMax());
            }

            inline std::vector<Point> corners(bool clockWise=false) const
            {
                auto topLeft = BlueMarble::Point(xMin(), yMin());
                auto bottomRight = BlueMarble::Point(xMax(), yMax());

                std::vector<BlueMarble::Point> points;
                if (!clockWise)
                {
                    points.push_back(topLeft);
                    points.push_back(BlueMarble::Point(topLeft.x(), bottomRight.y()));
                    points.push_back(bottomRight);
                    points.push_back(BlueMarble::Point(bottomRight.x(), topLeft.y()));
                }
                else
                {
                    points.push_back(BlueMarble::Point(bottomRight.x(), topLeft.y()));
                    points.push_back(bottomRight);
                    points.push_back(BlueMarble::Point(topLeft.x(), bottomRight.y()));
                    points.push_back(topLeft);
                }
                
                return points;
            }

            inline double xMin() const { return m_xMin; }
            inline double yMin() const { return m_yMin; }
            inline double xMax() const { return m_xMax; }
            inline double yMax() const { return m_yMax; }
            inline Point center() const { return Point(xMin() + width()*0.5, yMin() + height()*0.5); }
            inline void reCenter(const Point& center) 
            { 
                double w = width();
                double h = height();
                m_xMin = center.x() - w*0.5;
                m_xMax = center.x() + w*0.5;
                m_yMin = center.y() - h*0.5;
                m_yMax = center.y() + h*0.5;
            }
            inline double width() const { return m_xMax-m_xMin; }
            inline double height() const { return m_yMax-m_yMin; }

            inline std::string toString()
            {
                return "Rectangle(" + std::to_string(m_xMin) + ", " + std::to_string(m_yMin) + ", " + std::to_string(m_xMax) + ", " + std::to_string(m_yMax) + ")";
            }

        private:
            double m_xMin;
            double m_yMin;
            double m_xMax;
            double m_yMax;

            bool m_isUndefined;
    };

    inline int64_t getTimeStampMs()
    {
        auto now = std::chrono::high_resolution_clock::now();

        // Convert the time point to a duration in milliseconds
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

        // Get the number of milliseconds since epoch
        auto epoch = now_ms.time_since_epoch();
        int64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();

        return milliseconds;
    }
} // namespace BlueMarble


#endif /* BLUEMARBLE_CORE */

