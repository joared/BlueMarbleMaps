#include "BlueMarbleMaps/Core/DataSets/ImageDataSet.h"
#include <gdal_priv.h>
#include <cpl_conv.h>

using namespace BlueMarble;

FeaturePtr loadWithGDAL(const std::string& filePath)
{
    static bool gdalInitialized = false;
    if (!gdalInitialized)
    {
        GDALAllRegister();
        gdalInitialized = true;
    }

    GDALDataset* ds = static_cast<GDALDataset*>(
        GDALOpen(filePath.c_str(), GA_ReadOnly)
    );

    if (!ds)
        return nullptr;

    int width  = ds->GetRasterXSize();
    int height = ds->GetRasterYSize();
    int channels = ds->GetRasterCount();

    if (channels == 0)
    {
        GDALClose(ds);
        return nullptr;
    }

    double gt[6];
    Rectangle bounds;
    if (ds->GetGeoTransform(gt) == CE_None)
    {
        double origX = gt[0];
        double origY = gt[3];
        double pixelSizeX = gt[1];
        double pixelSizeY = gt[5];
        bounds = Rectangle(origX, 
                           origY+height*pixelSizeY,  // Use the original width              
                           origX+width*pixelSizeX, // Use the original height
                           origY);
    }
    else
    {
        // No georef
        GDALClose(ds);
        return nullptr;
    }

    // Force RGB or grayscale output
    int outBands = std::min(channels, 3);
    
    // Adjust size such that it fits in gpu memory
    int maxTex = 16384;
    int newW = std::min(width, maxTex);
    int newH = std::min(height, maxTex);
    unsigned char* data = new unsigned char[newW * newH * outBands];

    int bandMap[3] = {1, 2, 3};
    if (outBands == 1)
        bandMap[0] = bandMap[1] = bandMap[2] = 1;
    else
        bandMap[0] = 1, bandMap[1] = 2, bandMap[2] = 3;

    CPLErr err = ds->RasterIO(
        GF_Read,
        0, 0,
        width, height,
        data,
        newW, newH,
        GDT_Byte,
        outBands,
        bandMap,
        outBands,
        outBands * newW,
        1
    );

    if (err != CE_None)
    {
        GDALClose(ds);
        delete[] data;
        data = nullptr;
        return nullptr;
    }

    auto projectionWKT = ds->GetProjectionRef();
    auto crs = Crs::fromWkt(projectionWKT);
    if (crs == nullptr)
    {
        BMM_DEBUG() << "Unsupported SRS: " << projectionWKT << ". Falling back to WGS84\n";
        crs = Crs::wgs84LngLat();
    }
    GDALClose(ds);

    auto raster = Raster(data, newW, newH, outBands);
    auto rasterGeometry = std::make_shared<RasterGeometry>(raster, bounds);

    auto feature = std::make_shared<Feature>(
        Id(0,0),
        crs,
        rasterGeometry
    );

    return feature;
}


ImageDataSet::ImageDataSet()
    : DataSet() 
    , m_filePath("")
    , m_rasterGeometry(nullptr)
    , m_overViews()
{
}


ImageDataSet::ImageDataSet(const std::string &filePath)
    : DataSet()
    , m_filePath(filePath)
    , m_rasterGeometry(nullptr)
    , m_overViews()
{
}


void ImageDataSet::init()
{
    assert(!m_filePath.empty());

    // Reference: https://help.allplan.com/Allplan/2016-1/1033/Allplan/51691.htm
    // Line 1: length of a pixel in the x direction (horizontal)
    // Line 2: angle of rotation (is usually 0 or ignored)
    // Line 3: angle of rotation (is usually 0 or ignored)
    // Line 4: negative length of a pixel in the y direction (vertical)
    // Line 5: x coordinate at the center of the pixel in the top left corner of the image
    // Line 6: y coordinate at the center of the pixel in the top left corner of the image
    
    // FIXME: Hardcoded
    // constexpr double xPixLen = 0.02222222222222*2;   // times 2 since we made the image smaller
    // constexpr double yPixLen = -0.02222222222222*2;  // times 2 since we made the image smaller
    // constexpr double xTopLeft = -179.98888888888889;
    // constexpr double yTopLeft = 89.98888888888889;

    // auto raster = Raster(m_filePath);
    // auto bounds = Rectangle(-179.98888888888889, -90.011111111093086, 180.01111111107508, 89.98888888888889);
    
    
    // BMM_DEBUG() << "MY RASTER AREA: " << bounds.toString() << "\n";
    // m_rasterGeometry = std::make_shared<RasterGeometry>(raster, bounds);
    // m_rasterFeature = std::make_shared<Feature>(generateId(), crs(), m_rasterGeometry);

    
    m_rasterFeature = loadWithGDAL(m_filePath);
    if (!m_rasterFeature)
    {
        throw std::runtime_error("Failed to read image file: " + m_filePath);
    }
    m_rasterGeometry = m_rasterFeature->geometryAsRaster();
    if (!m_rasterGeometry)
    {
        throw std::runtime_error("Not a raster geometry from this file!: " + m_filePath);
    }

    // Overviews
    //generateOverViews();
    BMM_DEBUG() << "ImageDataSet: Data loaded: \n";
    BMM_DEBUG() << "Width: " << m_rasterGeometry->raster().width() << "\n";
    BMM_DEBUG() << "Height: " << m_rasterGeometry->raster().height() << "\n";
    BMM_DEBUG() << "Channels: " << m_rasterGeometry->raster().channels() << "\n";
    BMM_DEBUG() << "Bounds: " << m_rasterGeometry->bounds().toString() << "\n";
}

IdCollectionPtr ImageDataSet::onGetFeatureIds(const FeatureQuery &featureQuery)
{
    auto ids = std::make_shared<IdCollection>();
    if(featureQuery.area().overlap(m_rasterGeometry->bounds()))
    {
        ids->add(m_rasterFeature->id());
    }

    return ids;
}

FeatureEnumeratorPtr ImageDataSet::onGetFeatures(const FeatureQuery &featureQuery)
{
    auto features = std::make_shared<FeatureEnumerator>();
    if (!isInitialized()) // TODO: move to base class
    {
        return features;
    }

    // Overviews
    // int invScaleIndex = 1.0 / featureQuery.scale() / 2.0;
    
    // if (m_overViews.find(invScaleIndex) != m_overViews.end())
    // {
    //     // Found an overview!
    // }
    // else
    // {
    //     // Did not find an overview, use lowest resolution
    //     invScaleIndex = m_overViews.size()-1;
    // }

    // auto& rasterGeometry = m_overViews[invScaleIndex];
    if(featureQuery.area().overlap(m_rasterGeometry->bounds()))
    {
        //auto feature = std::make_shared<Feature>(Id(0,0), rasterGeometry); // TODO: overviews improves performance for software implementation
        features->add(m_rasterFeature);
    }

    return features;
}

FeaturePtr ImageDataSet::onGetFeature(const Id &id)
{
    // TODO
    if (id == m_rasterFeature->id())
        return m_rasterFeature;

    return nullptr;
}

void ImageDataSet::filePath(const std::string &filePath)
{
    m_filePath = filePath;
}

void ImageDataSet::generateOverViews()
{
    const int LIMIT = 500; // TODO
    int s = 0;
    m_overViews.emplace(s, m_rasterGeometry);
    s++;

    const Raster& rasterOrig = m_rasterGeometry->raster();
    const Rectangle& boundsOrig = m_rasterGeometry->bounds();

    std::cout << "Generating overviews...\n";
    while (true)
    {
        double factor = std::pow(0.5, s);
        std::cout << "Copy\n";
        Raster overview = rasterOrig;
        std::cout << "Resize raster overview\n";
        overview.resize((int)(overview.width()*factor), 
                        (int)(overview.height()*factor), 
                        Raster::ResizeInterpolation::NearestNeighbor);

        if (overview.width() < LIMIT || overview.height() < LIMIT)
        {
            break;
        }
        std::cout << "Add: " << overview.width() << ", " << overview.height() << "\n";
        double cellWidth = 1.0/factor;
        double cellHeight = 1.0/factor;
        auto bounds = boundsOrig; // Same bounds
        auto rasterGeometry = std::make_shared<RasterGeometry>(overview, bounds);
        m_overViews.emplace(s, rasterGeometry);

        std::cout << "Overview: " << s << "\n";
        s++;
    }
}