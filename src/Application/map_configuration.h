#ifndef MAP_CONFIGURATION
#define MAP_CONFIGURATION

#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/Layer/StandardLayer.h"
#include "BlueMarbleMaps/Core/DataSets/DataSets.h"

using namespace BlueMarble;

void setupAirPlaneLayerVisualization(const BlueMarble::StandardLayerPtr& layer)
{
    auto symVis1 = std::make_shared<BlueMarble::SymbolVisualizer>();
    
    symVis1->symbol(SymbolVisualizer::Symbol());
    symVis1->size(DirectDoubleAttributeVariable(10));

    symVis1->symbol(SymbolVisualizer::Symbol("/home/joar/BlueMarbleMaps/geodata/symbols/airplane.png", HotSpotAlignments::Center));
    symVis1->size([] (const FeaturePtr& f, Attributes& updateAttribbutes)
        {
            double mapScale = updateAttribbutes.get<double>(UpdateAttributeKeys::UpdateViewScale);
            return Utils::clampValue(0.06*mapScale, 0.01, 0.06);
        }
    );

    symVis1->rotation(IndirectDoubleAttributeVariable("Rotation", 0.0));

    auto symVis2 = std::make_shared<BlueMarble::SymbolVisualizer>();
    symVis2->size(DirectDoubleAttributeVariable(10));
    symVis2->color(DirectColorAttributeVariable(Color::black()));

    auto textSelHoverViz = std::make_shared<TextVisualizer>();
    textSelHoverViz->text(IndirectStringAttributeVariable("Name", std::string("Unknown aircraft")));
    textSelHoverViz->offsetY(DirectDoubleAttributeVariable(-100));
    textSelHoverViz->color(DirectColorAttributeVariable(Color::black()));
    textSelHoverViz->backgroundColor(DirectColorAttributeVariable(Color::white(0.5)));

    auto textSelViz = std::make_shared<TextVisualizer>();
    textSelViz->text(IndirectStringAttributeVariable("Name", std::string("Unknown aircraft")));
    textSelViz->offsetY(DirectDoubleAttributeVariable(-100));
    textSelViz->color(DirectColorAttributeVariable(Color(255, 255, 0)));
    textSelViz->backgroundColor(DirectColorAttributeVariable(Color::white(0.5)));

    layer->visualizers().push_back(symVis2);
    layer->visualizers().push_back(symVis1);
    layer->hoverVisualizers().push_back(textSelHoverViz);
    layer->selectionVisualizers().push_back(textSelViz);
    //layer.effects().push_back(std::make_shared<BlueMarble::DropShadowEffect>(0.0, 10, 10, 0.5));
}

void addDataSetInitializationObserver(const MapPtr& map)
{
    static std::vector<DataSetPtr> dataSetsInitializing;
    static int64_t startTs = getTimeStampMs();
    static std::mutex m;

    DataSet::globalEvents.onInitializing += [&](auto d)
    {
        // These signals might be notified from a different thread
        std::lock_guard lock(m);
        dataSetsInitializing.push_back(d);
    };

    DataSet::globalEvents.onInitialized += [&](auto d)
    {
        // These signals might be notified from a different thread
        std::lock_guard lock(m);
        for (auto it = dataSetsInitializing.begin(); it != dataSetsInitializing.end(); ++it)
        {
            if (*it == d)
            {
                dataSetsInitializing.erase(it);
                break;
            }
        }
    };

    auto generateArcLine = [](double r, double theta1, double theta2)
    {
        auto line = std::make_shared<LineGeometry>();

        int pointsPerRev = 30;
        double diff = Utils::normalizeValue(theta2-theta1, 0.0, BMM_PI*2.0);
        int nPoints = diff / (BMM_PI*2.0) * pointsPerRev;

        for (int i(0); i<nPoints; ++i)
        {
            double a = theta1 + i*diff/(double)nPoints;
            double x = r*std::cos(a);
            double y = r*std::sin(a);
            auto p = Point(x,y);
            line->points().push_back(p);
        }

        return line;
    };

    auto drawLoadingSymbol = [&](const DrawablePtr& drawable, int x, int y, double radius, double progress)
    {
        static auto radiusEval = AnimationFunctions::AnimationBuilder().subDivide(2).easeInCubic().inverseAt(0.5).build();
        static auto theta1Eval = AnimationFunctions::AnimationBuilder().subDivide(2).offset(0.5).easeOut(2.5).build();
        static auto theta2Eval = AnimationFunctions::AnimationBuilder().subDivide(2).easeOut(2.5).build();

        double radiusProgress = radiusEval(progress);
        radius =  (1.0-radiusProgress) * 5.0 + std::max(radius - 5.0, 5.0);
        
        double ratio1 = theta1Eval(progress);
        double ratio2 = theta2Eval(progress);

        double aTo = ratio1 * BMM_PI * 2.0 - BMM_PI * 0.2;
        double aFrom = ratio2 * BMM_PI * 2.0 - BMM_PI * 0.2;
        auto line = generateArcLine(radius, aFrom, aTo);
        
        line->move(Point(x, y));
        Pen p;
        p.setColors(Color::colorRamp(Color::red(0.2), Color::red(), line->points().size()));
        p.setThickness(5.0);
        drawable->drawLine(line, p);
    };

    auto tempAnimationView = [&](Map& view)
    {
        static auto anim1 = AnimationFunctions::AnimationBuilder().alternate().inverse().build();
        static auto anim2 = AnimationFunctions::AnimationBuilder().alternate().inverse().easeInCubic().build();
        static auto anim3 = AnimationFunctions::AnimationBuilder().alternate().inverse().easeInCubic().inverse().build();

        static auto functions = std::vector<AnimationFunctions::AnimFunc>{anim1, anim2, anim3};
        static auto colors    = Color::colorRamp(Color::red(), Color::blue(), functions.size());

        auto d = view.drawable();
        auto delta = Point(d->width()*0.05, d->height()*0.25);
        std::vector<LineGeometryPtr> lines;
        lines.reserve(functions.size());
        for (int x(0); x < functions.size(); ++x) 
        {
            LineGeometryPtr line = std::make_shared<LineGeometry>();
            lines.push_back(line);
        }

        int nSamples = 100;
        for (int i(0); i<nSamples; ++i)
        {
            double progress = i / double(nSamples);
            for (int j(0); j < functions.size(); ++j)
            {
                double outP = functions[j](progress);
                auto line = lines[j];
                line->points().push_back(Point(i, -outP*100) + delta);
            }
        }
        
        auto backgroundRect = Rectangle(0, 0, nSamples, nSamples);
        backgroundRect.offset(delta.x(), delta.y()-nSamples);
        auto polyBackground = std::make_shared<PolygonGeometry>(backgroundRect.corners());
        Pen penBack; penBack.setColors(Color::colorRamp(Color::yellow(), Color::gray(), 4));
        Brush brushBack; brushBack.setColor(Color::gray(0.5));
        d->drawPolygon(polyBackground, penBack, brushBack);

        for (int k(0); k < lines.size(); ++k)
        {
            auto l = lines[k];
            auto c = colors[k];
            Pen pen;
            pen.setColor(c);
            d->drawLine(l, pen);
        }
    };
    //map->events.onCustomDraw += tempAnimationView;

    map->events.onCustomDraw += ([=](Map& view)
    {
        constexpr int animationTime = 2000;
        int64_t elapsed = getTimeStampMs()-startTs;
        elapsed = elapsed % animationTime;
        
        int nDataSets = 0;
        {
            std::lock_guard lock(m);
            nDataSets = dataSetsInitializing.size();
        }

        if (nDataSets == 0) return;

        auto drawable = view.drawable();
        double radius = 15.0;
        double offset = radius*3.0;
        int baseX = drawable->width() - offset;
        int baseY = drawable->height() - offset;
        double progress = elapsed/double(animationTime);
        for (int i(0); i<nDataSets; ++i)
        {
            int x = baseX;
            int y = baseY - offset*i;
            drawLoadingSymbol(drawable, x, y, radius, progress);
        }

        view.update();
    });
}

void configureGui(const MapPtr& map)
{
    auto northArrowDataSet = std::make_shared<MemoryDataSet>();northArrowDataSet->initialize();
    auto northArrowLayer = std::make_shared<StandardLayer>(true); northArrowLayer->addDataSet(northArrowDataSet);
    northArrowLayer->visualizers().push_back(std::make_shared<PolygonVisualizer>());
    northArrowLayer->selectable(true);

    auto polyGeometry = std::make_shared<PolygonGeometry>(Rectangle(0,0,40,40).corners());

    auto feature = northArrowDataSet->createFeature(polyGeometry);
    northArrowDataSet->addFeature(feature);
    map->addLayer(northArrowLayer);
    
    map->events.onUpdating.subscribe([=](Map& view)
    {
        int w = view.drawable()->width();
        int h = view.drawable()->height();
        auto mapPoints = view.screenToMap(Rectangle(w/2.0,h/2.0,w/2.0+40,h/2.0+40).corners());
        auto dataSetPoints = view.crs()->projectTo(northArrowDataSet->crs(), mapPoints.begin(), mapPoints.end());
        auto temp = std::vector<Point>(dataSetPoints->begin(), dataSetPoints->end());
        polyGeometry->rings()[0] = temp;
        // BMM_DEBUG() << polyGeometry->rings()[0][0].toString() << "\n";
    }).permanent();
    static bool exist = true;
    map->events.onUpdated += ([=](Map& view)
    {
        if (std::abs(view.rotation()) < 1e-7 && exist)
        {
            northArrowDataSet->removeFeature(feature->id());
            exist = false;
        }
        else if (view.rotation() != 0.0 && !exist)
        {
            northArrowDataSet->addFeature(feature);
            exist = true;
        }
    });

    map->events.onHoverChanged += ([=](Map& view, const Id& id)
    {
        if (id == feature->id())
        {
            BMM_DEBUG() << "Hovered north arrow!!!\n";
        }
    });

    map->events.onSelectionChanged += ([=](Map& view, const IdCollectionPtr& ids)
    {
        if (ids->contains(feature->id()))
        {
            BMM_DEBUG() << "Hovered north arrow clicked!!!\n";
            // TODO: rotation animation
            view.rotation(0.0);
            view.deSelect(feature);
            view.update();
        }
    });

    map->events.onCustomDraw += ([](Map& view)
    {
        
    });
}

void configureMap(const MapPtr& map, bool includeBackground=false, bool includeRoads=false, bool includeAirPlanes=false)
{
    const bool asyncBackgroundReading = true;
    const bool backgroundLayersSelectable = true;
    const DataSetInitializationType dataSetInitialization = DataSetInitializationType::BackgroundThread;
    const std::string commonIndexPath = "../../../bluemarble_index"; // Relative to the build/bin/<debug/release>/ folder


    addDataSetInitializationObserver(map);

    ////////////////////////////////////////////////////////
    static auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/BlueMarbleMaps/geodata/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.tif");
    static auto backgroundDataSet2 = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/BlueMarbleMaps/geodata/BlueMarble.jpeg");

    static auto svenskaStader = std::make_shared<BlueMarble::CsvFileDataSet>("/home/joar/BlueMarbleMaps/geodata/svenska_stader/svenska-stader.csv");
    static auto northAmerica = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/world_geojson/northamerica_high_fixed.geo.json");
    static auto southAmerica = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/world_geojson/southamerica_high.geo.json");
    static auto world = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/world_geojson/world_high.geo.json");
    static auto continents = std::make_shared<BlueMarble::GeoJsonFileDataSet>("../../../geodata/continents/continents.json");
    continents->name("Continents");
    static auto sverigeRoadsDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/svenska_vagar/hotosm_swe_roads_lines_geojson.geojson"); sverigeRoadsDataSet->name("Roads sverige");
    static auto roadsDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/roads_geojson/europe-road.geojson"); roadsDataSet->name("Roads");
    static auto svenskaLandskapDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/svenska_landskap/svenska-landskap-klippt.geo.json");
    static auto markerDataSet = std::make_shared<BlueMarble::MemoryDataSet>(); markerDataSet->name("MarkerDataSet");
    static auto airPlaneDataSet = std::make_shared<BlueMarble::MemoryDataSet>(); airPlaneDataSet->name("AirPlanesDataSet");

    continents->indexPath(commonIndexPath);
    continents->initialize(dataSetInitialization);
    // svenskaStader->initialize();
    // svenskaLandskapDataSet->initialize();
    // markerDataSet->initialize();
    
    airPlaneDataSet->initialize(DataSetInitializationType::RightHereRightNow); 
    // Populate airplanes and start animations
    if (includeAirPlanes)
    {
        for (int i=0; i < 10000; i++)
        {
            auto from = Point(std::rand() % 360 - 180, std::rand() % 180 - 90);
            auto to = Point(std::rand() % 360 - 180, std::rand() % 180 - 90);
            auto airPlaneFeature = airPlaneDataSet->createFeature(std::make_shared<PointGeometry>(from));
            airPlaneFeature->attributes().set("Name", "Aircraft #" + std::to_string(i+1));
            airPlaneDataSet->addFeature(airPlaneFeature); airPlaneDataSet->startFeatureAnimation(airPlaneFeature, from, to);
        }
    }

    auto backgroundLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    
    auto geoJsonLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer()); geoJsonLayer->asyncRead(asyncBackgroundReading);
    auto continentsLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer()); continentsLayer->asyncRead(asyncBackgroundReading);
    auto roadsGeoJsonLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    auto shapeFileLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    auto airPlaneLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer(false)); 
    if(includeAirPlanes) setupAirPlaneLayerVisualization(airPlaneLayer);
    auto csvLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    auto sverigeLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    auto debugLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    if (includeBackground)
    {
        //backgroundDataSet->initialize();
        //backgroundLayer->addDataSet(backgroundDataSet);

        auto backgroundLayer2 = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer(false));
        auto rasterVis = std::make_shared<RasterVisualizer>();
        rasterVis->alpha(DirectDoubleAttributeVariable(0.3));
        backgroundLayer2->visualizers().push_back(rasterVis);
        backgroundDataSet2->initialize();
        backgroundLayer2->addDataSet(backgroundDataSet2);
        backgroundLayer2->asyncRead(true);
        map->addLayer(backgroundLayer2);
    }
    
    double minScaleCountries = 1.0/60000000.0;
    geoJsonLayer->minScale(minScaleCountries);
    geoJsonLayer->selectable(backgroundLayersSelectable);

    bool includeCountryPolygons = true; // TODO add parameter
    if (includeCountryPolygons) 
    {
        northAmerica->indexPath(commonIndexPath);
        southAmerica->indexPath(commonIndexPath);
        world->indexPath(commonIndexPath);
        northAmerica->initialize(dataSetInitialization);
        southAmerica->initialize(dataSetInitialization);
        world->initialize(dataSetInitialization);
        geoJsonLayer->addDataSet(northAmerica); 
        geoJsonLayer->addDataSet(southAmerica); 
        geoJsonLayer->addDataSet(world);
    }

    continentsLayer->maxScale(minScaleCountries);
    continentsLayer->addDataSet(continents);
    continentsLayer->selectable(backgroundLayersSelectable);
    if (includeRoads)
    {
        roadsDataSet->indexPath(commonIndexPath);
        roadsDataSet->initialize(dataSetInitialization);

        // FeatureQuery featureQuery;
        // featureQuery.area(Rectangle(-180, -90, 180, 90));
        // auto features = roadsDataSet->getFeatures(featureQuery);
        // int n = features->size();
        // BMM_DEBUG() << "NUMBER OF ROAD FEATURES: " << n << "\n";

        roadsGeoJsonLayer->addDataSet(roadsDataSet);
        roadsGeoJsonLayer->asyncRead(asyncBackgroundReading);
        roadsGeoJsonLayer->selectable(backgroundLayersSelectable);
        // sverigeRoadsDataSet->indexPath(commonIndexPath);
        // sverigeRoadsDataSet->initialize(dataSetInitialization); // Takes very long to initialize (1.4 GB large)
        // roadsGeoJsonLayer->addDataSet(sverigeRoadsDataSet);
        
        roadsGeoJsonLayer->minScale(1.0/250000000.0);
        //roadsGeoJsonLayer->enabledDuringQuickUpdates(false);
    }
    
    csvLayer->addDataSet(svenskaStader);
    sverigeLayer->addDataSet(svenskaLandskapDataSet);
    airPlaneLayer->addDataSet(airPlaneDataSet);
    
    
    map->addLayer(backgroundLayer);
    
    map->addLayer(geoJsonLayer);
    map->addLayer(continentsLayer);
    //map->addLayer(sverigeLayer);
    map->addLayer(roadsGeoJsonLayer);
    //map->addLayer(csvLayer);
    map->addLayer(airPlaneLayer);
    //map->addLayer(debugLayer);
    ////////////////////////////////////////////////////////OLD
    configureGui(map);
}

#endif /* MAP_CONFIGURATION */
