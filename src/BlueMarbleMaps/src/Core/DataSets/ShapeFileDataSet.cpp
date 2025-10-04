#include "BlueMarbleMaps/Core/DataSets/ShapeFileDataSet.h"

using namespace BlueMarble;

ShapeFileDataSet::ShapeFileDataSet(const std::string& filePath)
    : AbstractFileDataSet(filePath)
{

}

void ShapeFileDataSet::read(const std::string& /*filePath*/)
{
        // 59.334591, 18.063240
    auto stockholm = std::make_shared<Feature>
    (
        Id(0, 1), 
        getCrs(),
        std::make_shared<PointGeometry>(Point(18.063240, 59.334591))
    );
    stockholm->attributes().set("NAME", "Stockholm");

    // 59.334591, 18.063240
    auto goteborg = std::make_shared<Feature>
    (
        Id(0, 2), 
        getCrs(),
        std::make_shared<PointGeometry>(Point(11.954, 57.706))
    );
    goteborg->attributes().set("NAME", "Goteborg");

    m_features.push_back(stockholm);
    m_features.push_back(goteborg);
}