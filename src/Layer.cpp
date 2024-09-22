#include "Layer.h"
#include "Map.h"
#include "DataSet.h"

using namespace BlueMarble;

Layer::Layer(bool createdefaultVisualizers)
    : m_enabled(true)
    , m_enabledDuringQuickUpdates(true)
    , m_maxScale(std::numeric_limits<double>::infinity())
    , m_minScale(0)
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
    auto screenFeatures = std::vector<FeaturePtr>();
    toScreen(map, features, screenFeatures);

    assert(features.size() == screenFeatures.size());

    for (size_t i(0); i<screenFeatures.size(); i++)
    {
        auto f = screenFeatures[i];
        auto sourceFeature = features[i];

        // Always apply standard visualization for now
        for (auto vis : m_visualizers)
            vis->attachFeature(f, sourceFeature, map.updateAttributes());

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

    // TODO: Selection and hover visualizer should render after other layers normal visualizers
    for (auto vis : m_visualizers)
        vis->render(map.drawable(), map.updateAttributes(), map.presentationObjects());
    for (auto hVis : m_hoverVisualizers)
        hVis->render(map.drawable(), map.updateAttributes(), map.presentationObjects());
    for (auto sVis : m_selectionVisualizers)
        sVis->render(map.drawable(), map.updateAttributes(), map.presentationObjects());
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

            auto screenRings = std::vector<std::vector<Point>>();
            for (auto& innerRing : f->geometryAsPolygon()->rings())
            {
                screenRings.push_back(map.lngLatToScreen(innerRing));
            }

            auto newF = std::make_shared<Feature>
            (
                f->id(),
                std::make_shared<PolygonGeometry>(screenRings)
            );
            newF->attributes() = f->attributes();
            screenFeatures.push_back(newF);
            break;
        }

        case GeometryType::MultiPolygon:
        {
            // auto geometry = std::make_shared<MultiPolygonGeometry>();
            // auto newF = std::make_shared<Feature>
            // (
            //     f->id(),
            //     geometry
            // );
            // newF->attributes() = f->attributes();
            // auto& polygons = f->geometryAsMultiPolygon()->polygons();
            // for (auto p : polygons)
            // {
            //     auto polGeo = PolygonGeometry(map.lngLatToScreen(p.points()));
            //     geometry->polygons().push_back(polGeo);
            // }
            
            // screenFeatures.push_back(newF);
            std::cout << "WARNING: Layer::toScreen() MultiPolygon not supported.\n";
            break;
        }

        case GeometryType::Raster:
            screenFeatures.push_back(f);
            break;
        default:
            std::cout << "Layer::toScreen() Unhandled geometry type: " << (int)f->geometryType() << "\n";
        }
    }
}


void Layer::createDefaultVisualizers()
{
    auto animatedDouble = [](FeaturePtr feature, Attributes& updateAttributes) 
    { 
        // // FIXME: What is bad about this is that other animated evaluations needs to store the features also.
        // // Better to use a global manager/feature state factory
        static std::map<Id, int> featureStartTimes;
        static Id prevId = Id(0,0); // TEMP fix
        static int prevTimeStamp = 0; // = updateAttributes.get<int>(UpdateAttributeKeys::UpdateTimeMs);
        static std::map<Id, int>  prevHandledFeatures;
        // static double progress;

        constexpr int duration = 150;


        int timeStamp = updateAttributes.get<int>(UpdateAttributeKeys::UpdateTimeMs);
        
        if (timeStamp != prevTimeStamp)
        {
            if (featureStartTimes.empty())
                std::cout << "New update with empty ids!\n";
            else
            {
                prevHandledFeatures = featureStartTimes;
                featureStartTimes.clear();
                prevTimeStamp = timeStamp;
            }
        }
        
        auto it2 = featureStartTimes.find(feature->id());
        auto it = prevHandledFeatures.find(feature->id());
        if (it == prevHandledFeatures.end() && it2 == featureStartTimes.end())
        {
            featureStartTimes[feature->id()] = timeStamp;
            // We need updates
            updateAttributes.set(UpdateAttributeKeys::UpdateRequired, true);
            
            std::cout << "Animation started for: " << "(" << feature->id().dataSetId() << ", " << feature->id().featureId() << "\n";

            return 0.0;
        }

        // Feature is being animated!
        int startTime = it->second;

        int elapsed = timeStamp - startTime;
        //std::cout << "Elapsed: " << elapsed << "\n";

        if (elapsed > duration)
        {
            
            std::cout << "Animation finished for: " << "(" << feature->id().dataSetId() << ", " << feature->id().featureId() << "\n";
            
            return 1.0;
        }

        updateAttributes.set(UpdateAttributeKeys::UpdateRequired, true);
        
        double progress = (double)elapsed/(double)duration;
        //std::cout << "Animation progress: " << progress << "\n";

        
        
        return progress;
    };

    // Standard visualizers
    ColorEvaluation colorEval = [](FeaturePtr f, auto)
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

    ColorEvaluation colorEvalHover = [=](auto f, auto updateAttributes)
    {
        auto color = colorEval(f, updateAttributes);
        double alpha = 0.5*animatedDouble(f, updateAttributes);
        return Color(color.r(), color.g(), color.b(), alpha);
    };

    ColorEvaluation colorEvalSelect = [](auto, auto)
    {
        return Color(255, 255, 0, 0.5);
    };

    // Point visualizers
    auto pointVis = std::make_shared<SymbolVisualizer>();
    pointVis->condition([](FeaturePtr f, auto) { return f->geometryType() == GeometryType::Point; });
    pointVis->isLabelOrganized(true);
    
    // Line visualizer
    auto lineVis = std::make_shared<LineVisualizer>();
    lineVis->color([](auto, auto) { return Color(50,50,50,0.5); });
    lineVis->width([](auto, auto) { return 1; });

    // Polygon visualizer
    auto polVis = std::make_shared<PolygonVisualizer>();
    polVis->color(colorEval);

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
    // textVis->color([](FeaturePtr, Attributes& updateAttributes) { 
    //     // TODO: add animation
    //     static bool reStart = false;
    //     static bool prevState = false;
    //     static int startTime = updateAttributes.get<int>("_timeMs");
    //     static bool update = false;
    //     static double alpha = 1.0;

    //     constexpr int duration = 500;

    //     bool currState = updateAttributes.get<bool>("_hover");
    //     if (currState == prevState)
    //         reStart = false;
    //     else
    //     {
    //         reStart = currState == true;
    //         prevState = currState;
    //     }

    //     if (reStart)
    //     {
    //         std::cout << "Animation started!\n";
    //         update = true;
    //         startTime = updateAttributes.get<int>("_timeMs");
    //     }

    //     if (update)
    //     {
    //         int elapsed = updateAttributes.get<int>("_timeMs")- startTime;
    //         if (elapsed > duration)
    //         {
    //             std::cout << "Animation finished!\n";
    //             alpha = 1.0;
    //             update = false;
    //         }
    //         else
    //         {
    //             alpha = (double)elapsed/(double)duration;
    //         }
    //         updateAttributes.set("updateRequired_", true);
    //     }
            
    //     return Color::black(alpha);
    // });

    m_visualizers.push_back(polVis);
    m_visualizers.push_back(pointVis);
    m_visualizers.push_back(lineVis);
    m_visualizers.push_back(rasterVis);
    m_visualizers.push_back(textVis);

    // Line visualizer
    auto lineVisHover = std::make_shared<LineVisualizer>();
    lineVisHover->color([](auto, auto) { return Color::white(); });
    lineVisHover->width([](auto, auto) { return 3.0; });

    auto pointVisHover = std::make_shared<SymbolVisualizer>();
    auto polVisHover = std::make_shared<PolygonVisualizer>();
    polVisHover->color(colorEvalHover);
    polVisHover->size([=](FeaturePtr f, Attributes& u) { return 1.0*animatedDouble(f,u); }); // This will ensure small polygons are visible
    // FIXME: messing with size might not be a good idea. See PolygonVisualizer::renderFeature()
    //polVisHover->size([](auto) { return 1.2; }); // This will ensure small polygons are visible
    auto rasterVisHover = std::make_shared<RasterVisualizer>();


    auto textVisHover = std::make_shared<TextVisualizer>(*textVis);
    // textVisHover->color([](FeaturePtr, Attributes& updateAttributes) { 
    textVisHover->color([=](FeaturePtr feature, Attributes& updateAttributes) { 
            return Color::white(animatedDouble(feature, updateAttributes));
        });
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
    
    auto textVisSelect = std::make_shared<TextVisualizer>(*textVisHover);

    m_selectionVisualizers.push_back(polVisSelect);
    m_selectionVisualizers.push_back(textVisSelect);
}
