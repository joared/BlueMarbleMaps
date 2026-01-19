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


void configureMap(const MapPtr& map)
{
    const std::string commonIndexPath = "../../../bluemarble_index"; // Relative to the build/bin/<debug/release>/ folder
    const DataSetInitializationType dataSetInitialization = DataSetInitializationType::BackgroundThread;
    const bool backgroundLayersSelectable = true;
    const bool asyncBackgroundReading = true;

    const bool includeBackgroundRaster = true;
    const bool includeContinents = true;
    const bool includeCountries = true;
    const bool includeRoadsEurope = true;
    const bool includeSwedenRoads = true;
    const bool includeMemoryDataSet = true;

    const double minScaleCountries = 1.0/60000000.0;

    addDataSetInitializationObserver(map);

    if (includeBackgroundRaster)
    {
        #ifdef WIN32
        #define PATH_TO_FANNY_FILE "../../../bluemarble_index/backgroundmap.png"
        #else 
        #define PATH_TO_FANNY_FILE "../../../geodata/HYP_HR_SR_OB_DR/HYP_HR_SR_OB_DR.tif"
        #endif
        auto backgroundImageDataSet = std::make_shared<ImageDataSet>(PATH_TO_FANNY_FILE);
        backgroundImageDataSet->initialize(DataSetInitializationType::RightHereRightNow);
        
        auto backgroundLayer = std::make_shared<StandardLayer>(false);
        backgroundLayer->addDataSet(backgroundImageDataSet);
        backgroundLayer->asyncRead(true); // NOTE: If this is false, changing coordinate system is expensive!
        auto rasterVis = std::make_shared<RasterVisualizer>();
        rasterVis->alpha(DirectDoubleAttributeVariable(0.7));
        backgroundLayer->visualizers().push_back(rasterVis);
        
        map->addLayer(backgroundLayer);
    }

    if (includeCountries) 
    {
        // Datasets
        auto northAmerica = std::make_shared<BlueMarble::GeoJsonFileDataSet>("../../../geodata/world_geojson/northamerica_high_fixed.geo.json");
        auto southAmerica = std::make_shared<BlueMarble::GeoJsonFileDataSet>("../../../geodata/world_geojson/southamerica_high.geo.json");
        auto world = std::make_shared<BlueMarble::GeoJsonFileDataSet>("../../../geodata/world_geojson/world_high.geo.json");
        northAmerica->indexPath(commonIndexPath);
        southAmerica->indexPath(commonIndexPath);
        world->indexPath(commonIndexPath);
        northAmerica->initialize(dataSetInitialization);
        southAmerica->initialize(dataSetInitialization);
        world->initialize(dataSetInitialization);
     
        // Layer
        auto countryLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer()); 
        countryLayer->asyncRead(asyncBackgroundReading);
        countryLayer->minScale(minScaleCountries);
        countryLayer->selectable(backgroundLayersSelectable);
        countryLayer->addDataSet(northAmerica); 
        countryLayer->addDataSet(southAmerica); 
        countryLayer->addDataSet(world);

        map->addLayer(countryLayer);
    }

    if (includeContinents)
    {
        // Dataset
        auto continents = std::make_shared<BlueMarble::GeoJsonFileDataSet>("../../../geodata/continents/continents.json");
        continents->indexPath(commonIndexPath);
        continents->initialize(dataSetInitialization);
        
        // Layer
        auto continentsLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer()); 
        continentsLayer->asyncRead(asyncBackgroundReading);
        continentsLayer->maxScale(minScaleCountries);
        continentsLayer->addDataSet(continents);
        continentsLayer->selectable(backgroundLayersSelectable);

        map->addLayer(continentsLayer);
    }
    
    if (includeRoadsEurope)
    {
        // Dataset
        auto roadsDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("../../../geodata/roads_geojson/europe-road.geojson"); 
        roadsDataSet->name("Roads Europe");
        roadsDataSet->indexPath(commonIndexPath);
        roadsDataSet->initialize(dataSetInitialization);

        // Layer
        auto roadsGeoJsonLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
        roadsGeoJsonLayer->addDataSet(roadsDataSet);
        roadsGeoJsonLayer->asyncRead(asyncBackgroundReading);
        roadsGeoJsonLayer->selectable(backgroundLayersSelectable);
        roadsGeoJsonLayer->minScale(1.0/2500000.0);
        //roadsGeoJsonLayer->enabledDuringQuickUpdates(false);

        map->addLayer(roadsGeoJsonLayer);
    }
    if (includeSwedenRoads)
    {
        // Dataset
        auto sverigeRoadsDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("../../../geodata/svenska_vagar/hotosm_swe_roads_lines_geojson.geojson"); 
        sverigeRoadsDataSet->name("Roads Sweden");
        sverigeRoadsDataSet->indexPath(commonIndexPath);
        sverigeRoadsDataSet->initialize(dataSetInitialization); // Takes very long to initialize (1.4 GB large)

        // Layer
        auto swedenroadsGeoJsonLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
        swedenroadsGeoJsonLayer->minScale(1.0/100000.0);
        swedenroadsGeoJsonLayer->addDataSet(sverigeRoadsDataSet);
        swedenroadsGeoJsonLayer->asyncRead(asyncBackgroundReading);

        map->addLayer(swedenroadsGeoJsonLayer);
    }

    if (includeMemoryDataSet)
    {
        // Test Polygon/Line/Symbol visualizers
        auto vectorDataSet = std::make_shared<MemoryDataSet>();
        vectorDataSet->initialize(DataSetInitializationType::RightHereRightNow);

        auto grusgrus = std::vector<Point>({{18.048981189498615, 59.34314379243086}, {18.048965934358133, 59.34327357866866}, {18.049345426522272, 59.34328190127159}, {18.049360728625665, 59.34315706201355}});
        auto polypoints = std::vector<Point>({{14,56}, {14,57}, {15,57}});
        auto linepoints = std::vector<Point>({{15,57}, {15,58}, {16,58}});
        auto grusgrusfeature = vectorDataSet->createFeature(std::make_shared<PolygonGeometry>(grusgrus));
        auto testfeature = vectorDataSet->createFeature(std::make_shared<PolygonGeometry>(polypoints));
        auto testfeatureLine = vectorDataSet->createFeature(std::make_shared<LineGeometry>(linepoints));
        testfeature->move({-15,-57});
        testfeatureLine->move({-15,-57});
        vectorDataSet->addFeature(grusgrusfeature);
        vectorDataSet->addFeature(testfeature);
        vectorDataSet->addFeature(testfeatureLine);

        auto vectorLayer = std::make_shared<StandardLayer>(true);
        vectorLayer->addDataSet(vectorDataSet);
        vectorLayer->selectable(true);

        auto polyVis = std::make_shared<PolygonVisualizer>();
        polyVis->color(DirectColorAttributeVariable(Color(255,194,209)));
        vectorLayer->visualizers().push_back(polyVis);

        map->addLayer(vectorLayer);
    }

}

#endif /* MAP_CONFIGURATION */
