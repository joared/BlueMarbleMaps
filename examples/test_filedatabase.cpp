#include "BlueMarbleMaps/Core/Index/FileDatabase.h"

using namespace BlueMarble;

void populateDatabase(IFeatureDataBase* db)
{
    std::vector<Point> points({ {16, 56}, {17, 57}, {15, 58} });
    
    std::vector<GeometryPtr> geometries;
    geometries.push_back(std::make_shared<PointGeometry>(points[0]));
    geometries.push_back(std::make_shared<LineGeometry>(points));
    geometries.push_back(std::make_shared<PolygonGeometry>(points));
    
    

    int featureId = 0;
    // for (auto g : geometries)
    // {
    //     featureId++;
    //     db->addFeature(std::make_shared<Feature>(
    //         Id(0, featureId),
    //         Crs::wgs84LngLat(),
    //         g,
    //         Attributes({{"some int", 1337+featureId}, 
    //                     {"some string", std::to_string(featureId)},
    //                     {"some double", 200.0+featureId},
    //                     {"a boolean", true},
    //                    })
    //     ));
    // }
}

int main() 
{
    auto database = FileDatabase();

    populateDatabase(&database);

    // database.save("testfile.database");
    // database.load("testfile.database");

    return 0;
}