#include "BlueMarbleMaps/Core/DataSets/CsvDataSet.h"
#include "BlueMarbleMaps/System/CSVFile.h"

using namespace BlueMarble;

CsvFileDataSet::CsvFileDataSet(const std::string& filePath)
    : AbstractFileDataSet(filePath)
{

}


FeatureCollectionPtr CsvFileDataSet::read(const std::string& filePath)
{
    auto features = std::make_shared<FeatureCollection>();
    auto csv = CSVFile(filePath, ",");
    
    // First line has the name of each column/attribute
    auto attrNames = csv.rows()[0];
    int lngIdx = std::find(attrNames.begin(), attrNames.end(), "Longitude") - attrNames.begin();
    int latIdx = std::find(attrNames.begin(), attrNames.end(), "Latitude") - attrNames.begin();
    
    int i = 0;
    for (auto& tokens : csv.rows())
    {
        if (i == 0)
        {
            i++;
            continue; // ignore first row   
        }
        
        // First extract geometry
        double lng = std::stod(tokens[lngIdx]);
        double lat = std::stod(tokens[latIdx]);
        PointGeometryPtr geometry = std::make_shared<PointGeometry>(Point(lng, lat));
        FeaturePtr feature = createFeature(geometry);

        // Then set attributes
        int j = 0;
        for (auto& attrStr : tokens)
        {
            if (j == lngIdx || j == latIdx)
            {
                j++;
                continue; // Ignore lnglat, already handled
            }
                
            // TODO: convert to int/double if possible
            feature->attributes().set(attrNames[j], attrStr);
            j++;
        }

        features->add(feature);

        i++;
    }


    // All below is temporary
    // Testing generating counties with convex hull

    // Set NAME attribute from "Locality"
    for (const auto& f : *features)
    {
        f->attributes().set("NAME", f->attributes().get<std::string>("Locality"));
    }

    // // Create polygon features for cities within same county
    // std::set<std::string> uniqueCounties;
    // std::map<std::string, std::vector<Point>> countyPoints;
    // for (auto f : m_features)
    // {
    //     std::string countyName = f->attributes().get<std::string>("County");
    //     uniqueCounties.insert(countyName);
    //     countyPoints[countyName].push_back(f->geometryAsPoint()->point());
    // }

    // for (auto& county : countyPoints)
    // {
    //     auto name = county.first;
    //     auto& points = county.second;
    //     if (points.size() < 3)
    //         continue;
        
    //     points = Utils::convexHull2D(points);
    //     if (points.size() < 3)
    //     {
    //         std::cout << "Not enough points!!! " << std::to_string(points.size()) << "\n";
    //         continue;
    //     }   

    //     auto countyPolygon = createFeature(std::make_shared<PolygonGeometry>(points));
    //     countyPolygon->attributes().set("NAME", name);
    //     m_features.insert(m_features.begin(), countyPolygon);
    // }
    return nullptr;
}