#ifndef WMSLAYER
#define WMSLAYER

#include "BlueMarbleMaps/Core/Layer/Layer.h"

namespace BlueMarble
{

class WmsLayer : public Layer
{
private:
    std::string m_url;          // Base WMS endpoint, e.g. "https://example.com/geoserver/wms"
    std::string m_layers;       // Value of the WMS LAYERS parameter
    std::string m_format;       // Value of the WMS FORMAT parameter
    bool        m_transparent;  // Value of the WMS TRANSPARENT parameter
    std::string m_vendorParams; // Vendor specific parameters, separated by &
    int m_imageWidth;
    int m_imageHeight;
    double m_opacity;
    RasterVisualizerPtr m_visualizer;
public:
    WmsLayer();
    ~WmsLayer();

    // TODO: add to Layer baseclass
    double opacity() { return m_opacity; }
    void setOpacity(double opacity);

    bool transparent() const { return m_transparent; };
    void transparent(bool transparent) { m_transparent = transparent; }

    const std::string& url() const { return m_url; }
    void url(const std::string& url) { m_url = url; }

    const std::string& layers() const { return m_layers; }
    void layers(const std::string& layers) { m_layers = layers; }

    const std::string& format() const { return m_format; }
    void format(const std::string& format) { m_format = format; }

    // Requested image size in pixels, used when the query doesn't specify a resolution
    void imageSize(int width, int height) { m_imageWidth = width; m_imageHeight = height; }

    const std::string& vendorParameters() const { return m_vendorParams; }
    void setVendorParameters(const std::string& params) { m_vendorParams = params; }

    virtual void hitTest(const MapPtr& map, const Rectangle& bounds, std::vector<PresentationObject>& presObjects) override {};
    virtual FeatureEnumeratorPtr prepare(const CrsPtr &crs, const FeatureQuery& featureQuery) override;
    virtual void update(const MapPtr& map, const FeatureEnumeratorPtr& features, const FeatureQuery& featureQuery) override;
    virtual FeatureEnumeratorPtr getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly) override;
    virtual void flushCache() override {};
};

typedef std::shared_ptr<WmsLayer> WmsLayerPtr;

}

#endif /* WMSLAYER */
