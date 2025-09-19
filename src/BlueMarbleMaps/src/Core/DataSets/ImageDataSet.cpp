#include "BlueMarbleMaps/Core/DataSets/ImageDataSet.h"

using namespace BlueMarble;

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

    // FIXME: Hardcoded
    constexpr double xPixLen = 0.02222222222222;
    constexpr double yPixLen = -0.02222222222222;
    constexpr double xTopLeft = -179.98888888888889;
    constexpr double yTopLeft = 89.98888888888889;

    auto raster = Raster(m_filePath);
    auto bounds = Rectangle(xTopLeft, yTopLeft, raster.width()*xPixLen, raster.height()*yPixLen);
    m_rasterGeometry = std::make_shared<RasterGeometry>(raster, bounds, xPixLen, yPixLen);

    generateOverViews();
    std::cout << "ImageDataSet: Data loaded!\n";
}


FeatureEnumeratorPtr ImageDataSet::getFeatures(const FeatureQuery &featureQuery)
{
    auto features = std::make_shared<FeatureEnumerator>();
    if (!isInitialized()) // TODO: move to base class
    {
        return features;
    }

    int invScaleIndex = 1.0 / featureQuery.scale() / 2.0;
    
    if (m_overViews.find(invScaleIndex) != m_overViews.end())
    {
        // Found an overview!
    }
    else
    {
        // Did not find an overview, use lowest resolution
        invScaleIndex = m_overViews.size()-1;
    }

    auto& rasterGeometry = m_overViews[invScaleIndex];
    if(featureQuery.area().overlap(m_rasterGeometry->bounds()))
    {
        //auto feature = std::make_shared<Feature>(Id(0,0), rasterGeometry); // TODO: overviews improves performance for software implementation
        auto feature = std::make_shared<Feature>(Id(0,0), getCrs(), m_rasterGeometry);
        features->add(feature);
    }

    return features;
}

FeaturePtr ImageDataSet::getFeature(const Id &id)
{
    // TODO
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
        auto rasterGeometry = std::make_shared<RasterGeometry>(overview, bounds, cellWidth, cellHeight);
        m_overViews.emplace(s, rasterGeometry);

        std::cout << "Overview: " << s << "\n";
        s++;
    }
}