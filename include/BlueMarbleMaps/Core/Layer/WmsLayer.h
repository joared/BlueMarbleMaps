#ifndef WMSLAYER
#define WMSLAYER

#include "BlueMarbleMaps/Core/Layer/Layer.h"

namespace BlueMarble
{

class WmsLayer : public Layer
{
private:
    std::string m_url;    // Base WMS endpoint, e.g. "https://example.com/geoserver/wms"
    std::string m_layers; // Value of the WMS LAYERS parameter
    std::string m_format; // Value of the WMS FORMAT parameter
    int m_imageWidth;
    int m_imageHeight;
public:
    WmsLayer();
    ~WmsLayer();

    const std::string& url() const { return m_url; }
    void url(const std::string& url) { m_url = url; }

    const std::string& layers() const { return m_layers; }
    void layers(const std::string& layers) { m_layers = layers; }

    const std::string& format() const { return m_format; }
    void format(const std::string& format) { m_format = format; }

    // Requested image size in pixels, used when the query doesn't specify a resolution
    void imageSize(int width, int height) { m_imageWidth = width; m_imageHeight = height; }

    virtual void hitTest(const MapPtr& map, const Rectangle& bounds, std::vector<PresentationObject>& presObjects) override {};
    virtual FeatureEnumeratorPtr prepare(const CrsPtr &crs, const FeatureQuery& featureQuery) override;
    virtual void update(const MapPtr& map, const FeatureEnumeratorPtr& features, const FeatureQuery& featureQuery) override;
    virtual FeatureEnumeratorPtr getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly) override;
    virtual void flushCache() override {};
};

typedef std::shared_ptr<WmsLayer> WmsLayerPtr;

}

#endif /* WMSLAYER */
