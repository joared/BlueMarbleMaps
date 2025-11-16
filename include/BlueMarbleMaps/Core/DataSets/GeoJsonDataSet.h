#ifndef BLUEMARBLE_GEOJSONDATASET
#define BLUEMARBLE_GEOJSONDATASET

#include "FileDataSet.h"

namespace BlueMarble
{
// Specification: https://geojson.org/geojson-spec.html
    class GeoJsonFileDataSet : public AbstractFileDataSet
    {
        public:
            GeoJsonFileDataSet(const std::string& filePath);
        protected:
            FeatureCollectionPtr read(const std::string& filePath) override final;
            void save(const std::string& filePath) const;
            // void handleJsonData(JsonValue* jsonValue);
            // void handleFeatureCollection(JsonValue* jsonValue);
            // FeaturePtr handleFeature(JsonValue* jsonValue);
            // GeometryPtr handleGeometry(JsonValue* jsonValue);
            // Attributes handleProperties(JsonValue *jsonValue);
            // GeometryPtr handlePointGeometry(JsonValue* coorcinates);
            // GeometryPtr handleLineGeometry(JsonValue* coorcinates);
            // GeometryPtr handlePolygon(JsonValue* coorcinates);
            // GeometryPtr handleMultiPolygon(JsonValue* coorcinates);
            // std::vector<Point> extractPoints(JsonValue* pointList);
            // Point extractPoint(JsonValue* point);
            // double extractCoordinate(JsonValue* coordinate);
    };
}

#endif /* BLUEMARBLE_GEOJSONDATASET */
