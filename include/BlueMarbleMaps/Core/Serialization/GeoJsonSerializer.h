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
            static GeometryPtr deserializePointGeometry(const JsonValue& coordinates);
            static GeometryPtr deserializeMultiLineGeometry(const JsonValue& coordinates);
            static GeometryPtr deserializeLineGeometry(const JsonValue& coordinates);
            static GeometryPtr deserializePolygon(const JsonValue& coordinates);
            static GeometryPtr deserializeMultiPolygon(const JsonValue& coordinates);
            static std::vector<Point> extractPoints(const JsonValue& pointList);
            static Point extractPoint(const JsonValue& point);
            static double extractCoordinate(const JsonValue& coordinate);
    };

}
#endif /* BLUEMARBLE_JSONSERIALIZER */
