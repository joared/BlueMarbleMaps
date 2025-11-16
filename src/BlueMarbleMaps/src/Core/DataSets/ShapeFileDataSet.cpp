#include "BlueMarbleMaps/Core/DataSets/ShapeFileDataSet.h"

using namespace BlueMarble;

ShapeFileDataSet::ShapeFileDataSet(const std::string& filePath)
    : AbstractFileDataSet(filePath)
{

}

FeatureCollectionPtr ShapeFileDataSet::read(const std::string& /*filePath*/)
{
    auto features = std::make_shared<FeatureCollection>();
    // 59.334591, 18.063240
    auto stockholm = std::make_shared<Feature>
    (
        Id(0, 1), 
        crs(),
        std::make_shared<PointGeometry>(Point(18.063240, 59.334591))
    );
    stockholm->attributes().set("NAME", "Stockholm");

    // 59.334591, 18.063240
    auto goteborg = std::make_shared<Feature>
    (
        Id(0, 2), 
        crs(),
        std::make_shared<PointGeometry>(Point(11.954, 57.706))
    );
    goteborg->attributes().set("NAME", "Goteborg");

    features->add(stockholm);
    features->add(goteborg);

    return features;
}