#ifndef BLUEMARBLE_JSONSERIALIZER
#define BLUEMARBLE_JSONSERIALIZER

//#include "BlueMarbleMaps/System/JsonFile.h"
#include "BlueMarbleMaps/Core/Serialization/JsonValue.h"
#include "BlueMarbleMaps/Core/Feature.h"

namespace BlueMarble
{
    class GeoJsonSerializer
    {
        public:
            static FeatureCollectionPtr deserialize(const JsonValue& jsonValue);
            static FeatureCollectionPtr deserializeFeatureCollection(const JsonValue& jsonValue);
            static FeaturePtr deserializeFeature(const JsonValue& jsonValue);
            static GeometryPtr deserializeGeometry(const JsonValue& jsonValue);
            static Attributes deserializeProperties(const JsonValue& jsonValue);
            static GeometryPtr deserializePointGeometry(const JsonValue& coorcinates);
            static GeometryPtr deserializeLineGeometry(const JsonValue& coorcinates);
            static GeometryPtr deserializePolygon(const JsonValue& coorcinates);
            static GeometryPtr deserializeMultiPolygon(const JsonValue& coorcinates);
            static std::vector<Point> extractPoints(const JsonValue& pointList);
            static Point extractPoint(const JsonValue& point);
            static double extractCoordinate(const JsonValue& coordinate);
    };

}
#endif /* BLUEMARBLE_JSONSERIALIZER */
