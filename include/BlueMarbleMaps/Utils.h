#ifndef UTILS
#define UTILS

#include "Core.h"

#include <algorithm>
#include <cmath>
#include <stack>
#include <functional>

namespace BlueMarble
{
    namespace Utils
    {
        #define LONGITUDE_MIN -180
        #define LONGITUDE_MAX 180
        #define LATITUDE_MIN -90
        #define LATITUDE_MAX 90

        inline double clampValue(double val, double minVal, double maxVal) { return std::min(std::max(val, minVal), maxVal); }
        inline double normalizeValue(double val, double minVal, double maxVal)
        {
            // Some kind of generalization of: https://stackoverflow.com/questions/11498169/dealing-with-angle-wrap-in-c-code
            double valShifted = val - minVal;
            double maxShifted = maxVal - minVal;
            valShifted = std::fmod(valShifted, maxShifted);
            if (valShifted < 0)
            {
                valShifted += maxShifted;
            }
            return valShifted + minVal;
        }
        inline double normalizeLongitude(double lng) { return normalizeValue(lng, LONGITUDE_MIN, LONGITUDE_MAX); }
        inline double normalizeLatitude(double lat) { return normalizeValue(lat, LATITUDE_MIN, LATITUDE_MAX); }

        // Calculates the unit area of a polygon using the
        // sholace formula: https://en.wikipedia.org/wiki/Shoelace_formula
        inline double polygonArea(const std::vector<Point>& polygon)
        {
            assert(polygon.size() > 2);

            double area = 0;
            for (size_t i(0); i<polygon.size(); i++)
            {
                size_t nextIdx = (i < polygon.size()-1) ? i+1 : 0;
                auto& p1 = polygon[i];
                auto& p2 = polygon[nextIdx];
                area += p1.x()*p2.y() - p1.y()*p2.x(); // Determinant
            }

            return area * 0.5;
        }

        inline Point averageCenter(const std::vector<Point>& points)
        {
            assert(!points.empty());

            double sumX = 0, sumY = 0;
            for (auto& p : points) 
            {
                sumX += p.x();
                sumY += p.y();
            }
            return { sumX / points.size(), sumY / points.size() };
        }

        inline Point centroid(const std::vector<Point>& points)
        {
            assert(points.size() > 1);

            double cx = 0;
            double cy = 0;
            double area = 0;
            for (size_t i(0); i<points.size(); i++)
            {
                size_t nextIdx = (i < points.size()-1) ? i+1 : 0;
                auto& p1 = points[i];
                auto& p2 = points[nextIdx];
                double det = p1.x() * p2.y() - p1.y()*p2.x(); // determinant
                cx += (p1.x() + p2.x())*det;
                cy += (p1.y() + p2.y())*det;
                area += det;
            }

            area = area * 0.5;
            cx = 1.0/(6.0*area) * cx;
            cy = 1.0/(6.0*area) * cy;

            return Point(cx, cy);
        }

        // Calculates convex hull for a set of 2D points using
        // graham scan: https://www.geeksforgeeks.org/convex-hull-using-graham-scan/
        inline std::vector<Point> convexHull2D(const std::vector<Point>& points) 
        { 
            assert(points.size() > 2);
            
            // 1. Find smallest y
            int smallestYIdx;
            Point smallestYPoint = Point::undefined();
            int i = 0;
            for (auto& p : points)
            {
                if (smallestYPoint.isUndefined() || p.y() < smallestYPoint.y())
                {
                    smallestYIdx = i;
                    smallestYPoint = p;
                }
                i++;
            }

            // 2. Sort by polar angle
            auto comparePolarAngle = [&] (const Point& p1, const Point& p2)
            {
                double a1 = std::atan2(p1.y()-smallestYPoint.y(), p1.x()-smallestYPoint.x());
                double a2 = std::atan2(p2.y()-smallestYPoint.y(), p2.x()-smallestYPoint.x());

                return a1 < a2;
            };

            std::vector<Point> sorted = points;
            sorted.erase(sorted.begin()+smallestYIdx);
            std::sort(sorted.begin(), sorted.end(), comparePolarAngle);
            sorted.insert(sorted.begin(), smallestYPoint);
            
            auto orientation = [] (const Point& p, const Point& q, const Point& r)
            {
                double val = (q.y() - p.y()) * (r.x() - q.x()) -
                             (q.x() - p.x()) * (r.y() - q.y());

                if (val == 0) return 0;  // collinear
                return (val > 0)? 1: 2; // clock or counterclock wise
            };

            // If two or more points make same angle with p0,
            // Remove all but the one that is farthest from p0
            // Remember that, in above sorting, our criteria was
            // to keep the farthest point at the end when more than
            // one points have same angle.
            int m = 1; // Initialize size of modified array
            int n = sorted.size();
            for (int i=1; i<n; i++)
            {
                // Keep removing i while angle of i and i+1 is same
                // with respect to p0
                while (i < n-1 && orientation(smallestYPoint, sorted[i], sorted[i+1]) == 0)
                    i++;


                sorted[m] = sorted[i];
                m++;  // Update size of modified array
            }

            // If modified array of points has less than 3 points,
            // convex hull is not possible
            if (m < 3) return std::vector<Point>();

            std::stack<Point> S;
            S.push(sorted[0]);
            S.push(sorted[1]);
            S.push(sorted[2]);

            // A utility function to find next to top in a stack
            auto nextToTop = [] (std::stack<Point>& S)
            {
                Point p = S.top();
                S.pop();
                Point res = S.top();
                S.push(p);
                return res;
            };

            // Process remaining n-3 points
            for (int i = 3; i < m; i++)
            {
                // Keep removing top while the angle formed by
                // points next-to-top, top, and points[i] makes
                // a non-left turn
                while (S.size()>1 && orientation(nextToTop(S), S.top(), sorted[i]) != 2)
                    S.pop();
                S.push(sorted[i]);
            }

            std::vector<Point> convexHullPoints;
            while (!S.empty())
            {
                convexHullPoints.push_back(S.top());
                S.pop();
            }
                

            return convexHullPoints;
        }

        inline void movePoints(std::vector<Point>& points, const Point& delta)
        {
            for (auto& p: points)
            {
                p += delta;
            }
        }

        inline void movePointsTo(std::vector<Point>& points, const Point& point)
        {
            auto avgCenter = averageCenter(points);
            auto delta = point - avgCenter;
            movePoints(points, delta);
        }

        // Checking if a point is inside a polygon: https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
        inline bool pointInsidePolygon(const Point& point, const std::vector<Point>& polygon)
        {
            int num_vertices = polygon.size();
            double x = point.x(), y = point.y();
            bool inside = false;
        
            // Store the first point in the polygon and initialize
            // the second point
            Point p1 = polygon[0], p2;
        
            // Loop through each edge in the polygon
            for (int i = 1; i <= num_vertices; i++) {
                // Get the next point in the polygon
                p2 = polygon[i % num_vertices];
        
                // Check if the point is above the minimum y
                // coordinate of the edge
                if (y > std::min(p1.y(), p2.y())) {
                    // Check if the point is below the maximum y
                    // coordinate of the edge
                    if (y <= std::max(p1.y(), p2.y())) {
                        // Check if the point is to the left of the
                        // maximum x coordinate of the edge
                        if (x <= std::max(p1.x(), p2.x())) {
                            // Calculate the x-intersection of the
                            // line connecting the point to the edge
                            double x_intersection
                                = (y - p1.y()) * (p2.x() - p1.x())
                                    / (p2.y() - p1.y())
                                + p1.x();
        
                            // Check if the point is on the same
                            // line as the edge or to the left of
                            // the x-intersection
                            if (p1.x() == p2.x()
                                || x <= x_intersection) {
                                // Flip the inside flag
                                inside = !inside;
                            }
                        }
                    }
                }
        
                // Store the current point as the first point for
                // the next iteration
                p1 = p2;
            }
        
            // Return the value of the inside flag
            return inside;
        }

        inline double distanceToLine(const Point& point, const Point& start, const Point& end)
        {
            auto v = point-start;               // Vector from start of line to the point
            auto line = end-start;              // Vector representing the line
            auto lineV = line.norm();           // Unit vector representing the line direction
            double dotP = lineV.dotProduct(v);  // Amount of direction of v in direction of lineV

            Point nearestPoint;
            if (dotP < 0)
            {
                // Negative projection meaning the nearest point is the start point
                nearestPoint = start;
            }
            else if (dotP > line.length())
            {
                // Larger projection than the length of the line, meaning the nearest point is the end point
                nearestPoint = end;
            }
            else
            {
                // The nearest point is on the line
                nearestPoint = start + lineV*dotP;   // Nearest point on the line is the start + projection
            }
            
            return (point-nearestPoint).length();
        }


        inline std::vector<Point> scalePoints(const std::vector<Point>& input, double scale, Point fixPoint=Point::undefined())
        {
            std::vector<Point> scaledPoints;

            if (fixPoint.isUndefined())
            {
                fixPoint = centroid(input);
            }

            for (auto& p: input)
            {
                auto vec = p-fixPoint;
                auto newP = fixPoint + vec * scale;
                scaledPoints.push_back(newP);
            }

            // if (keepCentering)
            // {
            //     auto center = centroid(input);
            //     auto centerAfter = centroid(scaledPoints);
            //     auto delta = center - centerAfter;
            //     movePoints(output, delta);
            //     std::cout << "Centroid diff: " << delta.length() << "\n"; // Should be zero?
            // }

            return scaledPoints;
        }

        inline Point rotatePoint(const Point& p, double angle, const Point& fixPoint)
        {
            auto vec = p-fixPoint;

            auto rotated = Point(std::cos(angle)*vec.x() + -std::sin(angle)*vec.y(),  
                                 std::sin(angle)*vec.x() + std::cos(angle)*vec.y());

            return fixPoint + rotated;
        }

        inline std::vector<Point> rotatePoints(const std::vector<Point>& input, double angle, Point fixPoint=Point::undefined())
        {
            std::vector<Point> rotatedPoints;

            if (fixPoint.isUndefined())
            {
                fixPoint = centroid(input);
            }

            for (auto& p: input)
            {
                rotatedPoints.push_back(rotatePoint(p, angle, fixPoint));
            }

            return rotatedPoints;
        }

        // Function to calculate the normal of a vector (p2 - p1)
        inline Point calculateNormal(const Point& p1, const Point& p2) 
        {
            double dx = p2.x() - p1.x();
            double dy = p2.y() - p1.y();
            Point normal(dy, -dx);

            return normal.norm();
        }

        // Function to extend the polygon
        inline std::vector<Point> extendPolygon(const std::vector<Point>& polygon, double distance) 
        {
            std::vector<Point> extendedPolygon;
            int n = polygon.size();

            for (int i = 0; i < n; i++) {
                const Point& prev = polygon[(i - 1 + n) % n];
                const Point& curr = polygon[i];
                const Point& next = polygon[(i + 1) % n];

                // Calculate normals for adjacent edges
                Point normalPrev = calculateNormal(prev, curr);
                Point normalNext = calculateNormal(curr, next);

                // Average the normals
                Point averageNormal = { (normalPrev.x() + normalNext.x()) / 2, (normalPrev.y() + normalNext.y()) / 2 };
                
                // Normalize the average normal vector
                averageNormal = averageNormal.norm();

                // Move the current vertex along the average normal by the specified distance
                Point newPoint = { curr.x() + averageNormal.x() * distance, curr.y() + averageNormal.y() * distance };
                extendedPolygon.push_back(newPoint);
            }

            return extendedPolygon;
        }

        // Some sort of custom implementation of vertex clustering
        // FIXME: this does not seem to work: see Värmland/Gävleborg/Norrbotten from csv file
        inline void simplifyPoints(const std::vector<Point>& input, std::vector<Point>& output, double gridCellSize)
        {   
            if (input.size() < 3)
            {
                output = input;
                return;
            }
            assert(output.size() == 0);

            // output = input;
            // return;

            std::vector<Point> pointsToMerge;
            Point pivot = input[0];
            output.push_back(pivot);

            for (size_t i(0); i < input.size(); i++)
            {
                if (pivot.distanceTo(input[i]) < gridCellSize)
                {
                    pointsToMerge.push_back(input[i]);
                }
                else
                {
                    if (!pointsToMerge.empty())
                    {
                        output.push_back(pointsToMerge[pointsToMerge.size()-1]);
                        pivot = input[i];
                        pointsToMerge.clear();
                        pointsToMerge.push_back(pivot);
                    }
                    else
                    {
                        pivot = input[i];
                        output.push_back(pivot);
                    }
                    
                }
            }
            if (!pointsToMerge.empty())
            {
                output.push_back(pointsToMerge[pointsToMerge.size()-1]);
            }

            // while (inputCopy.size() > 1)
            // {
            //     std::vector<Point> gridCellPoints;
            //     Point pivotPoint = inputCopy[0];
            //     Rectangle gridCell(pivotPoint.x()-0.5, pivotPoint.y()-0.5, pivotPoint.x()+0.5, pivotPoint.y()+0.5);
            //     gridCell.scale(gridCellSize);
            //     for (auto& p : inputCopy)
            //     {
            //         if (gridCell.isInside(p))
            //         {
            //             gridCellPoints.push_back(p);
            //         }
            //     }
            //     if (gridCellPoints.size() > 1)
            //     {
            //         output.push_back(centroid(gridCellPoints));
            //     }
            //     else
            //     {
            //         output.push_back(gridCellPoints[0]);
            //     }

            //     // Remove the used points
            //     for (size_t i(0); i<gridCellPoints.size(); i++)
            //     {
            //         inputCopy.erase(inputCopy.begin());
            //     }
            // }
        }

        inline std::vector<std::string> splitString(std::string s, const std::string& delimiter) 
        {
            std::vector<std::string> tokens;
            size_t pos = 0;
            std::string token;
            while ((pos = s.find(delimiter)) != std::string::npos) 
            {
                token = s.substr(0, pos);
                tokens.push_back(token);
                s.erase(0, pos + delimiter.length());
            }
            tokens.push_back(s);

            return tokens;
        }

        template <typename T>
        class CachableVariable
        {
            public:
                inline CachableVariable(const std::function<T()>& func) : m_hasValue(false), m_val(), m_calcFunction(func) {}
                inline CachableVariable(const CachableVariable& other) = delete;
                inline const T& get() 
                { 
                    if (!m_hasValue)
                    {
                        m_val = m_calcFunction();
                        m_hasValue = true;
                    }
                        
                    return m_val; 
                }
                inline void clear() { m_hasValue=false; }
                // template<typename T>
                // operator T() const 
                // {
                //     return get<T>();
                // }
            private:
                bool m_hasValue;
                T m_val;
                std::function<T()> m_calcFunction;
        };
    }
}

#endif /* UTILS */
