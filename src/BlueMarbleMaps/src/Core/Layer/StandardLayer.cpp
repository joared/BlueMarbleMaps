#include "BlueMarbleMaps/Core/Layer/StandardLayer.h"
#include "BlueMarbleMaps/Core/Map.h"

#include <chrono>
#include <future>


using namespace BlueMarble;

StandardLayer::StandardLayer(bool createDefaultVisualizerz)
    : Layer()
    , m_visualizers()
    , m_hoverVisualizers()
    , m_selectionVisualizers()
    , m_effects()
    , m_dataSets()
    , m_cache(std::make_shared<FIFOCache>())
    , m_readAsync(false)
    , m_doRead(false)
    , m_stop(false)
    , m_queriedFeatures(std::make_shared<FeatureEnumerator>())
{
    // TODO: remove, this is a temporary solution
    if (createDefaultVisualizerz)
        createDefaultVisualizers();
}

StandardLayer::~StandardLayer()
{
    stopBackgroundReadingThread();
}

void StandardLayer::asyncRead(bool async)
{
    if (m_readAsync && !async)
    {
        stopBackgroundReadingThread();
    }
    else if (!m_readAsync && async)
    {
        startbackgroundReadingThread();
    }

    m_readAsync = async;
}

bool StandardLayer::asyncRead()
{
    return m_readAsync;
}

void StandardLayer::addDataSet(const DataSetPtr &dataSet)
{
    return m_dataSets.push_back(dataSet);
}

void StandardLayer::hitTest(const MapPtr& map, const Rectangle& bounds, std::vector<PresentationObject>& presObjects)
{
    FeatureQuery featureQuery;
    featureQuery.area(bounds);
    featureQuery.scale(map->scale());
    featureQuery.updateAttributes(&map->updateAttributes());
    
    if (!isActiveForQuery(featureQuery))
    {
        return;
    }

    auto features = getFeatures(m_crs, featureQuery, true);

    while (features->moveNext())
    {
        const auto& f = features->current();

        for (const auto& vis : visualizers())
        {
            vis->hitTest(f, map->drawable(), bounds, presObjects);
        }

        if (map->isSelected(f->id()))
        {
            for (const auto& vis : selectionVisualizers())
            {
                vis->hitTest(f, map->drawable(), bounds, presObjects);
            }
        }
        else if (map->isHovered(f->id()))
        {
            for (const auto& vis : hoverVisualizers())
            {
                vis->hitTest(f, map->drawable(), bounds, presObjects);
            }
        }
    }
}

void StandardLayer::prepare(const CrsPtr &crs, const FeatureQuery &featureQuery)
{
    m_queriedFeatures = std::make_shared<FeatureEnumerator>();
    if (!isActiveForQuery(featureQuery))
    {
        return;
    }

    if (m_readAsync)
    {
        {
            std::lock_guard lock(m_mutex);
            auto cachedFeatures = m_cache->getAllFeatures();
            for (const auto& f : *cachedFeatures)
            {
                if (f->bounds().overlap(featureQuery.area()))
                {
                    m_queriedFeatures->add(f);
                }
            }
            m_doRead = true;
            m_query = featureQuery;
            m_crs = crs;
        }
        m_cond.notify_one();
    }
    else
    {
        m_queriedFeatures = getFeatures(crs, featureQuery, true);
    }
}

void StandardLayer::update(const MapPtr& map)
{
    const auto& features = m_queriedFeatures;
    auto& updateAttributes = map->updateAttributes();
    
    std::vector<FeaturePtr> hoveredFeatures;
    std::vector<FeaturePtr> selectedFeatures;
    
    bool hasAddedHoverAnSelection = false;
    
    for (const auto& vis : visualizers())
    {
        features->reset();
        map->drawable()->beginBatches();
        while (features->moveNext())
        {
            const auto& f = features->current();

            if (renderingEnabled())
            {
                //m_drawable->visualizerBegin();
                vis->renderFeature(*map->drawable(), f, updateAttributes); // Calls drawable->drawLine, drawable->drawPolygon etc
                //m_drawable->visualizerEnd();
            }

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
        map->drawable()->endBatches();
        hasAddedHoverAnSelection = true;
    }

    for (const auto& vis : hoverVisualizers())
    {
        map->drawable()->beginBatches();
        for (const auto& f : hoveredFeatures)
        {
            if (renderingEnabled())
                vis->renderFeature(*map->drawable(), f, updateAttributes);
        }
        map->drawable()->endBatches();
    }
    for (const auto& vis : selectionVisualizers())
    {
        map->drawable()->beginBatches();
        for (const auto& f : selectedFeatures)
        {
            if (renderingEnabled())
                vis->renderFeature(*map->drawable(), f, updateAttributes);
        }
        map->drawable()->endBatches();
    }
}

FeatureEnumeratorPtr StandardLayer::getFeatures(const CrsPtr &crs, const FeatureQuery& featureQuery, bool activeLayersOnly)
{
    auto features = std::make_shared<FeatureEnumerator>();
    if (activeLayersOnly && !isActiveForQuery(featureQuery))
    {
        return features;
    }
    
    for (const auto& d : m_dataSets)
    {
        auto dataSetFeatures = d->getFeatures(featureQuery);
        features->addEnumerator(dataSetFeatures);
    }

    return features;
}


void StandardLayer::createDefaultVisualizers()
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
    m_visualizers.push_back(polVis);
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
    m_hoverVisualizers.push_back(lineVisHover);
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
    polVisSelect->rotation([=](FeaturePtr f, Attributes& u) { return 2.0*BMM_PI*animatedDouble(f,u); });
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


void StandardLayer::backgroundReadingThread()
{
    BMM_DEBUG() << "StandardLayer::backgroundReadingThread() Started background reading thread\n";
    
    while (true)
    {
        std::unique_lock lock(m_mutex);
        m_cond.wait(lock, [this](){ return m_doRead || m_stop; });
        if (m_stop) break;

        m_doRead = false;
        auto crs = m_crs;
        auto query = m_query;
        lock.unlock();

        BMM_DEBUG() << "StandardLayer::backgroundReadingThread() LOAD for some time\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Faking load
        BMM_DEBUG() << "StandardLayer::backgroundReadingThread() LOAD done!\n";
        auto features = getFeatures(crs, query, true);
        
        std::unique_lock lock2(m_mutex);
        while (features->moveNext())
        {
            const auto& f = features->current();
            m_cache->insert(f->id(), f);
        }
        lock2.unlock();
        
        m_cond.notify_one();
    }

    BMM_DEBUG() << "StandardLayer::backgroundReadingThread() Background reading thread exited\n";
}

void StandardLayer::startbackgroundReadingThread()
{
    m_readingThread = std::thread([this](){backgroundReadingThread();});
}

void StandardLayer::stopBackgroundReadingThread()
{
    {
        std::lock_guard lock(m_mutex);
        m_stop = true;
    }
    m_cond.notify_one();
    if (m_readingThread.joinable())
    {
        m_readingThread.join();
    }
}
