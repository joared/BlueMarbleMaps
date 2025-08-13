#include "BlueMarbleMaps/Core/Layer.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/DataSet.h"
#define _USE_MATH_DEFINES
#include <math.h>
using namespace BlueMarble;

Layer::Layer(bool createdefaultVisualizers)
    : m_enabled(true)
    , m_selectable(true) // TODO: use this for something
    , m_enabledDuringQuickUpdates(true)
    , m_maxScale(std::numeric_limits<double>::infinity())
    , m_minScale(0)
    , m_effects()
    , m_drawable(nullptr)
{
    // TODO: remove, this is a temporary solution
    if (createdefaultVisualizers)
        createDefaultVisualizers();
}


void Layer::enabled(bool enabled)
{
    m_enabled = enabled;
}


bool Layer::enabled() const
{
    return m_enabled;
}

void Layer::selectable(bool selectable)
{
    m_selectable = selectable;
}

bool Layer::selectable()
{
    return m_selectable;
}

void Layer::onUpdateRequest(Map &map, const Rectangle &updateArea, FeatureHandler* /*handler*/)
{
    if (!enabled())
        return;
    if (map.scale() > maxScale())
        return;
    if (map.scale() < minScale())
        return;
    if (map.quickUpdateEnabled() && !enabledDuringQuickUpdates())
        return;

    sendUpdateRequest(map, updateArea);
}

void Layer::onGetFeaturesRequest(const Attributes &attributes, std::vector<FeaturePtr>& features)
{
    if (!enabled())
        return;
    sendGetFeaturesRequest(attributes, features);
}

FeaturePtr Layer::onGetFeatureRequest(const Id &id)
{
    std::cout << "Layer::onGetFeatureRequest\n";
    if (!enabled())
        return nullptr;

    return sendGetFeatureRequest(id);
}


void Layer::onFeatureInput(Map& map, const std::vector<FeaturePtr>& features)
{
    for (size_t i(0); i< features.size(); i++)
    {
        auto sourceFeature = features[i];

        FeaturePtr f;
        
        switch (sourceFeature->geometryType())
        {
        case GeometryType::Raster:
            f = sourceFeature;
            break;
        
        case GeometryType::Polygon:
            f = std::make_shared<Feature>(
                sourceFeature->id(),
                sourceFeature->crs(),
                std::static_pointer_cast<Geometry>(sourceFeature->geometry()->deepClone())
            );
            //f->geometryAsPolygon()->outerRing() = map.lngLatToMap(f->geometryAsPolygon()->outerRing());
            //f->projectTo(map.getCrs());
            break;

        default:
            f = std::make_shared<Feature>(
                sourceFeature->id(),
                sourceFeature->crs(),
                std::static_pointer_cast<Geometry>(sourceFeature->geometry()->deepClone())
            );
            break;
        }
        
        // Always apply standard visualization for now
        for (auto vis : m_visualizers)
        {
            vis->attachFeature(f, sourceFeature, map.updateAttributes());
        }
        if (map.isSelected(sourceFeature))
        {
            for (auto sVis : m_selectionVisualizers)
                sVis->attachFeature(f, sourceFeature, map.updateAttributes());
        }
        else if (map.isHovered(sourceFeature))
        {
            for (auto hVis : m_hoverVisualizers)
                hVis->attachFeature(f, sourceFeature, map.updateAttributes());
        }
    }

    auto drawable = map.drawable();
    if (!m_effects.empty())
    {
        // TODO: Not implemented. m_drawable is null
        if (map.drawable()->width() != m_drawable->width() ||
            map.drawable()->height() != m_drawable->height())
        {
            // Resize
            m_drawable->resize(map.drawable()->width(), map.drawable()->height());
        }
        m_drawable->fill(0);
        drawable = m_drawable;
    }
    // TODO: Selection and hover visualizer should render after other layers normal visualizers
    for (auto vis : m_visualizers)
        vis->render(*drawable, map.updateAttributes(), map.presentationObjects());
    for (auto hVis : m_hoverVisualizers)
        hVis->render(*drawable, map.updateAttributes(), map.presentationObjects());
    for (auto sVis : m_selectionVisualizers)
        sVis->render(*drawable, map.updateAttributes(), map.presentationObjects());

    for (const auto& e : m_effects)
    {
        e->apply(*map.drawable());
    }
}

void Layer::createDefaultVisualizers()
{
    auto animatedDouble = [](FeaturePtr feature, Attributes& updateAttributes) 
    { 
        static auto from = DirectDoubleAttributeVariable(0.0);
        static auto to = DirectDoubleAttributeVariable(1.0);
        static auto value = AnimatedDoubleAttributeVariable(
            from, 
            to, 
            700,
            EasingFunctionType::Linear);
        
        return value(feature, updateAttributes);
    };

    // Standard visualizers
    auto colorEval = [](FeaturePtr f, Attributes&)
    {
        int r=0, g=0, b=255;
        double a=0.1;

        if (f->attributes().contains("COLOR_R"))
        {
            r = f->attributes().get<int>("COLOR_R");
        }
        if (f->attributes().contains("COLOR_G"))
        {
            g = f->attributes().get<int>("COLOR_G");
        }
        if (f->attributes().contains("COLOR_B"))
        {
            b = f->attributes().get<int>("COLOR_B");
        }
        if (f->attributes().contains("COLOR_A"))
        {
            a = f->attributes().get<double>("COLOR_A");
        }

        return Color(r,g,b,a);
    };

    ColorEvaluation colorEvalHover = [colorEval, animatedDouble](FeaturePtr f, Attributes& updateAttributes) -> Color
    {
        Color color = colorEval(f, updateAttributes);
        double alpha = 0.5*animatedDouble(f, updateAttributes);
        return Color(color.r(), color.g(), color.b(), alpha);
    };

    ColorEvaluation colorEvalSelect = [colorEval, animatedDouble](FeaturePtr f, Attributes& updateAttributes)
    {
        auto toColor = Color(255, 255, 0, 0.5);
        Color fromColor = colorEval(f, updateAttributes);
        double progress = animatedDouble(f, updateAttributes);
        double alpha = 0.5*progress;

        double red = fromColor.r() + (double)(toColor.r()-fromColor.r())*progress;
        double green = fromColor.g() + (double)(toColor.g()-fromColor.g())*progress;
        double blue = fromColor.b() + (double)(toColor.b()-fromColor.b())*progress;
        auto color = Color(red, green, blue, alpha);
        // std::cout << "From: " <<  fromColor.toString() << "\n";
        // std::cout << "Result: " << color.toString() << "\n";

        return color;
    };

    // Point visualizers
    auto pointVis = std::make_shared<SymbolVisualizer>();
    pointVis->condition([](FeaturePtr f, auto) { return f->geometryType() == GeometryType::Point; });
    pointVis->isLabelOrganized(true);
    
    // Node visualizers
    auto nodeVis = std::make_shared<SymbolVisualizer>();
    nodeVis->condition([](FeaturePtr f, auto) { return f->geometryType() == GeometryType::Polygon; });
    nodeVis->color(DirectColorAttributeVariable(Color(255, 175, 60)));
    nodeVis->size(DirectDoubleAttributeVariable(4.0));

    // Line visualizer
    auto lineVis = std::make_shared<LineVisualizer>();
    lineVis->color(ColorEvaluation([](FeaturePtr, Attributes&) { return Color(50,50,50,0.25); }));
    lineVis->width([](FeaturePtr, Attributes&) -> double { return 10.0; });

    // Polygon visualizer
    auto polVis = std::make_shared<PolygonVisualizer>();
    polVis->color(ColorEvaluation(colorEval));

    // Raster visualizer
    auto rasterVis = std::make_shared<RasterVisualizer>();

    // Text visualizer
    auto textVis = std::make_shared<TextVisualizer>();
    textVis->atCenter(true);
    textVis->isLabelOrganized(true);
    textVis->text(
        [](FeaturePtr f, auto) 
        { 
            if(f->attributes().contains("NAME"))
                return f->attributes().get<std::string>("NAME");
            else if(f->attributes().contains("name"))
                return f->attributes().get<std::string>("name");
            else if(f->attributes().contains("CONTINENT"))
                return f->attributes().get<std::string>("CONTINENT");
            return std::string();
        }
    );

    m_visualizers.push_back(polVis);
    m_visualizers.push_back(pointVis);
    m_visualizers.push_back(lineVis);
    m_visualizers.push_back(nodeVis);
    m_visualizers.push_back(rasterVis);
    m_visualizers.push_back(textVis);

    // Line visualizer
    auto lineVisHover = std::make_shared<LineVisualizer>();
    lineVisHover->color([](auto, auto) { return Color(50,50,50,0.25); });
    lineVisHover->width(DirectDoubleAttributeVariable(3.0)); //[](auto, auto) { return 3.0; });

    auto pointVisHover = std::make_shared<SymbolVisualizer>();
    auto polVisHover = std::make_shared<PolygonVisualizer>();
    polVisHover->color(colorEvalHover);
    polVisHover->size([=](FeaturePtr f, Attributes& u) { return 1.0*animatedDouble(f,u); }); // This will ensure small polygons are visible
    // FIXME: messing with size might not be a good idea. See PolygonVisualizer::renderFeature()
    //polVisHover->size([](auto) { return 1.2; }); // This will ensure small polygons are visible
    auto rasterVisHover = std::make_shared<RasterVisualizer>();


    auto textVisHover = std::make_shared<TextVisualizer>(*textVis);
    // textVisHover->color([](FeaturePtr, Attributes& updateAttributes) { 
    textVisHover->color(ColorEvaluation([=](FeaturePtr feature, Attributes& updateAttributes) 
        { 
            return Color::white(animatedDouble(feature, updateAttributes));
        }));
    //textVisHover->backgroundColor([](auto) { return Color::black(0.5); });
    textVisHover->offsetX( [](auto, auto) {return -3.0; });
    textVisHover->offsetY( [](auto, auto) {return -3.0; });

    // m_hoverVisualizers.push_back(pointVis);
    
    m_hoverVisualizers.push_back(polVisHover);
    m_hoverVisualizers.push_back(lineVisHover);
    // m_hoverVisualizers.push_back(rasterVis);
    m_hoverVisualizers.push_back(textVisHover);

    
    auto polVisSelect = std::make_shared<PolygonVisualizer>();
    polVisSelect->color(colorEvalSelect);
    polVisSelect->size([=](FeaturePtr f, Attributes& u) { return 1.0*animatedDouble(f,u); });
    polVisSelect->rotation([=](FeaturePtr f, Attributes& u) { return 2.0*M_PI*animatedDouble(f,u); });
    polVisSelect->offsetX([=](FeaturePtr f, Attributes& u) { return 500.0*(1.0-animatedDouble(f,u)); });
    polVisSelect->offsetY([=](FeaturePtr f, Attributes& u) { return 500.0*(1.0-animatedDouble(f,u)); });
    
    auto textVisSelect = std::make_shared<TextVisualizer>(*textVisHover);

    m_selectionVisualizers.push_back(polVisSelect);
    m_selectionVisualizers.push_back(textVisSelect);
}
