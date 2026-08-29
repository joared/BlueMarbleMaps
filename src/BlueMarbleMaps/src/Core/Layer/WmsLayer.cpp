#include "BlueMarbleMaps/Core/Layer/WmsLayer.h"
#include "BlueMarbleMaps/Core/Map.h"

#ifndef __EMSCRIPTEN__
#include <cpl_http.h>
#endif

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>


using namespace BlueMarble;

namespace
{
    constexpr int WmsLayerDefaultImageSize = 1024;
    constexpr int WmsLayerMaxImageSize = 2048;
    const std::string WmsLayerJpegFormat = "image/jpeg";
}

WmsLayer::WmsLayer()
    : Layer()
    , m_url()
    , m_layers()
    , m_format("image/png")
    , m_imageWidth(WmsLayerDefaultImageSize)
    , m_imageHeight(WmsLayerDefaultImageSize)
    , m_visualizer(std::make_shared<RasterVisualizer>())
{
    setOpacity(1.0);
}

WmsLayer::~WmsLayer()
{
}

void WmsLayer::setOpacity(double opacity)
{
    m_opacity = opacity;
    m_visualizer->alpha(DirectDoubleAttributeVariable(opacity));
}

FeatureEnumeratorPtr WmsLayer::prepare(const CrsPtr &crs, const FeatureQuery &featureQuery)
{
    return getFeatures(crs, featureQuery, true);
}

void WmsLayer::update(const MapPtr &map, const FeatureEnumeratorPtr &features, const FeatureQuery &featureQuery)
{
    map->drawable()->beginBatches();
    while (features->moveNext())
    {
        m_visualizer->renderFeature(
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

    std::string espg;
    bool flipAxis = false;
    if (crs->isFunctionallyEquivalent(Crs::wgs84MercatorWeb()))
    {
        espg = "3857";
    }
    else if (crs->isFunctionallyEquivalent(Crs::wgs84LngLat()))
    {
        espg = "4326";
        flipAxis = true;
    }
    else
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
    int width = m_imageWidth;
    int height = m_imageHeight;
    if (featureQuery.resolution() > 0.0)
    {
        double reqResolution = featureQuery.resolution();
        double areaWidth = area.width();
        double areaHeight = area.width();
        width = static_cast<int>(std::round(area.width() / reqResolution));
        height = static_cast<int>(std::round(area.height() / reqResolution));

        // double maxRes = 1500.0; // meters per pixel
        // if ((areaWidth / (double)width)*crs->globalMetersPerUnit() > maxRes)
        // {
        //     width = static_cast<int>((areaWidth / maxRes)*crs->globalMetersPerUnit());
        // }
        // if ((areaHeight / (double)height)*crs->globalMetersPerUnit() > maxRes)
        // {
        //     height = static_cast<int>((areaHeight / maxRes)*crs->globalMetersPerUnit());
        // }

        // BMM_DEBUG() << "W resolution: " << areaWidth / width * crs->globalMetersPerUnit() << "\n";
        // BMM_DEBUG() << "H resolution: " << areaHeight / height * crs->globalMetersPerUnit() << "\n";
    }


    width = std::clamp(width, 1, WmsLayerMaxImageSize);
    height = std::clamp(height, 1, WmsLayerMaxImageSize);

    // BMM_DEBUG() << "W resolution: " << area.width() / width * crs->globalMetersPerUnit() << "\n";
    // BMM_DEBUG() << "H resolution: " << area.height() / height * crs->globalMetersPerUnit() << "\n";

    // BMM_DEBUG() << "WMS requested size: " << width << " x " << height << "\n";

    std::ostringstream bboxStr; 
    bboxStr << std::fixed 
            << std::setprecision(std::numeric_limits<long double>::digits10 + 1); // max precision

    if (flipAxis)
    {
        bboxStr << area.yMin() << "," << area.xMin() << "," << area.yMax() << "," << area.xMax();
    }
    else
    {
        bboxStr << area.xMin() << "," << area.yMin() << "," << area.xMax() << "," << area.yMax();
    }

    // A fully opaque layer doesn't need an alpha channel, so request JPEG instead of the
    // (typically PNG) configured format: smaller payload, cheaper to encode server-side and
    // decode client-side. Layers with any transparency keep the configured format.
    bool opaque = (opacity() == 1.0 && transparent() != true);
    const std::string& requestFormat = opaque ? WmsLayerJpegFormat : m_format;

    std::ostringstream requestUrl;
    requestUrl << m_url
                << (m_url.find('?') == std::string::npos ? "?" : "&")
                << "SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap"
                << "&LAYERS=" << m_layers
                << "&STYLES="
                << "&CRS=EPSG:" << espg
                << "&BBOX=" << bboxStr.str()
                << "&WIDTH=" << width
                << "&HEIGHT=" << height
                << "&FORMAT=" << requestFormat
                << m_vendorParams;
    if (!opaque)
    {
        requestUrl << "&TRANSPARENT=TRUE";
    }

    BMM_DEBUG() << "WmsLayer request: " << requestUrl.str() << "\n";

    // Fetch the raw response bytes directly instead of going through GDAL's generic
    // dataset/driver machinery (GDALOpen + RasterIO): that path does format sniffing and a
    // generic band-interleave copy on every request, which is unnecessary overhead for what's
    // just "GET bytes, decode a PNG/JPEG". Decoding reuses the same stb_image path as the rest
    // of this codebase via Raster::decode().
    CPLHTTPResult* result = CPLHTTPFetch(requestUrl.str().c_str(), nullptr);
    if (!result || result->nStatus != 0 || result->nDataLen <= 0)
    {
        BMM_DEBUG() << "WmsLayer: HTTP fetch failed: "
                    << (result && result->pszErrBuf ? result->pszErrBuf : "unknown error") << "\n";
        if (result)
        {
            CPLHTTPDestroyResult(result);
        }
        return enumerator;
    }

    Raster raster;
    try
    {
        raster = Raster::decode(result->pabyData, static_cast<size_t>(result->nDataLen));
    }
    catch (const std::exception& e)
    {
        BMM_DEBUG() << "WmsLayer: failed to decode WMS response: " << e.what() << "\n";
        CPLHTTPDestroyResult(result);
        return enumerator;
    }

    CPLHTTPDestroyResult(result);

    // BMM_DEBUG() << "WMS received size: " << raster.width() << " x " << raster.height() << " x " << raster.channels() << "\n";

    if (raster.width() > WmsLayerMaxImageSize || raster.height() > WmsLayerMaxImageSize)
    {
        BMM_DEBUG() << "WmsLayer: Received a too large image... ignoring\n";
        return enumerator;
    }

    auto rasterGeometry = std::make_shared<RasterGeometry>(std::move(raster), area);
    auto feature = std::make_shared<Feature>(Id(0, 0), crs, rasterGeometry);
    feature->attributes().set("__TILE_LAYER_RENDER_ALPHA", m_opacity);

    enumerator->add(feature);
#endif

    return enumerator;
}
