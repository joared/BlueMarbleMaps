#ifndef BLUEMARBLE_PRESENTATIONOBJECT
#define BLUEMARBLE_PRESENTATIONOBJECT

#include "Feature.h"

namespace BlueMarble
{
    /* A presentation object simply acts as a presentation for a certain feature,
    where the attached visualizers determines how the feature is presented.
    The presentation object does not necesarily represent the presentation of the
    original feature, if the feature was created in the operator chain (not from dataset).
    However, the original feature can always be accessed through the "sourceFeature" property.
    In addition, there may be multiple 
    */

    // enum class PresentationObjectType
    // {
    //     Point,
    //     Line,
    //     Polygon,
    //     Raster
    // };

    class Visualizer; // Forward declaration

    class PresentationObject
    {
        public:
            PresentationObject(FeaturePtr feature, FeaturePtr sourceFeature, Visualizer* visualizer);
            bool hitTest(int x, int y, double pointerRadius);
            bool hitTest(const Rectangle& bounds);
            FeaturePtr feature() { return m_feature; }
            FeaturePtr sourceFeature() { return m_sourceFeature; }
            Visualizer* visualizer() { return m_visualizer; };
        private:
            FeaturePtr              m_feature;
            FeaturePtr              m_sourceFeature;
            Visualizer*             m_visualizer;
    };

    bool hitTestPoint(int x, int y, double pointerRadius, PointGeometryPtr geometry);
    bool hitTestLine(int x, int y, double pointerRadius, LineGeometryPtr geometry);
    bool hitTestPolygon(int x, int y, double pointerRadius, PolygonGeometryPtr geometry);
    bool hitTestRaster(int x, int y, double pointerRadius, RasterGeometryPtr geometry);

}

#endif /* BLUEMARBLE_PRESENTATIONOBJECT */
