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

// void GeoJsonFileDataSet::handleJsonData(JsonValue *jsonValue)
// {
//     auto jsonData = jsonValue->get<JsonData>();
//     auto type = jsonData["type"]->get<std::string>();

//     if (type == "FeatureCollection")
//     {
//         try
//         {
//             handleFeatureCollection(jsonData["features"]);
//         }
//         catch(const std::exception& e)
//         {
//             std::cerr << e.what() << '\n';
//             std::string info;
//             int idx(0);
//             JSONFile::prettyString(jsonValue, info, idx);
//             std::cerr << info;
//             std::cerr << "Error when handling feature:\n";
//             std::cerr << e.what() << '\n';
//         }
//     }
// }

// void GeoJsonFileDataSet::handleFeatureCollection(JsonValue *jsonValue)
// {
//     auto featureList = jsonValue->get<JsonList>();
//     m_features.reserve(featureList.size());

//     int i = 0;
//     for (auto f : featureList)
//     {
//         FeaturePtr feature(nullptr);
//         try
//         {
//             feature = handleFeature(f);
//         }
//         catch(const std::exception& e)
//         {
//             std::cerr << e.what() << '\n';
//             std::string info;
//             int idx(0);
//             JSONFile::prettyString(f, info, idx);
//             std::cerr << info;
//             std::cerr << "Error when handling feature:\n";
//             std::cerr << e.what() << '\n';
//             return;
//         }

//         if (feature)
//         {   
//             if (auto multiPolygon = feature->geometryAsMultiPolygon())
//             {
//                 // Special handling for multi-geometries (convert into normal geometries)
//                 // std::cout << "MultiPolygon size: " << multiPolygon->polygons().size() << "\n";
//                 //feature->attributes().set("SOURCE_GEOMETRY", "Multipolygon" + std::to_string(multiPolygon->polygons().size()));
//                 auto commonId = feature->id();
//                 for (auto polygon : multiPolygon->polygons())
//                 {
//                     auto polFeature = createFeature(std::make_shared<PolygonGeometry>(polygon));
//                      // FIXME: temporary fix to make selection of one polygon select all polygons within a multipolygon
//                      // Update: removed commonId since it can mess up caching
//                     //polFeature->id(commonId); // Use same id
                    
//                     auto attrIbutesCopy = feature->attributes();
//                     polFeature->attributes() = attrIbutesCopy;
//                     m_features.push_back(polFeature);
//                 }
//             }
//             else
//             {
//                 m_features.push_back(feature);
//             }
//         }
//         i++;
//         m_progress = (double)i/featureList.size();
//     }
// }

// FeaturePtr GeoJsonFileDataSet::handleFeature(JsonValue *jsonValue)
// {
//     // std::cout << "handleFeature\n";

//     auto jsonData = jsonValue->get<JsonData>();
//     auto type = jsonData["type"]->get<std::string>();
//     assert(type == "Feature");

//     auto geometry = handleGeometry(jsonData["geometry"]);
//     if (geometry)
//     {
//         auto feature = createFeature(geometry);
//         feature->attributes() = handleProperties(jsonData["properties"]);

//         return feature;
//     }

//     return nullptr;
// }

// GeometryPtr GeoJsonFileDataSet::handleGeometry(JsonValue *jsonValue)
// {
//     // std::cout << "handleGeometry\n";

//     JsonData jsonData;
//     try
//     {
//         jsonData = jsonValue->get<JsonData>();
//     }
//     catch (...)
//     {
//         std::cout << "Failed to read geometry...\n";
//         return nullptr;
//     }
    
//     // std::cout << "Geometry type\n";
//     std::string type;
//     try
//     {
//         type = jsonData["type"]->get<std::string>();
//     }
//     catch(const std::exception& e)
//     {
//         std::cout << "Failed to read geometry type...\n";
//         return nullptr;
//     }
    
//     auto coordinates = jsonData["coordinates"];
//     if (type == "Point")
//     {
//         return handlePointGeometry(coordinates);
//     }
//     else if (type == "LineString")
//     {
//         return handleLineGeometry(coordinates);
//     }
//     else if (type == "Polygon")
//     {
//         return handlePolygon(coordinates);
//     }
//     else if (type == "MultiPolygon")
//     {
//         return handleMultiPolygon(coordinates);
//     }
//     else
//     {
//         std::cout << "GeoJsonFileDataSet::handleGeometry() Unhandled geometry type: " << type << "\n";
//     }

//     return nullptr;
// }

// Attributes GeoJsonFileDataSet::handleProperties(JsonValue *jsonValue)
// {
//     // std::cout << "handleProperties\n";
    
//     auto jsonData = jsonValue->get<JsonData>();
//     auto attr = Attributes();

//     for (auto& pair : jsonData)
//     {
//         auto value = pair.second;
//         auto x = AttributeValue();
//         if (value->isType<int>())
//         {
//             x = value->get<int>();
//         }
//         else if (value->isType<double>())
//         {
//             x = value->get<double>();
//         }
//         else if (value->isType<std::string>())
//         {
//             x = value->get<std::string>();
//         }
//         else
//         {
//             // std::cout << "Attribute '" << pair.first << "' is of unsupported type and cannot be added to feature: " << value->typeAsString() << "\n";
//             continue;
//         }
        
//         attr.set(pair.first, x);
//     }

//     // TODO: remove below. Adding extra attributes for visualization
//     if (jsonData.find("name_en") != jsonData.end())
//     {
//         // "mapcolor7":2,"mapcolor8":5,"mapcolor9":7,"mapcolor13":7
//         auto name = jsonData["name_en"]->get<std::string>();
//         auto c7 = jsonData["mapcolor7"]->get<int>();
//         auto c8 = jsonData["mapcolor8"]->get<int>();
//         auto c9 = jsonData["mapcolor9"]->get<int>();
//         //auto c13 = jsonData["mapcolor13"]->get<int>();

//         attr.set("NAME", name);
//         attr.set("COLOR_R", 50*c7);
//         attr.set("COLOR_G", 50*c8);
//         attr.set("COLOR_B", 50*c9);
//         attr.set("COLOR_A", 0.25);
//     }
//     else if(jsonData.find("namn1") != jsonData.end())
//     {
//         attr.set("NAME", jsonData["namn1"]->get<std::string>());
//     }
//     else if(jsonData.find("landskap") != jsonData.end())
//     {
//         attr.set("NAME", jsonData["landskap"]->get<std::string>());
//         auto c1 = jsonData["landsdelskod"]->get<int>();
//         auto c2 = jsonData["landskapskod"]->get<int>();
//         auto c3 = 1;
//         if (c2 > 15)
//         {
//             c3 = c2-15;
//             c2 = 15;   
//         }
//         attr.set("COLOR_R", 20*c3);
//         attr.set("COLOR_G", 10*c2);
//         attr.set("COLOR_B", 50*c1);
//         attr.set("COLOR_A", 0.25);
//     }
//     else if (jsonData.find("CONTINENT") != jsonData.end())
//     {
//         attr.set("NAME", jsonData["CONTINENT"]->get<std::string>());
//     }


//     return attr;
// }

// GeometryPtr GeoJsonFileDataSet::handlePointGeometry(JsonValue* coorcinates)
// {
//     auto point = extractPoint(coorcinates);
//     return std::make_shared<PointGeometry>(point);
// }

// GeometryPtr GeoJsonFileDataSet::handleLineGeometry(JsonValue* coorcinates)
// {
//     // std::cout << "GeoJsonFileDataSet::handleLineGeometry\n";

//     JsonList ringList = coorcinates->get<JsonList>();
//     auto lineGeometry = std::make_shared<LineGeometry>();
//     lineGeometry->points() = extractPoints(coorcinates);

//     // std::cout << "line length: " << lineGeometry->points().size() << "\n";

//     return lineGeometry;
// }

// GeometryPtr GeoJsonFileDataSet::handlePolygon(JsonValue* coorcinates)
// {
//     // std::cout << "handlePolygon\n";

//     JsonList ringList = coorcinates->get<JsonList>();
//     auto polygon = std::make_shared<PolygonGeometry>();

//     // std::cout << "Polly coord size: " << coordinates.size() << "\n";
//     for (auto ring : ringList)
//     {
//         auto points = extractPoints(ring);
//         assert(points.size() > 3);          // An extra point exist in points where the last is the same as the first
//         points.erase(points.end()-1);       // Remove the last point, we don't use it in our representation (redundant)

//         auto r = std::vector<Point>();
//         for (auto& p : points)
//         {
//             r.push_back(p);
//         }

//         polygon->rings().push_back(r);
//     }

//     return polygon;
// }


// GeometryPtr GeoJsonFileDataSet::handleMultiPolygon(JsonValue* coorcinates)
// {
//     // std::cout << "handleMultiPolygon\n";

//     JsonList polygonList = coorcinates->get<JsonList>();

//     auto multiPolygon = std::make_shared<MultiPolygonGeometry>();
//     for (auto p : polygonList)
//     {
//         auto polGeom = std::static_pointer_cast<PolygonGeometry>(handlePolygon(p));
//         multiPolygon->polygons().push_back(PolygonGeometry(*polGeom));
//     }

//     return multiPolygon;
// }


// std::vector<Point> GeoJsonFileDataSet::extractPoints(JsonValue* pointListValue)
// {
//     // std::cout << "GeoJsonFileDataSet::extractPoints\n";
//     auto& pointList = pointListValue->get<JsonList>();
//     std::vector<Point> points;
//     for (auto p : pointList)
//     {
//         points.push_back(extractPoint(p));
//     }

//     return points;
// }

// Point GeoJsonFileDataSet::extractPoint(JsonValue* pointValue)
// {
//     auto& pointV = pointValue->get<JsonList>(); 
//     assert(pointV.size() == 2);

//     double lng = extractCoordinate(pointV[0]);
//     double lat = extractCoordinate(pointV[1]);

//     return Point(lng, lat);
// }

// double GeoJsonFileDataSet::extractCoordinate(JsonValue* coordinate)
// {
    
//     if (coordinate->isType<double>())
//     {
//         return coordinate->get<double>();
//     }
//     else if (coordinate->isType<int>())
//     {
//         return (double)coordinate->get<int>();
//     }

//     std::cout << "Failed to retrieve coordinate position: " << coordinate->typeAsString() << "\n";
//     return 0;
// }