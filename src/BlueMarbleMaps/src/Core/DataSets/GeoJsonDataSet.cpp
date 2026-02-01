#include "BlueMarbleMaps/Core/DataSets/GeoJsonDataSet.h"
#include "BlueMarbleMaps/Core/Serialization/GeoJsonSerializer.h"


using namespace BlueMarble;

GeoJsonFileDataSet::GeoJsonFileDataSet(const std::string &filePath)
    : AbstractFileDataSet(filePath)
{
}

FeatureCollectionPtr GeoJsonFileDataSet::read(const std::string &filePath)
{
    // Specification: https://geojson.org/geojson-spec.html
    auto file = File(filePath);
    if (!file.isOpen())
        BMM_DEBUG() << "GeoJsonFileDataSet::read() Failed to open file...\n";
    auto json = JsonValue::fromString(file.asString());

    BMM_DEBUG() << "Reading GeoJson file '" << filePath << "'\n";
    auto features = GeoJsonSerializer::deserialize(json);
    
    BMM_DEBUG() << "GeoJson file resulted in " << features->size() << " features.\n";

    return features;
}

void GeoJsonFileDataSet::save(const std::string &filePath) const
{
}
