#include "BlueMarbleMaps/Core/Layer/WmsLayer.h"
#include "BlueMarbleMaps/Core/Map.h"

#ifndef __EMSCRIPTEN__
#include <gdal_priv.h>
#include <cpl_conv.h>
#endif

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>


using namespace BlueMarble;

namespace
{
    constexpr int WmsLayerDefaultImageSize = 512;
    constexpr int WmsLayerMaxImageSize = 2048;
}

WmsLayer::WmsLayer()
    : Layer()
    , m_url()
    , m_layers()
    , m_format("image/png")
    , m_imageWidth(WmsLayerDefaultImageSize)
    , m_imageHeight(WmsLayerDefaultImageSize)
{
}

WmsLayer::~WmsLayer()
{
}

FeatureEnumeratorPtr WmsLayer::prepare(const CrsPtr &crs, const FeatureQuery& featureQuery)
{
    return getFeatures(crs, featureQuery, true);
}

void WmsLayer::update(const MapPtr &map, const FeatureEnumeratorPtr &features, const FeatureQuery &featureQuery)
{
    static RasterVisualizerPtr visualizer;
    if (!visualizer)
    {
        visualizer = std::make_shared<RasterVisualizer>();
    }

    map->drawable()->beginBatches();
    while (features->moveNext())
    {
        visualizer->renderFeature(
            *map->drawable(), 
            features->current(),
            map->updateAttributes(),
            featureQuery.area());
    }
    map->drawable()->endBatches();
}

FeatureEnumeratorPtr WmsLayer::getFeatures(const CrsPtr &crs, const FeatureQuery &featureQuery, bool activeLayersOnly)
{
    auto enumerator = std::make_shared<FeatureEnumerator>();

    if (!isActiveForQuery(featureQuery))
    {
        return enumerator;
    }

    // WmsLayer currently only supports requesting Web Mercator (EPSG:3857)
    if (!crs || !crs->isFunctionallyEquivalent(Crs::wgs84MercatorWeb()))
    {
        return enumerator;
    }

    const Rectangle& area = featureQuery.area();
    if (area.isUndefined() ||
        !std::isfinite(area.width()) || !std::isfinite(area.height()) ||
        area.width() <= 0.0 || area.height() <= 0.0)
    {
        return enumerator;
    }

    if (m_url.empty())
    {
        return enumerator;
    }

#ifndef __EMSCRIPTEN__
    static bool gdalInitialized = false;
    if (!gdalInitialized)
    {
        GDALAllRegister();
        gdalInitialized = true;
    }

    int width = m_imageWidth;
    int height = m_imageHeight;
    if (featureQuery.resolution() > 0.0)
    {
        width = static_cast<int>(std::round(area.width() / featureQuery.resolution()));
        height = static_cast<int>(std::round(area.height() / featureQuery.resolution()));
    }
    width = std::clamp(width, 1, WmsLayerMaxImageSize);
    height = std::clamp(height, 1, WmsLayerMaxImageSize);

    BMM_DEBUG() << "WMS requested size: " << width << " x " << height << "\n";

    // /vsicurl_streaming/ (rather than /vsicurl/) is used because a WMS GetMap response is
    // generated on the fly and most WMS servers don't support byte-range requests against it;
    // /vsicurl/ assumes a static, range-readable file and fails with "Range downloading not
    // supported" against such servers. The streaming variant just reads the response sequentially.
    std::ostringstream requestUrl;
    requestUrl << "/vsicurl_streaming/" << m_url
                << (m_url.find('?') == std::string::npos ? "?" : "&")
                << "SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap"
                << "&LAYERS=" << m_layers
                << "&STYLES="
                << "&CRS=EPSG:3857"
                << std::fixed << std::setprecision(3)
                << "&BBOX=" << area.xMin() << "," << area.yMin() << "," << area.xMax() << "," << area.yMax()
                << "&WIDTH=" << width
                << "&HEIGHT=" << height
                << "&FORMAT=" << m_format
                << "&TRANSPARENT=TRUE";

    BMM_DEBUG() << "WmsLayer request: " << requestUrl.str() << "\n";
    GDALDataset* ds = static_cast<GDALDataset*>(GDALOpen(requestUrl.str().c_str(), GA_ReadOnly));
    if (!ds)
    {
        BMM_DEBUG() << "WmsLayer: FAILED TO OPEN GDAL DATASET\n";
        return enumerator;
    }

    int imgWidth = ds->GetRasterXSize();
    int imgHeight = ds->GetRasterYSize();
    int channels = ds->GetRasterCount();

    if (imgWidth <= 0 || imgHeight <= 0 || channels == 0)
    {
        BMM_DEBUG() << "WmsLayer: erronous image size\n";
        GDALClose(ds);
        return enumerator;
    }

    int outBands = std::min(channels, 4); // Keep the alpha band if the WMS response has one
    BMM_DEBUG() << "WMS recieved size: " << imgWidth << " x " << imgHeight << " x " << outBands << "\n";
    std::vector<unsigned char> data(static_cast<size_t>(imgWidth) * imgHeight * outBands);

    int bandMap[4] = {1, 2, 3, 4};

    CPLErr err = ds->RasterIO(
        GF_Read,
        0, 0,
        imgWidth, imgHeight,
        data.data(),
        imgWidth, imgHeight,
        GDT_Byte,
        outBands,
        bandMap,
        outBands,
        outBands * imgWidth,
        1
    );

    GDALClose(ds);

    if (err != CE_None)
    {
        BMM_DEBUG() << "WmsLayer: gdal error\n";
        return enumerator;
    }

    auto raster = Raster(data.data(), imgWidth, imgHeight, outBands);
    auto rasterGeometry = std::make_shared<RasterGeometry>(std::move(raster), area);
    auto feature = std::make_shared<Feature>(Id(0, 0), crs, rasterGeometry);

    enumerator->add(feature);
#endif

    return enumerator;
}
