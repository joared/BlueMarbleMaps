#include "BlueMarbleMaps/Core/Visualizer.h"

using namespace BlueMarble;


Visualizer::Visualizer()
    : m_renderingEnabled(true)
    , m_attachedFeatures()
    , m_sourceFeatures()
    , m_condition([](auto, auto) { return true; })
    , m_antialiasEval([](auto, auto) { return false; })
    , m_colorEval(ColorEvaluation([](auto, auto) { return Color::black(); }))
    , m_sizeEval([](auto, auto) { return 1.0; })
    , m_sizeAddEval([](auto, auto) { return 1.0; })
    , m_textEval([](auto, auto) { return ""; })
    , m_rotationEval([](auto, auto) { return 0.0; })
    , m_offsetXEval([](auto, auto) { return 0.0; })
    , m_offsetYEval([](auto, auto) { return 0.0; })
    , m_offsetZEval([](auto, auto) { return 0.0; })
    , m_lengthUnit(VisualizerLengtUnit::Pixels)
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

const Condition& BlueMarble::Visualizer::condition()
{
    return m_condition;
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

void Visualizer::rotation(const DoubleEvaluation &rotationEval)
{
    m_rotationEval = rotationEval;
}

void Visualizer::generatePresentationObjects(const FeaturePtr& feature, const FeaturePtr& sourceFeature, Attributes& updateAttributes, std::vector<PresentationObject>& presentationObjects)
{
    // Default implementation
    if (!isValidGeometry(feature->geometryType()) || !m_condition(feature, updateAttributes))
        return;

    auto p = PresentationObject(feature, sourceFeature, this, false, -1);
    presentationObjects.push_back(p);
}

PointVisualizer::PointVisualizer()
    : Visualizer()
    , m_isLabelOrganized(false)
    , m_atCenter(false)
    , m_labelOrganizer()
{
}

void PointVisualizer::generatePresentationObjects(const FeaturePtr& feature, const FeaturePtr& sourceFeature, Attributes& updateAttributes, std::vector<PresentationObject>& presentationObjects)
{
    auto cond = condition();
    if (!isValidGeometry(feature->geometryType()) || !cond(feature, updateAttributes))
        return;

    // Convert Polygon and Line features into point features
    auto pointFeatures = std::vector<FeaturePtr>();
    toPointFeature(feature, updateAttributes, pointFeatures);
    assert(pointFeatures.size() > 0);

    for (int nodeIndex=0; nodeIndex<pointFeatures.size(); ++nodeIndex)
    {
        presentationObjects.push_back(PresentationObject(pointFeatures[nodeIndex], sourceFeature, this, false, nodeIndex));
    }
}

void PointVisualizer::renderFeature(Drawable& drawable, const FeaturePtr& feature, Attributes& updateAttributes, const Rectangle& updateArea)
{
    auto cond = condition();
    if (!isValidGeometry(feature->geometryType()) || !cond(feature, updateAttributes))
        return;

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

    // Defer rendering to sub class
    renderPoints(drawable, points, feature, nullptr, updateAttributes);
}

bool PointVisualizer::hitTest(const FeaturePtr &feature, const DrawablePtr &drawable, const Rectangle &area, std::vector<PresentationObject> &outPresentation)
{
    // TODO
    return Visualizer::hitTest(feature, drawable, area, outPresentation);
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
    double radius = m_sizeEval(feature, updateAttributes) / updateAttributes.get<double>(UpdateAttributeKeys::UpdateViewScale);
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

void TextVisualizer::backgroundColor(ColorEvaluation colorEval)
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

void LineVisualizer::generatePresentationObjects(const FeaturePtr& feature, const FeaturePtr& sourceFeature, Attributes& updateAttributes, std::vector<PresentationObject>& presentationObjects)
{
    // Default implementation
    auto cond = condition();
    if (!isValidGeometry(feature->geometryType()) || !cond(feature, updateAttributes))
        return;

    if (feature->geometryType() == GeometryType::Polygon)
    {
        auto poly = feature->geometryAsPolygon();
        if (poly->outerRing().empty())
        {
            return;
        }
        for (auto& line : poly->rings())
        {
            auto lineFeature = std::make_shared<Feature>(
                feature->id(), 
                feature->crs(), 
                std::make_shared<LineGeometry>(line), 
                feature->attributes()
            );

            presentationObjects.push_back(PresentationObject(lineFeature, sourceFeature, this, false, -1)); // TODO add ringIndex to presentationobject
        }
    }
    else
    {
        // Line
        presentationObjects.push_back(PresentationObject(feature, sourceFeature, this, false, -1));
    }
}

bool LineVisualizer::isValidGeometry(GeometryType type)
{
    return (type == GeometryType::Line
            || type == GeometryType::Polygon);
}

Pen LineVisualizer::createPen(const FeaturePtr &feature, Attributes &attributes)
{
    Pen pen;
    pen.setAntiAlias(m_antialiasEval(feature, attributes));
    pen.setColor(m_colorEval(feature, attributes));
    pen.setThickness(m_widthEval(feature, attributes));

    return pen;
}

void LineVisualizer::renderFeature(Drawable& drawable, const FeaturePtr& feature, Attributes& updateAttributes, const Rectangle& updateArea)
{
    auto cond = condition();
    if (!isValidGeometry(feature->geometryType()) || !cond(feature, updateAttributes))
        return;

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
            // TODO: use line geometry with isClosed=true
            ring.push_back(ring[0]); // Make a closed loop
            lines.push_back(ring);
        }
        break;
    default:
        std::cout << "LineVisualizer::renderFeature() Unhandled GeometryType: " << (int)feature->geometryType() << "\n";
        throw std::exception();
    }

    double offsetZ = m_offsetZEval(feature, updateAttributes);
    offsetZ /= feature->crs()->globalMetersPerUnit(); // Meters

    for (auto& line : lines)
    {
        LineGeometryPtr linePtr = std::make_shared<LineGeometry>(line);
        linePtr->move(Point(0,0,offsetZ));
        drawable.drawLine(linePtr, Pen(m_colorEval(feature, updateAttributes), m_widthEval(feature, updateAttributes)));
    }
}

bool LineVisualizer::hitTest(const FeaturePtr &feature, const DrawablePtr &drawable, const Rectangle &area, std::vector<PresentationObject> &outPresentation)
{
    // TODO
    return Visualizer::hitTest(feature, drawable, area, outPresentation);
}

void LineVisualizer::width(DoubleEvaluation widthEval)
{
    m_widthEval = widthEval;
}

PolygonVisualizer::PolygonVisualizer()
    : Visualizer()
{
}

Brush PolygonVisualizer::createBrush(const FeaturePtr &feature, Attributes &attributes)
{
    Brush brush;
    brush.setAntiAlias(m_antialiasEval(feature, attributes));
    brush.setColor(m_colorEval(feature, attributes));

    return brush;
}

bool PolygonVisualizer::isValidGeometry(GeometryType type)
{
    return type == GeometryType::Polygon;
}

void PolygonVisualizer::renderFeature(Drawable& drawable, const FeaturePtr& feature, Attributes& updateAttributes, const Rectangle& updateArea)
{
    auto cond = condition();
    if (!isValidGeometry(feature->geometryType()) || !cond(feature, updateAttributes))
        return;

    auto geometry = feature->geometryAsPolygon();
    
    
    // FIXME: changing the geometry like this will mess with hittest of 
    // other presentation objects that in fact was rendered with 
    // a different geometry. Should probably not be done?
    double scale = m_sizeEval(feature, updateAttributes);
    double rotation = m_rotationEval(feature, updateAttributes);
    double extend = m_sizeAddEval(feature, updateAttributes);
    double offX = m_offsetXEval(feature, updateAttributes);
    double offY = m_offsetYEval(feature, updateAttributes);

    if (scale != 1.0 || extend != 1.0 || rotation != 0.0 || offX != 0.0 || offY != 0.0)
    {
        // Need to clone the geometry if we intent modify it, otherwise hittesting for previous visualizers will mess up
        geometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry->clone());
    }
    auto& polygonPoints = geometry->outerRing();

    double epsilon = 0.000001;
    if (scale != 1.0)
    {
        // TODO: should we do this?
        // if (abs(scale) < epsilon)
        //     scale = epsilon;
        polygonPoints = Utils::scalePoints(polygonPoints, scale);
    }

    if (extend != 1.0)
    {
        // TODO: should we do this?
        // if (abs(extend) < epsilon)
        //     extend = epsilon;
        polygonPoints = Utils::extendPolygon(polygonPoints, extend);
    }

    if (rotation != 0.0)
    {
        polygonPoints = Utils::rotatePoints(polygonPoints, rotation);
    }

    if (offX != 0.0 || offY != 0.0)
    {
        double incScale = 1.0 / updateAttributes.get<double>(UpdateAttributeKeys::UpdateViewScale);
        offX *= incScale;
        offY *= incScale;
        Utils::movePoints(polygonPoints, Point(offX, offY));
    }

    // Draw the polygon
    //geometry->outerRing() = polygonPoints;
    drawable.drawPolygon(geometry, Pen::transparent(), createBrush(feature, updateAttributes));
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

void RasterVisualizer::renderFeature(Drawable& drawable, const FeaturePtr& feature, Attributes& updateAttributes, const Rectangle& updateArea)
{
    auto cond = condition();
    if (!isValidGeometry(feature->geometryType()) || !cond(feature, updateAttributes))
        return;

    auto geometry = feature->geometryAsRaster();
    double alpha = m_alphaEval(feature, updateAttributes);
    auto c = Color::white(alpha);
    drawable.drawRaster(geometry, Brush(std::vector<Color>{c, c, c, c}), updateArea);
}

void RasterVisualizer::alpha(const DoubleEvaluation& alphaEval)
{
    m_alphaEval = alphaEval;
}
