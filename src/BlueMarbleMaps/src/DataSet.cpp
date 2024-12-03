#include "DataSet.h"
#include "Map.h"
#include "File.h"
#include "Utils.h"
#include "FeatureAnimation.h"

#include <cassert>
#include <set>
#include <thread>
#include <functional>

using namespace BlueMarble;

Id BlueMarble::DataSet::generateId()
{
    static std::atomic<FeatureId> globalFeatureIdCounter = 0;
    globalFeatureIdCounter++;
    return Id(m_dataSetId, globalFeatureIdCounter);
}

FeaturePtr DataSet::createFeature(GeometryPtr geometry)
{
    auto feature = std::make_shared<Feature>
    (
        generateId(),
        geometry
    );

    return feature;
}

void DataSet::restartVisualizationAnimation(FeaturePtr feature, int64_t timeStamp)
{
    assert(feature->id().dataSetId() == dataSetId());
    if (timeStamp == -1)
        timeStamp = getTimeStampMs();

    //feature->attributes().set(FeatureAttributeKeys::StartAnimationTimeMs, timeStamp);
    m_idToVisualizationTimeStamp[feature->id()] = timeStamp;
}


void DataSet::initialize(DataSetInitializationType initType)
{
    assert(!m_isInitialized);
    auto initWork = [this]()
    {
        init();
        dataSets[m_dataSetId] = shared_from_this();
        m_isInitialized = true;
    };

    if (initType == DataSetInitializationType::RightHereRightNow)
    {
        initWork();
    }
    else
    {
        std::thread readThread(initWork);
        readThread.detach();
    }
}

/* Returns the stored start timestamp for animated visualization if the 
id exists, otherwise -1;*/
int64_t BlueMarble::DataSet::getVisualizationTimeStampForFeature(const Id& id)
{
    auto it = m_idToVisualizationTimeStamp.find(id);
    if (it != m_idToVisualizationTimeStamp.end())
        return it->second;
    
    return -1;
}

std::map<DataSetId, DataSetPtr> DataSet::dataSets;

DataSetPtr DataSet::getDataSetById(const DataSetId& dataSetId)
{
    if (dataSets.find(dataSetId) == dataSets.end())
        return nullptr;
    
    return dataSets[dataSetId];
}

DataSet::DataSet()
    : m_dataSetId(DataSetId(this))
    , m_isInitialized(false)
    , m_idToVisualizationTimeStamp()
{
}

BlueMarble::DataSet::~DataSet()
{
    dataSets.erase(m_dataSetId);
}

ImageDataSet::ImageDataSet()
    : DataSet() 
    , m_filePath("")
    , m_raster(0,0,0,0) // Prevent warning
    , m_rasterPrev(0,0,0,0) // Prevent warning
    , m_overViews()
{
}


ImageDataSet::ImageDataSet(const std::string &filePath)
    : DataSet()
    , m_filePath(filePath)
    , m_raster(0,0,0,0) // Prevent warning
    , m_rasterPrev(0,0,0,0) // Prevent warning
    , m_overViews()
{
}


void ImageDataSet::init()
{
    assert(!m_filePath.empty());
    m_raster = Raster(m_filePath);
    m_rasterPrev = m_raster;
    generateOverViews();
    std::cout << "ImageDataSet: Data loaded!\n";
}

void BlueMarble::ImageDataSet::onUpdateRequest(Map &map, const Rectangle& updateArea, FeatureHandler* handler)
{
    if (!isInitialized()) // TODO: move to base class
    {
        return;
    }
    // first get the overview with specific inverted scale
    // TODO: Shouldn't be divided by two. Temporary fix. Fix together with intepolation and limit in generateOverviews
    int invScaleIndex = (int)map.invertedScale() / 2.0;
    
    if (m_overViews.find(invScaleIndex) != m_overViews.end())
    {
        // Found an overview!
    }
    else
    {
        // Did not find an overview, use lowest resolution
        invScaleIndex = m_overViews.size()-1;
    }

    auto& backgroundRaster = m_overViews[invScaleIndex];

    double invScaleX = (double)m_raster.width() / backgroundRaster.width();
    double invScaleY = (double)m_raster.height() / backgroundRaster.height();

    // Crop image
    auto bounds = updateArea;//map.area(); // TODO: testing, remove!!!!
    int x0 = std::max(0.0, bounds.xMin() / invScaleX);
    int y0 = std::max(0.0, bounds.yMin() / invScaleY);
    int x1 = std::min(backgroundRaster.width()-1.0, bounds.xMax() / invScaleX);
    int y1 = std::min(backgroundRaster.height()-1.0, bounds.yMax() / invScaleY);

    if (x0 > x1 || y0 > y1)
        return;

    auto newImage = backgroundRaster.getCrop(std::round(x0), 
                                            std::round(y0), 
                                            std::round(x1), 
                                            std::round(y1));

    
    // Resize the cropped image
    int newWidth = std::round(newImage.width()*map.scale()*invScaleX);
    int newHeight = std::round(newImage.height()*map.scale()*invScaleY);
    newImage.resize(newWidth, newHeight, Raster::ResizeInterpolation::NearestNeighbor); // TODO: Want to use NoInterpolation but it doesn't work

    // Compute the position to center the image
    auto displayCenter = map.screenCenter();
    
    auto offset = Point(-map.center().x()*map.scale() + displayCenter.x(),
                        -map.center().y()*map.scale() + displayCenter.y());

    // Account for pixel truncation in offset
    //auto offset2 = map.mapToScreen(bounds).minCorner(); // TODO remove if not working
    if (offset.x() < 0)
    {
        offset = Point((std::round(x0)*invScaleX - bounds.xMin())*map.scale(), offset.y());
        //offset = Point(offset.x()+offset2.x(), offset.y());
    }

    if (offset.y() < 0)
    {
        offset = Point(offset.x(), (std::round(y0)*invScaleY - bounds.yMin())*map.scale());
        //offset = Point(offset.x(), offset.y()+offset2.y());
    }

    offset = offset.round();


    // std::string info = "Overview index " + std::to_string(invScaleIndex) + "(" + std::to_string(backgroundRaster.width()) + ", " + std::to_string(backgroundRaster.height()) + ")";
    // map.drawable().drawText(map.drawable().width() / 2.0, 10, info, Color(0,0,0));

    auto geometry = std::make_shared<RasterGeometry>(newImage, offset);
    auto feature = std::make_shared<Feature>(Id(0,0), geometry);

    handler->onFeatureInput(map, std::vector<FeaturePtr>({ feature }));
    // map.drawable().drawRaster(offset.x(), offset.y(), newImage, 0.9);
    // std::string info = "Overview index " + std::to_string(invScaleIndex) + "(" + std::to_string(backgroundRaster.width()) + ", " + std::to_string(backgroundRaster.height()) + ")";
    // map.drawable().drawText(map.drawable().width() / 2.0, 10, info, Color(0,0,0));
}

void ImageDataSet::OnUpdateRequest(Map &map, const Rectangle& updateArea)
{
    OnUpdateRequestOld(map, updateArea);
    // if (!m_isInitialized)
    // {
    //     init();
    // }
    // // first get the overview with specific inverted scale
    // int invScaleIndex = (int)map.invertedScale() / 2.0;
    
    // if (m_overViews.find(invScaleIndex) != m_overViews.end())
    // {
    //     // Found an overview!
    // }
    // else
    // {
    //     // Did not find an overview, use lowest resolution
    //     invScaleIndex = m_overViews.size()-1;

    // }
    // auto& backgroundRaster = m_overViews[invScaleIndex];

    // double invScaleX = (double)m_raster.width() / backgroundRaster.width();
    // double invScaleY = (double)m_raster.height() / backgroundRaster.height();

    // // Crop image
    // Raster newImage;
    // auto bounds = map.area();
    // if (map.scaleChanged())
    // {
    //     int x0 = std::max(0.0, bounds.xMin() / invScaleX);
    //     int y0 = std::max(0.0, bounds.yMin() / invScaleY);
    //     int x1 = std::min(backgroundRaster.width()-1.0, bounds.xMax() / invScaleX);
    //     int y1 = std::min(backgroundRaster.height()-1.0, bounds.yMax() / invScaleY);
    //     newImage = backgroundRaster.getCrop(std::round(x0), 
    //                                             std::round(y0), 
    //                                             std::round(x1), 
    //                                             std::round(y1));

    //     // Resize the cropped image
    //     int newWidth = newImage.width()*map.scale()*invScaleX;
    //     int newHeight = newImage.height()*map.scale()*invScaleY;
    //     newImage.resize(newWidth, newHeight);
    //     m_rasterPrev = newImage;
    // }
    // else
    // {
    //     // TODO
    // }

    // // Compute the position to center the image
    // auto displayCenter = map.screenCenter();
    // auto offset = Point(-map.center().x()*map.scale() + displayCenter.x(),
    //                     -map.center().y()*map.scale() + displayCenter.y());

    // // Account for pixel truncation in offset
    // if (offset.x() < 0)
    //         offset = Point((std::round(x0)*invScaleX - bounds.xMin())*map.scale(), offset.y());
    // if (offset.y() < 0)
    //         offset = Point(offset.x(), (std::round(y0)*invScaleY - bounds.yMin())*map.scale());
    // offset = offset.round();

    // // Draw new image
    // // TODO: Rotation
    // //(*(cimg_library::CImg<unsigned char>*)newImage.data()).rotate(m_rotation);
    // //drawImg.rotate(m_rotation, m_center.x(), m_center.y()); 
    // map.drawable().drawRaster(offset.x(), offset.y(), newImage, 1.0);
}

void ImageDataSet::OnUpdateRequestOld(Map &map, const Rectangle& /*updateArea*/)
{
    if (!isInitialized()) // TODO: move to base class
    {
        init();
    }
    // first get the overview with specific inverted scale
    // TODO: Shouldn't be divided by two. Temporary fix. Fix together with intepolation and limit in generateOverviews
    int invScaleIndex = (int)map.invertedScale() / 2.0;
    
    if (m_overViews.find(invScaleIndex) != m_overViews.end())
    {
        // Found an overview!
    }
    else
    {
        // Did not find an overview, use lowest resolution
        invScaleIndex = m_overViews.size()-1;
    }

    auto& backgroundRaster = m_overViews[invScaleIndex];

    double invScaleX = (double)m_raster.width() / backgroundRaster.width();
    double invScaleY = (double)m_raster.height() / backgroundRaster.height();

    // Crop image
    auto bounds = map.area();
    int x0 = std::max(0.0, bounds.xMin() / invScaleX);
    int y0 = std::max(0.0, bounds.yMin() / invScaleY);
    int x1 = std::min(backgroundRaster.width()-1.0, bounds.xMax() / invScaleX);
    int y1 = std::min(backgroundRaster.height()-1.0, bounds.yMax() / invScaleY);

    if (x0 > x1 || y0 > y1)
        return;

    auto newImage = backgroundRaster.getCrop(std::round(x0), 
                                            std::round(y0), 
                                            std::round(x1), 
                                            std::round(y1));

    // Resize the cropped image
    int newWidth = newImage.width()*map.scale()*invScaleX;
    int newHeight = newImage.height()*map.scale()*invScaleY;
    newImage.resize(newWidth, newHeight, Raster::ResizeInterpolation::NearestNeighbor); // TODO: Want to use NoInterpolation but it doesn't work

    // Compute the position to center the image
    auto displayCenter = map.screenCenter();
    auto offset = Point(-map.center().x()*map.scale() + displayCenter.x(),
                        -map.center().y()*map.scale() + displayCenter.y());

    // Account for pixel truncation in offset
    if (offset.x() < 0)
            offset = Point((std::round(x0)*invScaleX - bounds.xMin())*map.scale(), offset.y());
    if (offset.y() < 0)
            offset = Point(offset.x(), (std::round(y0)*invScaleY - bounds.yMin())*map.scale());
    offset = offset.round();

    // Draw new image
    // TODO: Rotation
    //(*(cimg_library::CImg<unsigned char>*)newImage.data()).rotate(m_rotation);
    //drawImg.rotate(m_rotation, m_center.x(), m_center.y()); 
    map.drawable()->drawRaster(offset.x(), offset.y(), newImage, 0.9);
    std::string info = "Overview index " + std::to_string(invScaleIndex) + "(" + std::to_string(backgroundRaster.width()) + ", " + std::to_string(backgroundRaster.height()) + ")";
    map.drawable()->drawText(map.drawable()->width() / 2.0, 10, info, Color(0,0,0));
}

void ImageDataSet::filePath(const std::string &filePath)
{
    m_filePath = filePath;
}

void ImageDataSet::generateOverViews()
{
    const int LIMIT = 500; // TODO
    int s = 0;
    m_overViews.emplace(s, m_raster);
    s++;

    std::cout << "Generating overviews...\n";
    while (true)
    {
        double factor = std::pow(0.5, s);
        std::cout << "Copy\n";
        Raster overview = m_raster;
        std::cout << "Resize\n";
        overview.resize((int)overview.width()*factor, (int)overview.height()*factor, Raster::ResizeInterpolation::NearestNeighbor);
        if (overview.width() < LIMIT || overview.height() < LIMIT)
        {
            break;
        }
        std::cout << "Add: " << overview.width() << ", " << overview.height() << "\n";
        m_overViews.emplace(s, overview);

        std::cout << "Overview: " << s << "\n";
        s++;
    }
}


AbstractFileDataSet::AbstractFileDataSet(const std::string& filePath)
    : DataSet() 
    , m_filePath(filePath)
    , m_featureTree(Rectangle(-180.0, -90.0, 180.0, 90.0), 0.05)
    , m_progress(0)
{
}

void AbstractFileDataSet::init()
{
    read(m_filePath);
    for (auto f : m_features)
    {
        m_featureTree.insertFeature(f);
    }
    std::cout << "AbstractFileDataSet::init() Data loaded!\n";
}

double AbstractFileDataSet::progress()
{
    return (isInitialized()) ? 1.0 : (double)m_progress;
}

void AbstractFileDataSet::onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler)
{
    if (!isInitialized()) // TODO: move to base class
    {
        return;
    }
    // Testing QuadTree
    FeatureCollection filteredFeatures;
    auto lngLat = map.mapToLngLat(updateArea);
    // std::cout << lngLat.toString() << "\n";
    m_featureTree.getFeaturesInside(lngLat, filteredFeatures);
    // std::cout << "Filtered: " << filteredFeatures.size() << "\n";

    //
    // auto node = m_featureTree.root();

    // auto points = node->bounds().corners();
    // points.push_back(points[0]);
    // points = map.lngLatToScreen(points);
    // map.drawable().drawLine(points, Color::red());
    // auto center = node->bounds().center();
    // center = map.lngLatToScreen(center);
    // map.drawable().drawText(center.x(), center.y(), "Root node", Color::green());

    // int i=0;
    // for (auto c : node->children())
    // {
    //     auto points = c->bounds().corners();
    //     points.push_back(points[0]);
    //     points = map.lngLatToScreen(points);
    //     map.drawable().drawLine(points, Color::red(0.5));
    //     auto center = c->bounds().center();
    //     center = map.lngLatToScreen(center);
    //     map.drawable().drawText(center.x(), center.y(), "Node: " + std::to_string(i), Color::green());
    //     i++;
    // }
    
    handler->onFeatureInput(map, filteredFeatures.getVector());

    // std::vector<FeaturePtr> filteredFeatures;    
    // for (auto f : m_features)
    // {
    //     switch (f->geometryType())
    //     {
    //     case GeometryType::Point:
    //     {
    //         auto& point = f->geometryAsPoint()->point();
        
    //         // Filter within bounds
    //         if (!updateArea.isInside(map.lngLatToMap(point)))
    //             continue;

    //         filteredFeatures.push_back(f);
    //         break;
    //     }

    //     case GeometryType::Line:
    //     {
    //         auto bounds = map.lngLatToMap(f->bounds());
    //         if (!bounds.overlap(updateArea))
    //             continue;

    //         filteredFeatures.push_back(f);
    //         break;
    //     }    
        
    //     case GeometryType::Polygon:
    //     {
    //         auto bounds = map.lngLatToMap(f->bounds());
    //         if (!bounds.overlap(updateArea))
    //             continue;

    //         filteredFeatures.push_back(f);
    //         break;
    //     }

    //     default:
    //         std::cout << "AbstractFileDataSet::onUpdateRequest() Unhandled feature type: " << (int)f->geometryType() << "\n";
    //         break;
    //     }

    // }

    //handler->onFeatureInput(map, filteredFeatures);
}

void BlueMarble::AbstractFileDataSet::onGetFeaturesRequest(const Attributes &attributes, std::vector<FeaturePtr>& features)
{
    std::cout << "AbstractFileDataSet::onGetFeaturesRequest\n";
    if (attributes.size() == 0)
    {
        for (auto f : m_features)
            features.push_back(f);
        return;
    }

    for (auto f : m_features)
    {
        for (auto attr : attributes)
        {
            if (f->attributes().contains(attr.first))
            {
                try
                {
                    if (f->attributes().get<std::string>(attr.first)
                        == attributes.get<std::string>(attr.first))
                    {
                        // Its a match! Add to feature list
                        features.push_back(f);
                    }
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
                
                
            }
        }
    }
}

FeaturePtr AbstractFileDataSet::onGetFeatureRequest(const Id &id)
{
    std::cout << "AbstractFileDataSet::onGetFeatureRequest\n";
    for (auto f : m_features)
    {
        if (f->id() == id)
        {
            std::cout << "Found a match!\n";
            return f;
        }
    }

    return nullptr;
}


CsvFileDataSet::CsvFileDataSet(const std::string& filePath)
    : AbstractFileDataSet(filePath)
{

}


void CsvFileDataSet::read(const std::string& filePath)
{
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

        m_features.push_back(feature);

        i++;
    }


    // All below is temporary
    // Testing generating counties with convex hull

    // Set NAME attribute from "Locality"
    for (auto f : m_features)
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
}


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
        std::make_shared<PointGeometry>(Point(18.063240, 59.334591))
    );
    stockholm->attributes().set("NAME", "Stockholm");

    // 59.334591, 18.063240
    auto goteborg = std::make_shared<Feature>
    (
        Id(0, 2), 
        std::make_shared<PointGeometry>(Point(11.954, 57.706))
    );
    goteborg->attributes().set("NAME", "Goteborg");

    m_features.push_back(stockholm);
    m_features.push_back(goteborg);
}

GeoJsonFileDataSet::GeoJsonFileDataSet(const std::string &filePath)
    : AbstractFileDataSet(filePath)
{
}

void GeoJsonFileDataSet::read(const std::string &filePath)
{
    // Specification: https://geojson.org/geojson-spec.html
    auto json = JSONFile(filePath);
    auto data = json.data();

    std::cout << "GeoJson file " << m_features.size() << " features.\n";
    handleJsonData(data);
    std::cout << "GeoJson file resulted in " << m_features.size() << " features.\n";
}

void BlueMarble::GeoJsonFileDataSet::save(const std::string &filePath) const
{
}

void GeoJsonFileDataSet::handleJsonData(JsonValue *jsonValue)
{
    auto jsonData = jsonValue->get<JsonData>();
    auto type = jsonData["type"]->get<std::string>();

    if (type == "FeatureCollection")
    {
        try
        {
            handleFeatureCollection(jsonData["features"]);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            std::string info;
            int idx(0);
            JSONFile::prettyString(jsonValue, info, idx);
            std::cerr << info;
            std::cerr << "Error when handling feature:\n";
            std::cerr << e.what() << '\n';
        }
    }
}

void GeoJsonFileDataSet::handleFeatureCollection(JsonValue *jsonValue)
{
    auto featureList = jsonValue->get<JsonList>();
    m_features.reserve(featureList.size());

    int i = 0;
    for (auto f : featureList)
    {
        FeaturePtr feature(nullptr);
        try
        {
            feature = handleFeature(f);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            std::string info;
            int idx(0);
            JSONFile::prettyString(f, info, idx);
            std::cerr << info;
            std::cerr << "Error when handling feature:\n";
            std::cerr << e.what() << '\n';
            return;
        }

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
                    auto polFeature = createFeature(std::make_shared<PolygonGeometry>(polygon));
                     // FIXME: temporary fix to make selection of one polygon select all polygons within a multipolygon
                    polFeature->id(commonId); // Use same id
                    auto attrIbutesCopy = feature->attributes();
                    polFeature->attributes() = attrIbutesCopy;
                    m_features.push_back(polFeature);
                }
            }
            else
            {
                m_features.push_back(feature);
            }
        }
        i++;
        m_progress = (double)i/featureList.size();
    }
}

FeaturePtr GeoJsonFileDataSet::handleFeature(JsonValue *jsonValue)
{
    // std::cout << "handleFeature\n";

    auto jsonData = jsonValue->get<JsonData>();
    auto type = jsonData["type"]->get<std::string>();
    assert(type == "Feature");

    auto geometry = handleGeometry(jsonData["geometry"]);
    if (geometry)
    {
        auto feature = createFeature(geometry);
        feature->attributes() = handleProperties(jsonData["properties"]);

        return feature;
    }

    return nullptr;
}

GeometryPtr GeoJsonFileDataSet::handleGeometry(JsonValue *jsonValue)
{
    // std::cout << "handleGeometry\n";

    JsonData jsonData;
    try
    {
        jsonData = jsonValue->get<JsonData>();
    }
    catch (...)
    {
        std::cout << "Failed to read geometry...\n";
        return nullptr;
    }
    
    // std::cout << "Geometry type\n";
    std::string type;
    try
    {
        type = jsonData["type"]->get<std::string>();
    }
    catch(const std::exception& e)
    {
        std::cout << "Failed to read geometry type...\n";
        return nullptr;
    }
    
    auto coordinates = jsonData["coordinates"];
    if (type == "Point")
    {
        return handlePointGeometry(coordinates);
    }
    else if (type == "LineString")
    {
        return handleLineGeometry(coordinates);
    }
    else if (type == "Polygon")
    {
        return handlePolygon(coordinates);
    }
    else if (type == "MultiPolygon")
    {
        return handleMultiPolygon(coordinates);
    }
    else
    {
        std::cout << "GeoJsonFileDataSet::handleGeometry() Unhandled geometry type: " << type << "\n";
    }

    return nullptr;
}

Attributes GeoJsonFileDataSet::handleProperties(JsonValue *jsonValue)
{
    // std::cout << "handleProperties\n";
    
    auto jsonData = jsonValue->get<JsonData>();
    auto attr = Attributes();

    for (auto& pair : jsonData)
    {
        auto value = pair.second;
        auto x = AttributeValue();
        if (value->isType<int>())
        {
            x = value->get<int>();
        }
        else if (value->isType<double>())
        {
            x = value->get<double>();
        }
        else if (value->isType<std::string>())
        {
            x = value->get<std::string>();
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
        auto name = jsonData["name_en"]->get<std::string>();
        auto c7 = jsonData["mapcolor7"]->get<int>();
        auto c8 = jsonData["mapcolor8"]->get<int>();
        auto c9 = jsonData["mapcolor9"]->get<int>();
        //auto c13 = jsonData["mapcolor13"]->get<int>();

        attr.set("NAME", name);
        attr.set("COLOR_R", 50*c7);
        attr.set("COLOR_G", 50*c8);
        attr.set("COLOR_B", 50*c9);
        attr.set("COLOR_A", 0.25);
    }
    else if(jsonData.find("namn1") != jsonData.end())
    {
        attr.set("NAME", jsonData["namn1"]->get<std::string>());
    }
    else if(jsonData.find("landskap") != jsonData.end())
    {
        attr.set("NAME", jsonData["landskap"]->get<std::string>());
        auto c1 = jsonData["landsdelskod"]->get<int>();
        auto c2 = jsonData["landskapskod"]->get<int>();
        auto c3 = 1;
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
        attr.set("NAME", jsonData["CONTINENT"]->get<std::string>());
    }


    return attr;
}

GeometryPtr GeoJsonFileDataSet::handlePointGeometry(JsonValue* coorcinates)
{
    auto point = extractPoint(coorcinates);
    return std::make_shared<PointGeometry>(point);
}

GeometryPtr GeoJsonFileDataSet::handleLineGeometry(JsonValue* coorcinates)
{
    // std::cout << "GeoJsonFileDataSet::handleLineGeometry\n";

    JsonList ringList = coorcinates->get<JsonList>();
    auto lineGeometry = std::make_shared<LineGeometry>();
    lineGeometry->points() = extractPoints(coorcinates);

    // std::cout << "line length: " << lineGeometry->points().size() << "\n";

    return lineGeometry;
}

GeometryPtr GeoJsonFileDataSet::handlePolygon(JsonValue* coorcinates)
{
    // std::cout << "handlePolygon\n";

    JsonList ringList = coorcinates->get<JsonList>();
    auto polygon = std::make_shared<PolygonGeometry>();

    // std::cout << "Polly coord size: " << coordinates.size() << "\n";
    for (auto ring : ringList)
    {
        auto points = extractPoints(ring);
        assert(points.size() > 3);          // An extra point exist in points where the last is the same as the first
        points.erase(points.end()-1);       // Remove the last point, we don't use it in our representation (redundant)

        auto r = std::vector<Point>();
        for (auto& p : points)
        {
            r.push_back(p);
        }

        polygon->rings().push_back(r);
    }

    return polygon;
}


GeometryPtr GeoJsonFileDataSet::handleMultiPolygon(JsonValue* coorcinates)
{
    // std::cout << "handleMultiPolygon\n";

    JsonList polygonList = coorcinates->get<JsonList>();

    auto multiPolygon = std::make_shared<MultiPolygonGeometry>();
    for (auto p : polygonList)
    {
        auto polGeom = std::static_pointer_cast<PolygonGeometry>(handlePolygon(p));
        multiPolygon->polygons().push_back(PolygonGeometry(*polGeom));
    }

    return multiPolygon;
}


std::vector<Point> GeoJsonFileDataSet::extractPoints(JsonValue* pointListValue)
{
    // std::cout << "GeoJsonFileDataSet::extractPoints\n";
    auto& pointList = pointListValue->get<JsonList>();
    std::vector<Point> points;
    for (auto p : pointList)
    {
        points.push_back(extractPoint(p));
    }

    return points;
}

Point GeoJsonFileDataSet::extractPoint(JsonValue* pointValue)
{
    auto& pointV = pointValue->get<JsonList>(); 
    assert(pointV.size() == 2);

    double lng = extractCoordinate(pointV[0]);
    double lat = extractCoordinate(pointV[1]);

    return Point(lng, lat);
}

double GeoJsonFileDataSet::extractCoordinate(JsonValue* coordinate)
{
    
    if (coordinate->isType<double>())
    {
        return coordinate->get<double>();
    }
    else if (coordinate->isType<int>())
    {
        return (double)coordinate->get<int>();
    }

    std::cout << "Failed to retrieve coordinate position: " << coordinate->typeAsString() << "\n";
    return 0;
}


MemoryDataSet::MemoryDataSet()
    : DataSet()
    , FeatureEventPublisher(false)
    , m_features()
    , m_idToFeatureAnimation()
{
}

FeaturePtr BlueMarble::MemoryDataSet::onGetFeatureRequest(const Id &id)
{
    for (auto& f : m_features)
    {
        if (f->id() == id)
            return f;
    }

    return nullptr;
}

void MemoryDataSet::startFeatureAnimation(FeaturePtr feature)
{
    assert(feature->id().dataSetId() == dataSetId());
    auto from = Point(-179, 38);
    auto to = Point(179, 31);
    startFeatureAnimation(feature, from, to);
}

void MemoryDataSet::startFeatureAnimation(FeaturePtr feature, const Point& from, const Point& to)
{
    assert(feature->id().dataSetId() == dataSetId());
    auto animation = std::make_shared<SinusoidalFeatureAnimation>(feature, from, to);
    m_idToFeatureAnimation[feature->id()] = animation;
}

// Currently only for triggering the onFeatureUpdated event
void MemoryDataSet::triggerFeatureUpdated(const FeaturePtr& feature)
{
    assert(feature->id().dataSetId() == dataSetId());
    if (featureEventsEnabled())
    {
        sendOnFeatureUpdated(feature);
    }
}

void MemoryDataSet::init()
{
}


void MemoryDataSet::addFeature(FeaturePtr feature)
{
    assert(feature->id().dataSetId() == dataSetId());

    m_features.add(feature);

    if (featureEventsEnabled())
        sendOnFeatureCreated(feature);
}


void MemoryDataSet::removeFeature(const Id &id)
{
    assert(id.dataSetId() == dataSetId());

    m_features.remove(id);

    if (featureEventsEnabled())
        sendOnFeatureDeleted(id);
}

void MemoryDataSet::onUpdateRequest(Map& map, const Rectangle &updateArea, FeatureHandler *handler)
{
    // TODO: feature animations should be performed before the OnUpdating event, not during rendering
    int timeStampMs = map.updateAttributes().get<int>(UpdateAttributeKeys::UpdateTimeMs);
    for (const auto& it : m_idToFeatureAnimation)
    {
        auto& animation = it.second;
        animation->updateTimeStamp(timeStampMs);
        if (animation->isFinished())
        {
            std::cout << "Feature animation finished (" << animation->feature()->id().toString() << ")\n";
            m_idToFeatureAnimation.erase(it.first);
        }
        map.updateAttributes().set(UpdateAttributeKeys::UpdateRequired, true);
    }

    std::vector<FeaturePtr> features;
    for (auto f : m_features)
    {
        if (f->isInside(map.mapToLngLat(updateArea)))
        {
            features.push_back(f);
        }
    }

    handler->onFeatureInput(map, features);
}

FeatureEventPublisher::FeatureEventPublisher(bool eventsEnabled)
{
    featureEventsEnabled(eventsEnabled);
}

void FeatureEventPublisher::addFeatureEventListener(IFeatureEventListener *listener, const Id &id)
{
    assert(featureEventsEnabled());

    auto it = m_listeners.find(id);
    if (it == m_listeners.end())
    {
        // No listeners exist. Create a new vector and add the listener
        m_listeners[id] = std::vector<IFeatureEventListener*>{listener};
        return;
    }

    // Listeners exist. Make sure it is not a double registration
    auto& listeners = it->second;
    for (auto l : listeners)
    {
        assert(l != listener);
    }

    // All ok. Add the listener
    listeners.push_back(listener);
}

void FeatureEventPublisher::removeFeatureEventListener(IFeatureEventListener* listener, const Id& id)
{
    assert(m_eventsEnabled);
    auto it = m_listeners.find(id);
    assert(it != m_listeners.end());

    auto& listeners = it->second;
    for (auto it2=listeners.begin(); it2!=listeners.end(); it2++)
    {
        if (*it2 == listener)
        {
            listeners.erase(it2);
            return;
        }
    }

    // Listener not found.
    std::cout << "FeatureEventPublisher::removeFeatureEventListener() called for a listener that is not registered.\n";
    throw std::exception();
}


bool FeatureEventPublisher::featureEventsEnabled()
{
    return m_eventsEnabled;
}


void FeatureEventPublisher::featureEventsEnabled(bool enabled)
{
    m_eventsEnabled = enabled;
}


void FeatureEventPublisher::sendOnFeatureCreated(const FeaturePtr& feature) const
{
    assert(m_eventsEnabled);
    auto it = m_listeners.find(feature->id());
    if (it == m_listeners.end())
        return;

    auto& listeners = it->second;
    for (auto listener : listeners)
    {
        listener->onFeatureCreated(feature);
    }
}

void FeatureEventPublisher::sendOnFeatureUpdated(const FeaturePtr& feature) const
{
    assert(m_eventsEnabled);
    auto it = m_listeners.find(feature->id());
    if (it == m_listeners.end())
        return;

    auto& listeners = it->second;
    for (auto listener : listeners)
    {
        listener->onFeatureUpdated(feature);
    }
}

void FeatureEventPublisher::sendOnFeatureDeleted(const Id& id) const
{
    assert(m_eventsEnabled);
    auto it = m_listeners.find(id);
    if (it == m_listeners.end())
        return;

    auto& listeners = it->second;
    for (auto listener : listeners)
    {
        listener->onFeatureDeleted(id);
    }
}
