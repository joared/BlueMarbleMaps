#include "Core/Visualizer.h"

using namespace BlueMarble;


Visualizer::Visualizer()
    : m_renderingEnabled(true)
    , m_attachedFeatures()
    , m_sourceFeatures()
    , m_condition([](auto, auto) { return true; })
    , m_colorEval(ColorEvaluation([](auto, auto) { return Color::black(); }))
    , m_sizeEval([](auto, auto) { return 1.0; })
    , m_sizeAddEval([](auto, auto) { return 1.0; })
    , m_textEval([](auto, auto) { return ""; })
    , m_rotationEval([](auto, auto) { return 0.0; })
    , m_offsetXEval([](auto, auto) { return 0.0; })
    , m_offsetYEval([](auto, auto) { return 0.0; })
{
}

void Visualizer::renderingEnabled(bool enabled)
{
    m_renderingEnabled = enabled;
}

bool Visualizer::renderingEnabled()
{
    return m_renderingEnabled;
}

void Visualizer::condition(const Condition& condition)
{
    m_condition = condition;
}


void Visualizer::color(const ColorEvaluation& colorEval)
{
    m_colorEval = colorEval;
}

void Visualizer::size(const DoubleEvaluation& sizeEval)
{
    m_sizeEval = sizeEval;
}

void Visualizer::sizeAdd(const DoubleEvaluation& sizeAddEval)
{
    m_sizeAddEval = sizeAddEval;
}

void BlueMarble::Visualizer::rotation(const DoubleEvaluation &rotationEval)
{
    m_rotationEval = rotationEval;
}

bool Visualizer::attachFeature(FeaturePtr feature, FeaturePtr sourceFeature, Attributes& updateAttributes)
{
    if (!isValidGeometry(feature->geometryType()) || !m_condition(feature, updateAttributes))
        return false;
    
    m_attachedFeatures.push_back(feature);
    m_sourceFeatures.push_back(sourceFeature);

    return true;
}


void Visualizer::render(Drawable& drawable, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs)
{
    if (m_attachedFeatures.empty())
    {
        // We have nothing to render. 
    }
    
    preRender(drawable, m_attachedFeatures, m_sourceFeatures, updateAttributes, presObjs);
    assert(m_attachedFeatures.size() == m_sourceFeatures.size());
    for (size_t i(0); i<m_attachedFeatures.size(); i++)
    {
        auto f = m_attachedFeatures[i];
        auto sourceF = m_sourceFeatures[i];
        if (m_renderingEnabled)
            renderFeature(drawable, f, sourceF, updateAttributes, presObjs);
        presObjs.push_back(PresentationObject(f, sourceF, this));
    }
    m_attachedFeatures.clear();
    m_sourceFeatures.clear();
}


PointVisualizer::PointVisualizer()
    : Visualizer()
    , m_isLabelOrganized(false)
    , m_atCenter(false)
    , m_labelOrganizer()
{
}

void BlueMarble::PointVisualizer::preRender(Drawable& drawable, 
                                            std::vector<FeaturePtr>& attachedFeatures, 
                                            std::vector<FeaturePtr>& sourceFeatures, 
                                            Attributes& updateAttributes,
                                            std::vector<PresentationObject>& presObjs)
{
    // Convert Polygon and Line features into point features
    // TODO: should this be done in attachFeature instead?
    auto newAttachedFeatures = std::vector<FeaturePtr>();
    auto newSourceFeatures = std::vector<FeaturePtr>();
    for (size_t i(0); i<attachedFeatures.size(); i++)
    {
        auto f = attachedFeatures[i];
        auto sourceF = sourceFeatures[i];
        
        auto pointFeatures = std::vector<FeaturePtr>();
        toPointFeature(f, updateAttributes, pointFeatures);
        assert(pointFeatures.size() > 0);
        for (auto newF : pointFeatures)
        {
            newAttachedFeatures.push_back(newF);
            newSourceFeatures.push_back(sourceF);
        }
    }

    // Label organization
    if (isLabelOrganized())
    {
        m_labelOrganizer.organize(newAttachedFeatures, newSourceFeatures);
        // TODO: Clip to view area. Need view area...
    }

    attachedFeatures = newAttachedFeatures;
    sourceFeatures = newSourceFeatures;
}

void PointVisualizer::renderFeature(Drawable &drawable, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes, std::vector<PresentationObject> &presObjs)
{
    // TODO: use point geometries and features, needed for label organization for attributes
    // use convertGeometry()
    auto points = std::vector<Point>();
    auto point = feature->geometryAsPoint()->point();
    points.push_back(point);
    
    // Defer rendering to sub class
    renderPoints(drawable, points, feature, source, updateAttributes);
}

bool PointVisualizer::isValidGeometry(GeometryType type)
{
    return (type == GeometryType::Point
            || type == GeometryType::Line
            || type == GeometryType::Polygon);
}

void PointVisualizer::toPointFeature(const FeaturePtr& feature, Attributes& updateAttributes, std::vector<FeaturePtr>& outPointFeatures)
{
    auto points = std::vector<Point>();
    if (atCenter())
    {
        points.push_back(feature->center());
    }
    else
    {
        switch (feature->geometryType())
        {
        case GeometryType::Point:
            points.push_back(feature->geometryAsPoint()->point());
            break;
        case GeometryType::Line:
            points = feature->geometryAsLine()->points();
            break;
        case GeometryType::Polygon:
            for (auto& ring : feature->geometryAsPolygon()->rings())
            {
                for (auto& p : ring)
                    points.push_back(p);
            }
            break;
        default:
            std::cout << "SymbolVisualizer::renderFeature() Unhandled GeometryType: " << (int)feature->geometryType() << "\n";
            throw std::exception();
        }
    }

    for (auto& p : points)
    {
        int offsetX = m_offsetXEval(feature, updateAttributes);
        int offsetY = m_offsetXEval(feature, updateAttributes);
        p += Point(offsetX, offsetY);
        auto geometry = std::make_shared<PointGeometry>(p);
        outPointFeatures.push_back(std::make_shared<Feature>(feature->id(), feature->crs(), geometry, feature->attributes()));
    }
}

SymbolVisualizer::SymbolVisualizer()
    : PointVisualizer()
    , m_symbol()
{
}

void SymbolVisualizer::renderPoints(Drawable& drawable, const std::vector<Point> &points, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes)
{
    double radius = m_sizeEval(feature, updateAttributes);
    Color color = m_colorEval(feature, updateAttributes);
    double rotation = m_rotationEval(feature, updateAttributes);
    for (auto& point : points)
    {
        //drawable.drawCircle(point.x(), point.y(), radius-3, color);
        m_symbol.render(drawable, point, radius, color, rotation);
    }
}


TextVisualizer::TextVisualizer()
    : PointVisualizer()
    , m_textEval([](auto, auto) { return ""; })
    , m_backgroundColorEval([](auto, auto) { return Color::transparent(); })
{
}


void TextVisualizer::text(const StringEvaluation& textEval)
{
    m_textEval = textEval;
}

void BlueMarble::TextVisualizer::backgroundColor(ColorEvaluation colorEval)
{
    m_backgroundColorEval = colorEval;
}

void TextVisualizer::renderPoints(Drawable &drawable, const std::vector<Point> &points, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes)
{
    auto text = m_textEval(feature, updateAttributes);
    if (text.empty())
        return;
    
    auto color = m_colorEval(feature, updateAttributes);
    auto backgroundColor = m_backgroundColorEval(feature, updateAttributes);
    int offX = m_offsetXEval(feature, updateAttributes);
    int offY = m_offsetYEval(feature, updateAttributes);
    double radius = 6;
    for (auto& point : points)
    {
        //drawable.drawText(point.x()-radius*4, point.y()-radius*4, text, Color::white(), 16, Color::black(0.6));
        int fontSize = 22;
        double offsetX = -(double)(fontSize*text.length())*0.25;
        double offsetY = -(double)fontSize*0.5;
        drawable.drawText(point.x()+offsetX+offX, point.y()+offsetY+offY, text, color, 22, backgroundColor);
    }
}

LineVisualizer::LineVisualizer()
    : Visualizer()
    , m_widthEval([](auto, auto) { return 1.0; })
{
}


bool LineVisualizer::isValidGeometry(GeometryType type)
{
    return (type == GeometryType::Line
            || type == GeometryType::Polygon);
}


void LineVisualizer::renderFeature(Drawable& drawable, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs)
{
    auto lines = std::vector<std::vector<Point>>();
    switch (feature->geometryType())
    {
    case GeometryType::Line:
        lines.push_back(feature->geometryAsLine()->points());
        break;
    case GeometryType::Polygon:
        for (auto ring : feature->geometryAsPolygon()->rings())
        {
            // Note: ring as copy!!!
            ring.push_back(ring[0]); // Make a closed loop
            lines.push_back(ring);
        }
        break;
    default:
        std::cout << "LineVisualizer::renderFeature() Unhandled GeometryType: " << (int)feature->geometryType() << "\n";
        throw std::exception();
    }

    for (auto& line : lines)
    {
        drawable.drawLine(line, m_colorEval(feature, updateAttributes), m_widthEval(feature, updateAttributes));
    }
}

void BlueMarble::LineVisualizer::width(DoubleEvaluation widthEval)
{
    m_widthEval = widthEval;
}

PolygonVisualizer::PolygonVisualizer()
    : Visualizer()
{
}

bool PolygonVisualizer::isValidGeometry(GeometryType type)
{
    return type == GeometryType::Polygon;
}

void PolygonVisualizer::renderFeature(Drawable& drawable, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs)
{
    auto geometry = feature->geometryAsPolygon();
    auto polygonPoints = geometry->outerRing(); // Make a copy to prevent affecting hit testing

    // FIXME: changing the geometry like this will mess with hittest of 
    // other presentation objects that in fact was rendered with 
    // a different geometry. Should probably not be done?
    double scale = m_sizeEval(feature, updateAttributes);
    if (scale != 1.0)
    {
        polygonPoints = Utils::scalePoints(polygonPoints, scale);
    }

    double extend = m_sizeAddEval(feature, updateAttributes);
    if (extend != 1.0)
    {
        std::vector<Point> newPoints;
        polygonPoints = Utils::extendPolygon(polygonPoints, extend);
    }

    double rotation = m_rotationEval(feature, updateAttributes);
    if (rotation != 0.0)
    {
        polygonPoints = Utils::rotatePoints(polygonPoints, rotation);
    }

    double offX = m_offsetXEval(feature, updateAttributes);
    double offY = m_offsetYEval(feature, updateAttributes);
    if (offX != 0.0 || offY != 0.0)
    {
        Utils::movePoints(polygonPoints, Point(offX, offY));
    }
    
    // Draw the polygon
    auto color = m_colorEval(feature, updateAttributes);
    drawable.drawPolygon(polygonPoints, color);
    
    return; // TODO: remove, testing new visualization structure

    // TODO: inner rings should be "holes". How do we do that?
    // for (auto innerRing : geometry->innerRings())
    // {
    //     drawable.drawPolygon(innerRing, Color::black(0.5));
    // }
}


RasterVisualizer::RasterVisualizer()
    : Visualizer()
    , m_alphaEval([](auto, auto) { return 1.0; })
{
}

bool RasterVisualizer::isValidGeometry(GeometryType type)
{
    return type == GeometryType::Raster;
}

void RasterVisualizer::renderFeature(Drawable& drawable, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs)
{
    auto geometry = feature->geometryAsRaster();
    // auto& newImage = geometry->raster();
    // auto offset = geometry->bounds().minCorner();
    // drawable.drawRaster(offset.x(), offset.y(), newImage, 1.0);
    double alpha = m_alphaEval(feature, updateAttributes);
    drawable.drawRaster(geometry, alpha);
}

void RasterVisualizer::alpha(const DoubleEvaluation& alphaEval)
{
    m_alphaEval = alphaEval;
}
