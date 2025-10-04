#include "BlueMarbleMaps/Core/Layer.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/DataSets/DataSet.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace BlueMarble;

Layer::Layer(bool createdefaultVisualizers)
    : m_enabled(true)
    , m_selectable(true) // TODO: use this for something
    , m_enabledDuringQuickUpdates(true)
    , m_maxScale(std::numeric_limits<double>::infinity())
    , m_minScale(0)
    , m_renderingEnabled(true)
    , m_effects()
    , m_drawable(nullptr)
{
    m_presentationObjects.reserve(100000);
    m_presentationObjectsHover.reserve(1);
    m_presentationObjectsSelection.reserve(1);
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

void Layer::update(const MapPtr& map, const CrsPtr &crs, const FeatureQuery &featureQuery)
{
    std::vector<FeaturePtr> hoveredFeatures;
    std::vector<FeaturePtr> selectedFeatures;
    auto features = getFeatures(crs, featureQuery, true);

    bool hasAddedHoverAnSelection = false;
    
    for (const auto& vis : m_visualizers)
    {
        features->reset();

        while (features->moveNext())
        {
            
            const auto& f = features->current();

            // if (m_cache[vis].has(f->id()))
            // {
            //     map->drawable()->drawDrawCalls(m_cache[vis].get(f->id()));
            // }
            // else
            // {
            //     map->drawable()->record();
            //     vis->renderFeature(*map->drawable(), f, map->updateAttributes());
            //     auto drawCalls = map->drawable()->stopRecording();
            //     m_cache[vis].set(f->id(), drawCalls);
            // }
            if (m_renderingEnabled)
                vis->renderFeature(*map->drawable(), f, map->updateAttributes());

            if (!hasAddedHoverAnSelection)
            {
                if (map->isSelected(f))
                {
                    selectedFeatures.push_back(f);
                }
                else if (map->isHovered(f))
                {
                    hoveredFeatures.push_back(f);
                }
            }
        }
        hasAddedHoverAnSelection = true;
    }
    

    for (const auto& vis : m_hoverVisualizers)
    {
        for (const auto& f : hoveredFeatures)
        {
            if (m_renderingEnabled)
                vis->renderFeature(*map->drawable(), f, map->updateAttributes());
        }
    }
    for (const auto& vis : m_selectionVisualizers)
    {
        for (const auto& f : selectedFeatures)
        {
            if (m_renderingEnabled)
                vis->renderFeature(*map->drawable(), f, map->updateAttributes());
        }
    }

    // m_presentationObjects.clear();
    // m_presentationObjectsHover.clear();
    // m_presentationObjectsSelection.clear();

    // auto features = getFeatures(crs, featureQuery, true);
    // while (features->moveNext())
    // {
    //     auto sourceFeature = features->current();
    //     auto f = sourceFeature; // NOTE: there is currently no difference between source and current

    //     // Generate all presentation objects
    //     for (auto vis : m_visualizers)
    //     {
    //         vis->generatePresentationObjects(f, sourceFeature, map->updateAttributes(), m_presentationObjects);
    //     }
    //     if (map->isSelected(sourceFeature))
    //     {
    //         for (const auto& sVis : m_selectionVisualizers)
    //             sVis->generatePresentationObjects(f, sourceFeature, map->updateAttributes(), m_presentationObjectsHover);
    //     }
    //     else if (map->isHovered(sourceFeature))
    //     {
    //         for (const auto& hVis : m_hoverVisualizers)
    //             hVis->generatePresentationObjects(f, sourceFeature, map->updateAttributes(), m_presentationObjectsSelection);
    //     }
    // }

    // // Render
    // auto drawable = map->drawable();

    // // TODO: we should iterate featres, not presentationobjects. Probably inefficient for rendering in the future
    // for (auto& p : m_presentationObjects)
    // {
    //     map->presentationObjects().push_back(p);
    //     p.visualizer()->renderFeature(*drawable, p.feature(), map->updateAttributes());
    // }

    // for (auto& p : m_presentationObjectsHover)
    // {
    //     map->presentationObjects().push_back(p);
    //     p.visualizer()->renderFeature(*drawable, p.feature(), map->updateAttributes());
    // }

    // for (auto& p : m_presentationObjectsSelection)
    // {
    //     map->presentationObjects().push_back(p);
    //     p.visualizer()->renderFeature(*drawable, p.feature(), map->updateAttributes());
    // }
}

FeatureEnumeratorPtr Layer::getFeatures(const CrsPtr &crs, const FeatureQuery& featureQuery, bool activeLayersOnly)
{
    auto features = std::make_shared<FeatureEnumerator>();
    if (activeLayersOnly)
    {
        if (featureQuery.scale() > maxScale())
            return features;
        if (featureQuery.scale() < minScale())
            return features;
        // if (featureQuery.quickUpdateEnabled() && !enabledDuringQuickUpdates())
        //     return features;
        if (!enabled())
        {
            return features;
        }
    }
    
    for (const auto& d : m_dataSets)
    {
        auto dataSetFeatures = d->getFeatures(featureQuery);
        features->addEnumerator(dataSetFeatures);
    }

    return features;
}


FeaturePtr Layer::getFeature(const Id& id)
{
    for (const auto& d : m_dataSets)
    {
        if (auto feature = d->getFeature(id))
        {
            return feature;
        }
    }

    // Not found
    return FeaturePtr();
}


void Layer::addDataSet(const DataSetPtr &dataSet)
{
    return m_dataSets.push_back(dataSet);
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
        auto toColor = Color(250, 134, 196, 0.75);
        Color fromColor = colorEval(f, updateAttributes);
        double progress = animatedDouble(f, updateAttributes);

        double red = fromColor.r() + (double)(toColor.r()-fromColor.r())*progress;
        double green = fromColor.g() + (double)(toColor.g()-fromColor.g())*progress;
        double blue = fromColor.b() + (double)(toColor.b()-fromColor.b())*progress;
        double alpha = fromColor.a() + (double)(toColor.a()-fromColor.a())*progress;
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
    lineVis->color(ColorEvaluation([](FeaturePtr, Attributes&) { return Color(50,50,50,1.0); }));
    lineVis->width([](FeaturePtr, Attributes&) -> double { return 3.0; });

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

    m_visualizers.push_back(rasterVis);
    //m_visualizers.push_back(polVis);
    //m_visualizers.push_back(pointVis);
    m_visualizers.push_back(lineVis);
    //m_visualizers.push_back(nodeVis);
    
    //m_visualizers.push_back(textVis);

    // Line visualizer
    auto lineVisHover = std::make_shared<LineVisualizer>();
    lineVisHover->color([](auto, auto) { return Color(255,255,255,1.0); });
    lineVisHover->width(DirectDoubleAttributeVariable(3.0)); //[](auto, auto) { return 3.0; });

    auto pointVisHover = std::make_shared<SymbolVisualizer>();
    auto polVisHover = std::make_shared<PolygonVisualizer>();
    polVisHover->color(colorEvalHover);
    polVisHover->size([=](FeaturePtr f, Attributes& u) { return 1.2*animatedDouble(f,u); }); // This will ensure small polygons are visible
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
    
    //m_hoverVisualizers.push_back(polVisHover);
    //m_hoverVisualizers.push_back(lineVisHover);
    // m_hoverVisualizers.push_back(rasterVis);
    //m_hoverVisualizers.push_back(nodeVis);
    //m_hoverVisualizers.push_back(textVisHover);

    auto polVisSelectShadow = std::make_shared<PolygonVisualizer>();
    polVisSelectShadow->color(ColorEvaluation([](FeaturePtr, Attributes&) { return Color(50,50,50,0.5); }));
    polVisSelectShadow->size([=](FeaturePtr f, Attributes& u) { return 1.0*animatedDouble(f,u); });
    polVisSelectShadow->offsetX([=](FeaturePtr f, Attributes& u) { return -10.0*(animatedDouble(f,u)); });
    polVisSelectShadow->offsetY([=](FeaturePtr f, Attributes& u) { return -10.0*(animatedDouble(f,u)); });

    auto polVisSelect = std::make_shared<PolygonVisualizer>();
    polVisSelect->color(colorEvalSelect);
    polVisSelect->size([=](FeaturePtr f, Attributes& u) { return 1.0*animatedDouble(f,u); });
    polVisSelect->rotation([=](FeaturePtr f, Attributes& u) { return 2.0*M_PI*animatedDouble(f,u); });
    //polVisSelect->offsetX([=](FeaturePtr f, Attributes& u) { return 1.0*(1.0-animatedDouble(f,u)); });
    //polVisSelect->offsetY([=](FeaturePtr f, Attributes& u) { return 1.0*(1.0-animatedDouble(f,u)); });

        // Line visualizer
    auto lineVisSelect = std::make_shared<LineVisualizer>();
    lineVisSelect->color([](auto, auto) { return Color(255,255,0,1.0); });
    lineVisSelect->width(DirectDoubleAttributeVariable(3.0)); //[](auto, auto) { return 3.0; });
    
    auto textVisSelect = std::make_shared<TextVisualizer>(*textVisHover);

    //m_selectionVisualizers.push_back(polVisSelectShadow);
    //m_selectionVisualizers.push_back(polVisSelect);
    m_selectionVisualizers.push_back(lineVisSelect);
    //m_selectionVisualizers.push_back(nodeVis);
    //m_selectionVisualizers.push_back(textVisSelect);
}
