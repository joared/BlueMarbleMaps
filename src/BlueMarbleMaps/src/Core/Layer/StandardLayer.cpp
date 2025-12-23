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

    FeatureEnumeratorPtr featureEnum;
    if (m_readAsync)
    {
        featureEnum = std::make_shared<FeatureEnumerator>();
        // If we are reading asynchronously, we are filling the cache and the features SHOULD be there
        // Additionally, if we try to call getFeatures() as in the else statement, we are concurrently 
        // querying datasets in two different threads. That should be okay, but doesnt work atm.
        auto ids = getFeatureIds(map->crs(), featureQuery);
        auto features = std::make_shared<FeatureCollection>();
        features->reserve(ids->size());
        // The cache needs to be locked
        std::lock_guard lock(m_mutex);
        for (const auto& id : *ids)
        {
            if (!m_cache->contains(id))
            {
                // There is something that is potentially is hit here that we haven't cached,
                // and in turn haven't rendered yet. Maybe it's okay not to return a result for something that hasn't been rendered?

                // Some debug info in case we change our minds
                // for (const auto& idd : *m_query.ids())
                // {
                //     BMM_DEBUG() << "Id in query: " << idd.toString() << "\n";
                // }
                //BMM_DEBUG() << "StandardLayer::hitTest() Feature with id '" + id.toString() + "' missing in cache, should not happen!\n";
            }
            else
            {
                auto f = m_cache->getFeature(id);
                features->add(f);
            }
        }
        featureEnum->setFeatures(features);
    }
    else
    {
        featureEnum = getFeatures(map->crs(), featureQuery, true);
    }

    while (featureEnum->moveNext())
    {
        const auto& f = featureEnum->current();

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
            // New
            auto ids = getFeatureIds(crs, featureQuery);
            auto cacheMissingIds = std::make_shared<IdCollection>();
            for (auto const& id : *ids)
            {
                if (m_cache->contains(id))
                {
                    m_queriedFeatures->add(m_cache->getFeature(id));
                }
                else
                {
                    cacheMissingIds->add(id);
                }
            }
            // Old
            // auto cachedFeatures = m_cache->getAllFeatures();
            // for (const auto& f : *cachedFeatures)
            // {
            //     if (f->bounds().overlap(featureQuery.area()))
            //     {
            //         m_queriedFeatures->add(f);
            //     }
            // }
            m_doRead = true;
            m_query = featureQuery;
            m_query.ids(cacheMissingIds);
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

FeatureEnumeratorPtr StandardLayer::getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly)
{
    auto features = std::make_shared<FeatureCollection>();
    auto enumerator = std::make_shared<FeatureEnumerator>();
    enumerator->setFeatures(features);
    if (activeLayersOnly && !isActiveForQuery(featureQuery))
    {
        return enumerator;
    }
    
    for (const auto& d : m_dataSets)
    {
        auto newQuery = featureQuery;
        newQuery.area(crs->projectTo(d->crs(), newQuery.area()));
        auto dataSetFeatures = d->getFeatures(newQuery);

        // If the crs and data set crs is different, we need to reproject them
        if (!crs->isFunctionallyEquivalent(d->crs()))
        {
            BMM_DEBUG() << "Reprojecting features!\n";
            while (dataSetFeatures->moveNext())
            {
                auto f = dataSetFeatures->current();
                auto newFeatures = f->projectTo(crs);
                features->addRange(*newFeatures);
            }
            dataSetFeatures->reset();
        }
        else
        {
            enumerator->addEnumerator(dataSetFeatures);
        }
    }

    return enumerator;
}

void StandardLayer::flushCache()
{
    {
        std::unique_lock lock2(m_mutex);
        m_cache->clear();
    }
    
    for (const auto& d : m_dataSets)
    {
        d->flushCache();
    }
}

IdCollectionPtr StandardLayer::getFeatureIds(const CrsPtr& crs, const FeatureQuery& featureQuery)
{
    auto ids = std::make_shared<IdCollection>();
    for (const auto& d : m_dataSets)
    {
        auto newQuery = featureQuery;
        newQuery.area(crs->projectTo(d->crs(), newQuery.area()));
        auto dataSetIds = d->getFeatureIds(newQuery);
        ids->addRange(*dataSetIds);
    }

    return ids;
}

FeatureCollectionPtr StandardLayer::getFeatures(const CrsPtr &crs, const IdCollectionPtr& ids)
{
    auto features = std::make_shared<FeatureCollection>();
    for (const auto& d : m_dataSets)
    {
        auto dataSetFeatures = d->getFeatures(ids);
        // If the crs and data set crs is different, we need to reproject them
        if (!crs->isFunctionallyEquivalent(d->crs()))
        {
            BMM_DEBUG() << "Reprojecting features!\n";
            for (const auto& f : *dataSetFeatures)
            {
                auto newFeatures = f->projectTo(crs);
                features->addRange(*newFeatures);
            }
        }
        else
        {
            features->addRange(*dataSetFeatures);
        }
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
            0);
        
        return value(feature, updateAttributes);
    };
    auto animatedDouble2 = [](FeaturePtr feature, Attributes& updateAttributes) 
    { 
        static auto from = DirectDoubleAttributeVariable(0.0);
        static auto to = DirectDoubleAttributeVariable(1.0);
        static auto value = AnimatedDoubleAttributeVariable(
            from, 
            to, 
            300,
            0);
        
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
        auto toColor = Color(250, 250, 196, 0.75);
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
    lineVis->color(ColorEvaluation([](FeaturePtr, Attributes&) { return Color(50,50,50,0.2); }));
    //lineVis->color(colorEvalSelect);
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
    lineVisHover->color([=](auto f, auto& u) { return Color(255,255,255,1.0); });
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
    lineVisSelect->color([=](auto f, auto& u) { 
        auto an = animatedDouble(f,u);
        return Color(255*an,255,255*(1.0-an),1.0); });
    lineVisSelect->width(DirectDoubleAttributeVariable(3.0)); //[](auto, auto) { return 3.0; });
    lineVisSelect->offsetZ([=](auto f, Attributes& u) { 
        return (1.0/100.0)/u.get<double>(UpdateAttributeKeys::UpdateViewScale)*animatedDouble(f,u); 
    }); // FIXME: not providing "auto&" with "&" causes update attributes to be copied. This is fragile, since e.g. animation depends on setting the UpdateRequired attribute

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

        // reset just so I can debug
        m_query = FeatureQuery();
        m_crs = nullptr;

        lock.unlock();

        // BMM_DEBUG() << "StandardLayer::backgroundReadingThread() LOAD for some time\n";
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Faking load
        // BMM_DEBUG() << "StandardLayer::backgroundReadingThread() LOAD done!\n";
        
        // New
        auto features = getFeatures(crs, query.ids());
        std::unique_lock lock2(m_mutex);
        for (const auto& f : *features)
        {
            m_cache->insert(f->id(), f);
        }
        
        // Old
        // auto features = getFeatures(crs, query, true);
        // std::unique_lock lock2(m_mutex);
        // while (features->moveNext())
        // {
        //     const auto& f = features->current();
        //     m_cache->insert(f->id(), f);
        // }
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
