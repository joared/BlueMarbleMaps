#include "BlueMarbleMaps/Core/Index/FileDatabase.h"
//#include "BlueMarbleMaps/System/JsonFile.h"
#include "BlueMarbleMaps/Core/Serialization/GeoJsonSerializer.h"

#include <fstream>

using namespace BlueMarble;

JsonValue serializePoint(const Point& point)
{
    JsonValue::Array list;

    auto x = JsonValue(point.x());
    auto y = JsonValue(point.y());

    list.push_back(x);
    list.push_back(y);

    return JsonValue(list);
}

JsonValue serializePoints(const std::vector<Point>& points)
{
    JsonValue::Array list;

    for (const auto& p: points)
    {
        list.push_back(serializePoint(p));
    }

    return JsonValue(list);
}

JsonValue serializePointGeometry(const PointGeometryPtr& geometry)
{
    // std::cout << "Point\n";
    JsonValue::Object root;
    root["type"] = JsonValue("Point");
    root["coordinates"] = serializePoint(geometry->point());
    
    return JsonValue(root);
}

JsonValue serializeLineGeometry(const LineGeometryPtr& geometry)
{
    // std::cout << "Line\n";
    JsonValue::Object root;
    root["type"] = JsonValue("LineString");
    root["coordinates"] = serializePoints(geometry->points());
    
    return JsonValue(root);
}

JsonValue serializePolygonGeometry(const PolygonGeometryPtr& geometry)
{
    // std::cout << "Polygon\n";
    if (!geometry)
    {
        return nullptr;
    }
    JsonValue::Object root;
    root["type"] = JsonValue("Polygon");

    JsonValue::Array coordinates;
    for (const auto& ring : geometry->rings())
    {
        // In GeoJson, we have dublicate first/last points in a ring  (closed line)
        auto v = serializePoints(ring);
        auto& list = v.get<JsonValue::Array>();
        list.push_back(list[0]);

        coordinates.push_back(v);
    }

    root["coordinates"] = JsonValue(coordinates);
    
    return JsonValue(root);
}

JsonValue serializeGeometry(const GeometryPtr& geometry)
{
    JsonValue value;

    auto type = geometry->type();
    switch (type)
    {
    case GeometryType::Point:
        value = serializePointGeometry(std::dynamic_pointer_cast<PointGeometry>(geometry));
        break;
    case GeometryType::Line:
        value = serializeLineGeometry(std::dynamic_pointer_cast<LineGeometry>(geometry));
        break;
    case GeometryType::Polygon:
        value = serializePolygonGeometry(std::dynamic_pointer_cast<PolygonGeometry>(geometry));
        break;
    default:
        break;
    }   

    return value;
}

JsonValue serializeAttributes(Attributes& attrs)
{
    JsonValue::Object root;

    for (const auto& it : attrs)
    {
        const AttributeValue& val = it.second;

        switch (val.type())
        {
        case AttributeValueType::Boolean:
            root[it.first] = JsonValue(val.getBoolean());
            break;
        case AttributeValueType::Integer:
            root[it.first] = JsonValue(val.getInteger());
            break;
        case AttributeValueType::Double:
            root[it.first] = JsonValue(val.getDouble());
            break;
        case AttributeValueType::String:
            root[it.first] = JsonValue(val.getString());
            break;
        default:
            std::cout << "serializeAttributes(Attributes& attrs) Inhandled attribute type: " << (int)val.type() << "\n";
        }
    }

    return JsonValue(root);
}

std::string serializeFeature(const FeaturePtr& feature)
{
    JsonValue::Object root;
    root["type"] = JsonValue("Feature");
    root["id"] = JsonValue(JsonValue::Array({JsonValue((int)feature->id().dataSetId()), JsonValue((int)feature->id().featureId())}));
    root["geometry"] = serializeGeometry(feature->geometry());
    root["properties"] = serializeAttributes(feature->attributes());

    auto value = JsonValue(root);
    return value.toString();
}

Id deserializeId(const std::string& string)
{
    auto value = JsonValue::fromString(string);
    auto idList = value.get<JsonValue::Object>()["id"].get<JsonValue::Array>();
    auto id = Id(idList[0].asInteger(), idList[1].asInteger());

    return id;
}

FeaturePtr deserializeFeature(const std::string& string)
{
    auto value = JsonValue::fromString(string);
    FeaturePtr feature = GeoJsonSerializer::deserializeFeature(value);
    
    auto idList = value.get<JsonValue::Object>()["id"].get<JsonValue::Array>();
    auto id = Id(idList[0].asInteger(), idList[1].asInteger());
    
    feature->id(id);

    return feature;
}

FileDatabase::FileDatabase()
    : m_filePath()
    , m_index()
    , m_isLoaded()
    , m_file()
{
}


FeaturePtr FileDatabase::getFeature(const FeatureId& id)
{   
    auto record = m_index.at(id);
    auto str = m_file->getLine(record.lineOffset);
    auto f = deserializeFeature(str);

    return f;
}

FeatureCollectionPtr FileDatabase::getFeatures(const FeatureIdCollectionPtr& ids)
{
    auto features = std::make_shared<FeatureCollection>();
    getFeatures(ids, features);
    return features;
}

void FileDatabase::getFeatures(const FeatureIdCollectionPtr &ids, FeatureCollectionPtr &featuresOut)
{
    for (const auto& id : *ids)
    {
        featuresOut->add(getFeature(id));
    }
}

FeatureCollectionPtr FileDatabase::getAllFeatures()
{
    auto features = std::make_shared<FeatureCollection>();
    features->reserve(size());

    for (const auto& it : m_index)
    {
        auto f = getFeature(it.first);
        features->add(f);
    }

    return features;
}

void FileDatabase::removeFeature(const FeatureId& id)
{
    // TODO
}

size_t FileDatabase::size() const
{
    return m_index.size();
}

void FileDatabase::save(const std::string& path) const
{
    throw std::runtime_error("FileDatabase::save() Not implemented.");
}

bool FileDatabase::load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.good())
    {
        return false;
    }

    m_filePath = path;
    m_file = std::make_unique<File>(path);
    m_file->buildIndex();
    m_index.clear();

    int64_t lineIdx = 0;
    auto lines = m_file->getLines();
    for (const auto& line : lines)
    {
        // TODO
        Id id = deserializeId(line);
        m_index[id.featureId()] = FeatureRecord{lineIdx};
        lineIdx++;
    }

    return m_index.size() > 0;
}

bool FileDatabase::build(const FeatureCollectionPtr& features, const std::string& path)
{
    std::vector<std::string> lines;
    
    for (const auto& f : *features)//*getAllFeatures())
    {
        auto str = serializeFeature(f);
        // str.erase(std::remove_if(str.begin(), str.end(),
        //                      [](unsigned char c) { return std::isspace(c); }),
        //       str.end());
        lines.push_back(str);
    }
    
    File::writeLines(path, lines);

    return true;
}

void FileDatabase::verifyLoaded() const
{
    if (!m_isLoaded)
    {
        throw std::runtime_error("FileDatabase not loaded. Use load() to load.");
    }
}
