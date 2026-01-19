#include "BlueMarbleMaps/Core/Serialization/GeoJsonSerializer.h"

using namespace BlueMarble;

FeatureCollectionPtr GeoJsonSerializer::deserialize(const JsonValue&jsonValue)
{
    auto jsonData = jsonValue.get<JsonValue::Object>();
    auto type = jsonData.at("type").get<std::string>();

    if (type == "FeatureCollection")
    {
        return deserializeFeatureCollection(jsonData.at("features"));
    }

    return FeatureCollectionPtr();
}

FeatureCollectionPtr GeoJsonSerializer::deserializeFeatureCollection(const JsonValue& jsonValue)
{
    auto& featureList = jsonValue.get<JsonValue::Array>();
    auto features = std::make_shared<FeatureCollection>();
    features->reserve(featureList.size());

    int i = 0;
    for (auto& f : featureList)
    {
        FeaturePtr feature(nullptr);
        feature = deserializeFeature(f);
        if (feature)
        {   
            if (auto multiPolygon = feature->geometryAsMultiPolygon())
            {
                // Special handling for multi-geometries (convert into normal geometries)
                // std::cout << "MultiPolygon size: " << multiPolygon->polygons().size() << "\n";
                //feature->attributes().set("SOURCE_GEOMETRY", "Multipolygon" + std::to_string(multiPolygon->polygons().size()));
                auto commonId = feature->id();
                for (auto polygon : multiPolygon->polygons())
                {
                    auto polFeature = std::make_shared<Feature>(Id(0,0), Crs::wgs84LngLat(), std::make_shared<PolygonGeometry>(polygon));
                     // FIXME: temporary fix to make selection of one polygon select all polygons within a multipolygon
                     // Update: removed commonId since it can mess up caching
                    //polFeature->id(commonId); // Use same id
                    
                    auto attrIbutesCopy = feature->attributes();
                    polFeature->attributes() = attrIbutesCopy;
                    features->add(polFeature);
                }
            }
            else if(auto multiLine = feature->geometryAsMultiLine())
            {
                //auto commonId = feature->id();
                for (auto line : multiLine->lines())
                {
                    auto lineFeature = std::make_shared<Feature>(Id(0,0), Crs::wgs84LngLat(), std::make_shared<LineGeometry>(line));
                     // FIXME: temporary fix to make selection of one polygon select all polygons within a multipolygon
                     // Update: removed commonId since it can mess up caching
                    //polFeature->id(commonId); // Use same id
                    
                    auto attrIbutesCopy = feature->attributes();
                    lineFeature->attributes() = attrIbutesCopy;
                    features->add(lineFeature);
                }
            }
            else
            {
                features->add(feature);
            }
        }
        i++;
    }

    return features;
}

FeaturePtr GeoJsonSerializer::deserializeFeature(const JsonValue& jsonValue)
{
    // std::cout << "deserializeFeature\n";

    auto& jsonData = jsonValue.get<JsonValue::Object>();
    const auto& type = jsonData.at("type").get<std::string>();
    assert(type == "Feature");

    auto geometry = deserializeGeometry(jsonData.at("geometry"));
    if (geometry)
    {
        auto feature = std::make_shared<Feature>(Id(0,0), Crs::wgs84LngLat(), geometry);
        feature->attributes() = deserializeProperties(jsonData.at("properties"));

        return feature;
    }
    else
    {
        throw std::runtime_error("Geometry is null!\n");
    }

    return nullptr;
}

GeometryPtr GeoJsonSerializer::deserializeGeometry(const JsonValue& jsonValue)
{
    // std::cout << "deserializeGeometry\n";

    const auto& jsonData = jsonValue.get<JsonValue::Object>();
    const auto& type = jsonData.at("type").get<std::string>();
    
    const auto& coordinates = jsonData.at("coordinates");
    if (type == "Point")
    {
        return deserializePointGeometry(coordinates);
    }
    else if (type == "LineString")
    {
        return deserializeLineGeometry(coordinates);
    }
    else if (type == "MultiLineString")
    {
        return deserializeMultiLineGeometry(coordinates);
    }
    else if (type == "Polygon")
    {
        return deserializePolygon(coordinates);
    }
    else if (type == "MultiPolygon")
    {
        return deserializeMultiPolygon(coordinates);
    }
    else
    {
        std::cout << "GeoJsonSerializer::deserializeGeometry() Unhandled geometry type: " << type << "\n";
    }

    return nullptr;
}

Attributes GeoJsonSerializer::deserializeProperties(const JsonValue& jsonValue)
{
    // std::cout << "deserializeProperties\n";
    
    const auto& jsonData = jsonValue.get<JsonValue::Object>();
    auto attr = Attributes();

    for (auto& pair : jsonData)
    {
        auto value = pair.second;
        auto x = AttributeValue();
        if (value.isInteger())
        {
            x = (int)value.asInteger();
        }
        else if (value.isType<double>())
        {
            x = value.get<double>();
        }
        else if (value.isType<std::string>())
        {
            x = value.get<std::string>();
        }
        else
        {
            // std::cout << "Attribute '" << pair.first << "' is of unsupported type and cannot be added to feature: " << value->typeAsString() << "\n";
            continue;
        }
        
        attr.set(pair.first, x);
    }

    // TODO: remove below. Adding extra attributes for visualization
    if (jsonData.find("name_en") != jsonData.end())
    {
        // "mapcolor7":2,"mapcolor8":5,"mapcolor9":7,"mapcolor13":7
        auto name = jsonData.at("name_en").get<std::string>();
        int c7 = jsonData.at("mapcolor7").asInteger();
        int c8 = jsonData.at("mapcolor8").asInteger();
        int c9 = jsonData.at("mapcolor9").asInteger();
        //auto c13 = jsonData.at("mapcolor13").asInteger();

        attr.set("NAME", name);
        attr.set("COLOR_R", 50*c7);
        attr.set("COLOR_G", 50*c8);
        attr.set("COLOR_B", 50*c9);
        attr.set("COLOR_A", 0.25);
    }
    else if(jsonData.find("namn1") != jsonData.end() && jsonData.at("namn1").isString())
    {
        attr.set("NAME", jsonData.at("namn1").get<std::string>());
    }
    else if(jsonData.find("landskap") != jsonData.end())
    {
        attr.set("NAME", jsonData.at("landskap").get<std::string>());
        int c1 = jsonData.at("landsdelskod").asInteger();
        int c2 = jsonData.at("landskapskod").asInteger();
        int c3 = 1;
        if (c2 > 15)
        {
            c3 = c2-15;
            c2 = 15;   
        }
        attr.set("COLOR_R", 20*c3);
        attr.set("COLOR_G", 10*c2);
        attr.set("COLOR_B", 50*c1);
        attr.set("COLOR_A", 0.25);
    }
    else if (jsonData.find("CONTINENT") != jsonData.end())
    {
        attr.set("NAME", jsonData.at("CONTINENT").get<std::string>());
    }


    return attr;
}

GeometryPtr GeoJsonSerializer::deserializePointGeometry(const JsonValue& coorcinates)
{
    auto point = extractPoint(coorcinates);
    return std::make_shared<PointGeometry>(point);
}

GeometryPtr GeoJsonSerializer::deserializeLineGeometry(const JsonValue& coorcinates)
{
    // std::cout << "GeoJsonSerializer::deserializeLineGeometry\n";

    const auto& ringList = coorcinates.get<JsonValue::Array>();
    auto lineGeometry = std::make_shared<LineGeometry>();
    lineGeometry->points() = std::move(extractPoints(coorcinates));

    // std::cout << "line length: " << lineGeometry->points().size() << "\n";

    return lineGeometry;
}

GeometryPtr GeoJsonSerializer::deserializePolygon(const JsonValue& coorcinates)
{
    // std::cout << "deserializePolygon\n";

    const auto& ringList = coorcinates.get<JsonValue::Array>();
    auto polygon = std::make_shared<PolygonGeometry>();

    // std::cout << "Polly coord size: " << coordinates.size() << "\n";
    for (const auto& ring : ringList)
    {
        auto points = std::move(extractPoints(ring));
        assert(points.size() > 3);          // An extra point exist in points where the last is the same as the first
        points.erase(points.end()-1);       // Remove the last point, we don't use it in our representation (redundant)

        polygon->rings().emplace_back(points);
    }

    return polygon;
}


GeometryPtr GeoJsonSerializer::deserializeMultiPolygon(const JsonValue& coorcinates)
{
    // std::cout << "deserializeMultiPolygon\n";

    const auto& polygonList = coorcinates.get<JsonValue::Array>();

    auto multiPolygon = std::make_shared<MultiPolygonGeometry>();
    for (const auto& p : polygonList)
    {
        auto polGeom = std::static_pointer_cast<PolygonGeometry>(deserializePolygon(p));
        multiPolygon->polygons().emplace_back(*polGeom);
    }

    return multiPolygon;
}

GeometryPtr GeoJsonSerializer::deserializeMultiLineGeometry(const JsonValue& coordinates)
{
    const auto& lineList = coordinates.get<JsonValue::Array>();

    auto multiLines = std::make_shared<MultiLineGeometry>();
    for (const auto& p : lineList)
    {
        auto lineGeom = std::static_pointer_cast<LineGeometry>(deserializeLineGeometry(p));
        multiLines->lines().emplace_back(*lineGeom);
    }

    return multiLines;
}

std::vector<Point> GeoJsonSerializer::extractPoints(const JsonValue& pointListValue)
{
    // std::cout << "GeoJsonSerializer::extractPoints\n";
    const auto& pointList = pointListValue.get<JsonValue::Array>();
    std::vector<Point> points;
    for (auto p : pointList)
    {
        points.push_back(extractPoint(p));
    }

    return points;
}

Point GeoJsonSerializer::extractPoint(const JsonValue& pointValue)
{
    auto& pointV = pointValue.get<JsonValue::Array>(); 
    assert(pointV.size() == 2);

    double lng = extractCoordinate(pointV[0]);
    double lat = extractCoordinate(pointV[1]);

    return Point(lng, lat);
}

double GeoJsonSerializer::extractCoordinate(const JsonValue& coordinate)
{
    
    if (coordinate.isType<double>())
    {
        return coordinate.get<double>();
    }
    else if (coordinate.isInteger())
    {
        return (double)coordinate.asInteger();
    }

    //std::cout << "Failed to retrieve coordinate position: " << coordinate->typeAsString() << "\n";

    return 0;
}