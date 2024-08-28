#include "Layer.h"
#include "Map.h"
#include "DataSet.h"

using namespace BlueMarble;

Layer::Layer()
    : m_enabled(true)
    , m_maxScale(std::numeric_limits<double>::infinity())
    , m_minScale(0)
{
}


void Layer::enabled(bool enabled)
{
    m_enabled = enabled;
}


bool Layer::enabled() const
{
    return m_enabled;
}


void Layer::onUpdateRequest(Map &map, const Rectangle &updateArea, FeatureHandler* /*handler*/)
{
    if (!enabled())
        return;
    if (map.scale() > maxScale())
        return;
    if (map.scale() < minScale())
        return;

    sendUpdateRequest(map, updateArea);
}

void BlueMarble::Layer::onGetFeaturesRequest(const Attributes &attributes, std::vector<FeaturePtr>& features)
{
    if (!enabled())
        return;
    sendGetFeaturesRequest(attributes, features);
}

FeaturePtr BlueMarble::Layer::onGetFeatureRequest(const Id &id)
{
    std::cout << "Layer::onGetFeatureRequest\n";
    if (!enabled())
        return nullptr;

    return sendGetFeatureRequest(id);
}

void Layer::onFeatureInput(Map &map, const std::vector<FeaturePtr> &features)
{
    auto screenFeatures = std::vector<FeaturePtr>();
    toScreen(map, features, screenFeatures);

    assert(features.size() == screenFeatures.size());

    for (size_t i(0); i<screenFeatures.size(); i++)
    {
        auto f = screenFeatures[i];
        switch (f->geometryType())
        {
        case GeometryType::Point:
        //     renderPoint(map, f);
            continue;
            break;
        case GeometryType::Line:
            renderLine(map, f);
            break;
        case GeometryType::Polygon:
            renderPolygon(map, f);
            break;
        // FIXME: should this be done or should we handle multipolygons as single polygons?
        // Presentation objects have to be handle multipolygons as well in that case
        // case GeometryType::MultiPolygon: 
        //     renderMultiPolygon(map, f);
        //     break;
        case GeometryType::Raster:
            renderRaster(map, f);
            break;
        default:
            std::cout << "Layer::onFeatureInput: Unknown GeometryType: " << (int)f->geometryType() << "\n";
        }

        // TODO: temporary. Add features as presentation objects somewhere else?
        map.presentationObjects().emplace_back(PresentationObject(f, features[i]));
    }

    // label organization (points)
    std::vector<FeaturePtr> points;
    std::vector<FeaturePtr> screenPoints;
    for (size_t j(0); j<features.size(); j++)
    {
        auto f = features[j];
        if (f->geometryType() == GeometryType::Point)
        {
            points.push_back(f);
            screenPoints.push_back(screenFeatures[j]);
        }
    }   

    double MIN_DISTANCE = 75;
    for (int i = 0; i<(int)screenPoints.size(); i++)
    {
        auto& p1 = screenPoints[i]->geometryAsPoint()->point();

        bool render = true;
        for (int j=i+1; j<(int)screenPoints.size(); j++)
        {
            auto& p2 = screenPoints[j]->geometryAsPoint()->point();
            if ((p1-p2).length() < MIN_DISTANCE)
            {
                render = false;
                break;
            }
        }

        if (render)
        {
            // TODO: temporary. Add features as presentation objects somewhere else?
            map.presentationObjects().emplace_back(PresentationObject(screenPoints[i], points[i]));
            renderPoint(map, screenPoints[i]);
        }
    }
}


void Layer::renderPoint(Map &map, FeaturePtr feature)
{
    auto geometry = feature->geometryAsPoint();
    auto& point = geometry->point();
    int radius = 6;

    if (feature->attributes().contains("NAME"))
    {
        auto& text = feature->attributes().get<std::string>("NAME");
        map.drawable().drawText(point.x()-radius*4, point.y()-radius*4, text, Color::white(), 16, Color::black(0.6));
    }

    auto backColor = Color(255,255,255,0.5);
    if (map.isSelected(feature))
    {
        radius *= 1.3;
        backColor = Color(255,255,0,0.75);
    }
    else if (map.isHovered(feature))
    {
        radius *= 1.3;
        backColor = Color(255,150,0,0.5);
    }
    map.drawable().drawCircle(point.x(), point.y(), radius, backColor);
    map.drawable().drawCircle(point.x(), point.y(), radius-3, Color(0,0,0));

}


void Layer::renderLine(Map &map, FeaturePtr feature)
{
    auto geometry = feature->geometryAsLine();
    auto input = geometry->points();
    geometry->points().clear();
    Utils::simplifyPoints(input, geometry->points(), 10.0);
    if (geometry->points().size() < 2)
        return;

    bool drawText = false;
    bool drawNodes = false;
    int r=0, g=0, b=255;
    double a=0.25;
    double lineWidth = 1;
    if (map.isSelected(feature))
    {
        r = 255;
        g = 255;
        b = 0;
        a = 0.5;
        lineWidth = 5;
        drawText = true;
        drawNodes = true;
    }
    else if (map.isHovered(feature))
    {
        // std::cout << "Found hovered polygon! Changing visualization\n";
        a = 0.5;
        lineWidth = 5;
        drawText = true;
    }

    if (drawText && feature->attributes().contains("NAME"))
    {
        auto& text = feature->attributes().get<std::string>("NAME");
        auto center = geometry->center();
        map.drawable().drawText(center.x()-20, center.y()-10, text, Color::black(), 16);
    }

    map.drawable().drawLine(geometry->points(), Color(r,g,b,a), lineWidth);
}


void Layer::renderPolygon(Map &map, FeaturePtr feature)
{
    auto geometry = feature->geometryAsPolygon();
    // FIXME: testing simplify points
    // auto input = geometry->points();
    // geometry->points().clear();
    // Utils::simplifyPoints(input, geometry->points(), 10.0);
    if (geometry->points().size() < 3)
        return;

    int r=0, g=0, b=255;
    double a=0.25;

    if (feature->attributes().contains("COLOR_R"))
    {
        r = feature->attributes().get<int>("COLOR_R");
    }
    if (feature->attributes().contains("COLOR_G"))
    {
        g = feature->attributes().get<int>("COLOR_G");
    }
    if (feature->attributes().contains("COLOR_B"))
    {
        b = feature->attributes().get<int>("COLOR_B");
    }
    if (feature->attributes().contains("COLOR_A"))
    {
        a = feature->attributes().get<double>("COLOR_A");
    }

    // TODO: remove this is temporary for handling selected/hovered features
    bool drawText = false;
    bool drawNodes = false;
    if (map.isSelected(feature))
    {
        r = 255;
        g = 255;
        b = 0;
        a = 0.5;
        drawText = true;
        drawNodes = true;
    }
    else if (map.isHovered(feature))
    {
        // std::cout << "Found hovered polygon! Changing visualization\n";
        a = 0.5;
        drawText = true;
    }

    // Draw the polygon
    map.drawable().drawPolygon(geometry->points(), Color(r,g,b,a));

    // Draw bounding line
    auto line = geometry->points();
    line.push_back(geometry->points()[0]); // Add the first point to make a close loop
    double lineWidth = std::max(std::min(map.scale()*3.0, 3.0), 1.0);
    map.drawable().drawLine(line, Color::white(), lineWidth);

    // Draw nodes
    if (drawNodes)
    {
        for (auto& p : geometry->points())
        {
            map.drawable().drawCircle(p.x(), p.y(), 2, Color::red(a));
        }
    }

    if ((map.scale() > 1.5 || drawText) && feature->attributes().contains("NAME"))
    {
        auto& text = feature->attributes().get<std::string>("NAME");
        auto center = geometry->center();
        map.drawable().drawText(center.x()-20, center.y()-10, text, Color::black(), 22);
    }
}


void Layer::renderMultiPolygon(Map& map, FeaturePtr feature)
{
    // FIXME: uggly fix for rendering multipolygons as separate polygons
    for (auto& p : feature->geometryAsMultiPolygon()->polygons())
    {
        auto f = std::make_shared<Feature>
        (
            feature->id(),
            std::make_shared<PolygonGeometry>(p)
        );
        f->attributes() = feature->attributes();
        renderPoint(map, f);
    }
}


void Layer::renderRaster(Map &map, FeaturePtr feature)
{
    auto geometry = feature->geometryAsRaster();
    auto& newImage = geometry->raster();
    auto& offset = geometry->offset();
    map.drawable().drawRaster(offset.x(), offset.y(), newImage, 1.0);
}

void Layer::toScreen(Map& map, const std::vector<FeaturePtr>& features, std::vector<FeaturePtr>& screenFeatures)
{
    for (auto f : features)
    {
        switch (f->geometryType())
        {
        case GeometryType::Point:
        {
            auto& point = f->geometryAsPoint()->point();

            auto screenPoint = map.lngLatToScreen(point).round();
            auto newF = std::make_shared<Feature>
            (
                f->id(),
                std::make_shared<PointGeometry>(screenPoint)
            );
            newF->attributes() = f->attributes();
            screenFeatures.push_back(newF);
            break;
        }

        case GeometryType::Line:
        {

            auto& points = f->geometryAsLine()->points();
            auto screenPoints = map.lngLatToScreen(points);
            auto newF = std::make_shared<Feature>
            (
                f->id(),
                std::make_shared<LineGeometry>(screenPoints)
            );
            newF->attributes() = f->attributes();
            screenFeatures.push_back(newF);
            break;
        }

        case GeometryType::Polygon:
        {

            auto& points = f->geometryAsPolygon()->points();
            auto screenPoints = map.lngLatToScreen(points);
            auto newF = std::make_shared<Feature>
            (
                f->id(),
                std::make_shared<PolygonGeometry>(screenPoints)
            );
            newF->attributes() = f->attributes();
            screenFeatures.push_back(newF);
            break;
        }

        case GeometryType::MultiPolygon:
        {
            auto geometry = std::make_shared<MultiPolygonGeometry>();
            auto newF = std::make_shared<Feature>
            (
                f->id(),
                geometry
            );
            newF->attributes() = f->attributes();
            auto& polygons = f->geometryAsMultiPolygon()->polygons();
            for (auto& p : polygons)
            {
                auto polGeo = PolygonGeometry(map.lngLatToScreen(p.points()));
                geometry->polygons().push_back(polGeo);
            }
            
            screenFeatures.push_back(newF);
            break;
        }

        default:
            // std::cout << "Layer::toScreen() Unhandled geometry type: " << (int)f->geometryType() << "\n";
            screenFeatures.push_back(f);
        }
    }
}
