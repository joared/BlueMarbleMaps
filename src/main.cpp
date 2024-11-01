#include <iostream>
#include <CImg.h>

#include "Map.h"
#include "DataSet.h"
#include "Core.h"
#include "Feature.h"
#include "CImgEventManager.h"
#include "PerformanceMonitor.h"

#include "DefaultEventHandlers.h"
#include "test_eventHandler.h"

using namespace cimg_library;

class DebugDataSet : public BlueMarble::DataSet
{
    public:
        

        void onUpdateRequest(BlueMarble::Map& map, const BlueMarble::Rectangle& updateArea, BlueMarble::FeatureHandler* handler) override final
        {

            // Draw center symbol
            auto sc = map.screenCenter();
            auto sc2 = map.mapToScreen(map.center());
            int radius = 15;
            map.drawable().drawCircle(sc.x(), sc.y(), radius, BlueMarble::Color(0, 0, 0, 0.2));

            // Cross
            // auto deltaW = Point(radius*0.75, 0);
            // auto deltaH = Point(0, radius*0.75);
            // std::vector<Point> line1 = {sc - deltaW, sc + deltaW};
            // std::vector<Point> line2 = {sc - deltaH, sc + deltaH};
            // m_drawable.drawLine(line1, Color(0, 0, 255, 0.7));
            // m_drawable.drawLine(line2, Color(0, 0, 255, 0.7));

            map.drawable().drawCircle(sc.x(), sc.y(), radius*0.3, BlueMarble::Color(0, 0, 255, 0.3));
            map.drawable().drawCircle(sc2.x(), sc2.y(), radius*0.1, BlueMarble::Color(1, 0, 0, 0.3));
            auto centerLngLat = map.mapToLngLat(map.center());
            std::string lngLatStr = "";
            lngLatStr += std::to_string(centerLngLat.x());
            lngLatStr += ", " + std::to_string(centerLngLat.y());
            int fontSize = 16;
            auto bcolor = BlueMarble::Color(25, 25, 75, 0.5);
            map.drawable().drawText(sc.x(), sc.y()+radius, lngLatStr.c_str(), BlueMarble::Color(255, 255, 255, 0.75), fontSize, bcolor);

            //handler->onFeatureInput() // TODO
        }

        void onGetFeaturesRequest(const BlueMarble::Attributes& attributes, std::vector<BlueMarble::FeaturePtr>& features) override final
        {

        }

        BlueMarble::FeaturePtr onGetFeatureRequest(const BlueMarble::Id& id) override final
        {
            return nullptr;
        }
    private:
        void init() override final {}
};

void setupMarkerLayerVisualization(BlueMarble::Layer& layer)
{
    auto symVis1 = std::make_shared<BlueMarble::SymbolVisualizer>();
    symVis1->color(ColorEvaluation([](auto, auto) { return BlueMarble::Color::black(0.5); }));
    symVis1->size([] (const FeaturePtr& feature, Attributes& updateAttributes) { 
        static auto from = DirectDoubleAttributeVariable(0.0);
        static auto to = DirectDoubleAttributeVariable(15.0);
        static auto value = AnimatedDoubleAttributeVariable(
            from, 
            to, 
            300,
            EasingFunctionType::Linear);
        
        return value(feature, updateAttributes);
        });
    auto symVis2 = std::make_shared<BlueMarble::SymbolVisualizer>();
    symVis2->color(ColorEvaluation([](auto, auto) { return BlueMarble::Color::blue(); }));
    symVis2->size([] (const FeaturePtr& feature, Attributes& updateAttributes) { 
        static auto from = DirectDoubleAttributeVariable(0.0);
        static auto to = DirectDoubleAttributeVariable(5.0);
        static auto value = AnimatedDoubleAttributeVariable(
            from, 
            to, 
            150,
            EasingFunctionType::Linear);
        
        return value(feature, updateAttributes);
        });
    auto textVis = std::make_shared<BlueMarble::TextVisualizer>();
    textVis->text([] (const FeaturePtr& f, auto) 
    { 
        if (f->attributes().contains("LNGLAT"))
        {
            return f->attributes().get<std::string>("LNGLAT");
        }
        return std::string();
    });
    textVis->offsetX([](auto, auto) {return -10; });
    textVis->offsetY([](auto, auto) {return -10; });

    layer.visualizers().push_back(symVis1);
    layer.visualizers().push_back(symVis2);
    layer.visualizers().push_back(textVis);
}

void setupAirPlaneLayerVisualization(BlueMarble::Layer& layer)
{
    auto symVis1 = std::make_shared<BlueMarble::SymbolVisualizer>();
    
    symVis1->symbol(SymbolVisualizer::Symbol());
    symVis1->size(DirectDoubleAttributeVariable(10));

    symVis1->symbol(SymbolVisualizer::Symbol("/home/joar/BlueMarbleMaps/geodata/symbols/airplane.png", HotSpotAlignments::Center));
    symVis1->size([] (const FeaturePtr& /*f*/, Attributes& updateAttribbutes)
        {
            double mapScale = updateAttribbutes.get<double>(UpdateAttributeKeys::UpdateViewScale);
            return Utils::clampValue(0.06*mapScale, 0.01, 0.06);
        }
    );
    //symVis1->rotation([] (const FeaturePtr&, Attributes&) { return 80.0; });
    symVis1->rotation(IndirectDoubleAttributeVariable("Rotation", 0.0));

    auto textSelHoverViz = std::make_shared<TextVisualizer>();
    textSelHoverViz->text(IndirectStringAttributeVariable("Name", std::string("Unknown aircraft")));
    textSelHoverViz->offsetY(DirectDoubleAttributeVariable(-20));
    textSelHoverViz->color(DirectColorAttributeVariable(Color::black()));
    textSelHoverViz->backgroundColor(DirectColorAttributeVariable(Color::white(0.5)));
    //symVis1->color([](auto, auto) { return BlueMarble::Color::black(0.5); });
    // symVis1->size([] (const FeaturePtr& feature, Attributes& updateAttributes) { 
    //     static auto from = DirectDoubleAttributeVariable(0.0);
    //     static auto to = DirectDoubleAttributeVariable(15.0);
    //     static auto value = AnimatedDoubleAttributeVariable(
    //         from, 
    //         to, 
    //         300,
    //         EasingFunctionType::Linear);
        
    //     return value(feature, updateAttributes);
    //     });
    layer.visualizers().push_back(symVis1);
    layer.hoverVisualizers().push_back(textSelHoverViz);
    layer.selectionVisualizers().push_back(textSelHoverViz);
    layer.effects().push_back(std::make_shared<BlueMarble::DropShadowEffect>(0.0, 10, 10, 0.5));
}

int main()
{
    // Create a display
    CImg<unsigned char> dummy; //("/home/joar/BlueMarbleMaps/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.tif");
    CImgDisplay display(dummy,"BlueMarbleMaps Demo", 3, true, true);
    //display.resize(3373, 1412, true); // Full screen doesnt work, using this temporarily
    display.resize(1500, 900, true);

    // New
    BlueMarble::Map map(display);
    PerformanceMonitor perfMonitor(map);
    
    auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/BlueMarbleMaps/geodata/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.tif");
    auto backgroundDataSet2 = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/BlueMarbleMaps/geodata/BlueMarble.jpeg");

    auto svenskaStader = std::make_shared<BlueMarble::CsvFileDataSet>("/home/joar/BlueMarbleMaps/geodata/svenska_stader/svenska-stader.csv");
    auto northAmerica = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/world_geojson/northamerica_high_fixed.geo.json");
    auto southAmerica = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/world_geojson/southamerica_high.geo.json");
    auto world = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/world_geojson/world_high.geo.json");
    auto continents = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/continents/continents.json");
    continents->name("Continents");
    auto sverigeRoadsDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/svenska_vagar/hotosm_swe_roads_lines_geojson.geojson"); sverigeRoadsDataSet->name("Roads sverige");
    auto roadsDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/roads_geojson/europe-road.geojson"); roadsDataSet->name("Roads");
    auto svenskaLandskapDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/svenska_landskap/svenska-landskap-klippt.geo.json");
    auto markerDataSet = std::make_shared<BlueMarble::MemoryDataSet>(); markerDataSet->name("MarkerDataSet");
    auto airPlaneDataSet = std::make_shared<BlueMarble::MemoryDataSet>(); airPlaneDataSet->name("AirPlanesDataSet");
    auto debugDataSet = std::make_shared<DebugDataSet>();
    
    //sverigeRoadsDataSet->initialize(); // Takes very long to initialize (1.4 GB large)
    roadsDataSet->initialize();
    backgroundDataSet->initialize(DataSetInitializationType::RightHereRightNow);
    backgroundDataSet2->initialize();

    northAmerica->initialize();
    southAmerica->initialize();
    world->initialize();
    continents->initialize();
    svenskaStader->initialize();
    svenskaLandskapDataSet->initialize();
    markerDataSet->initialize();
    airPlaneDataSet->initialize(DataSetInitializationType::RightHereRightNow); 
    // Populate airplanes and start animations
    for (int i=0; i < 10000; i++)
    {
        auto from = Point(std::rand() % 360 - 180, std::rand() % 180 - 90);
        auto to = Point(std::rand() % 360 - 180, std::rand() % 180 - 90);
        auto airPlaneFeature = airPlaneDataSet->createFeature(std::make_shared<PointGeometry>(from));
        airPlaneFeature->attributes().set("Name", "Aircraft #" + std::to_string(i+1));
        airPlaneDataSet->addFeature(airPlaneFeature); airPlaneDataSet->startFeatureAnimation(airPlaneFeature, from, to);
    }
    
    debugDataSet->initialize(); // Does nothing

    auto backgroundLayer = BlueMarble::Layer();
    auto backgroundLayer2 = BlueMarble::Layer();
    auto geoJsonLayer = BlueMarble::Layer();
    auto continentsLayer = BlueMarble::Layer();
    auto roadsGeoJsonLayer = BlueMarble::Layer();
    auto shapeFileLayer = BlueMarble::Layer();
    auto csvLayer = BlueMarble::Layer();
    auto sverigeLayer = BlueMarble::Layer();
    auto markerLayer = BlueMarble::Layer(false); setupMarkerLayerVisualization(markerLayer);
    auto airPlaneLayer = BlueMarble::Layer(false); setupAirPlaneLayerVisualization(airPlaneLayer);
    auto debugLayer = BlueMarble::Layer();
    backgroundLayer.addUpdateHandler(backgroundDataSet.get());
    backgroundLayer2.addUpdateHandler(backgroundDataSet2.get());
    
    double minScaleCountries = 0.25;
    geoJsonLayer.minScale(minScaleCountries);
    geoJsonLayer.addUpdateHandler(northAmerica.get()); geoJsonLayer.addUpdateHandler(southAmerica.get()); geoJsonLayer.addUpdateHandler(world.get());

    continentsLayer.maxScale(minScaleCountries);
    continentsLayer.addUpdateHandler(continents.get());
    roadsGeoJsonLayer.addUpdateHandler(roadsDataSet.get());
    roadsGeoJsonLayer.addUpdateHandler(sverigeRoadsDataSet.get());
    roadsGeoJsonLayer.minScale(5.0);
    roadsGeoJsonLayer.enabledDuringQuickUpdates(true);
    csvLayer.addUpdateHandler(svenskaStader.get());
    sverigeLayer.addUpdateHandler(svenskaLandskapDataSet.get());
    markerLayer.addUpdateHandler(markerDataSet.get());
    airPlaneLayer.addUpdateHandler(airPlaneDataSet.get());
    debugLayer.addUpdateHandler(debugDataSet.get());
    
    map.addLayer(&backgroundLayer);
    map.addLayer(&backgroundLayer2);
    map.addLayer(&geoJsonLayer);
    map.addLayer(&continentsLayer);
    map.addLayer(&sverigeLayer);
    map.addLayer(&roadsGeoJsonLayer);
    map.addLayer(&csvLayer);
    map.addLayer(&markerLayer);
    map.addLayer(&airPlaneLayer);
    map.addLayer(&debugLayer);

    // Setup event manager and event handlers
    BlueMarble::CImgEventManager eventManager(display);
    auto panHandler = BlueMarble::PanEventHandler(map, *world);
    //auto polygonHandler = BlueMarble::PolygonEventHandler(map);
    //auto testEventHandler = BlueMarble::TestEventHandler();
    eventManager.addSubscriber(&panHandler);
    // eventManager.addSubscriber(&polygonHandler);
    // eventManager.addSubscriber(&testEventHandler);
    
    auto geometry = std::make_shared<BlueMarble::RasterGeometry>();
    
    auto feature = BlueMarble::Feature({0,0}, geometry);
    feature.attributes().set("attribute1", 10);
    feature.attributes().set("attribute2", 10.01);
    feature.attributes().set("attribute3", std::string("hello"));
    feature.attributes().set("attribute4", true);

    std::cout << "attribute1: " << feature.attributes().tryGet<int>("attribute1") << "\n";
    std::cout << "attribute2: " << feature.attributes().tryGet<double>("attribute2") << "\n";
    std::cout << "attribute3: " << feature.attributes().tryGet<std::string>("attribute3") << "\n";
    std::cout << "attribute4: " << feature.attributes().tryGet<bool>("attribute4") << "\n";

    // Main loop
    map.startInitialAnimation();
    bool requireUpdate = map.update();
    while (!display.is_closed() && !display.is_keyESC()) 
    {
        if(!requireUpdate)
        {
            eventManager.wait();
        }
        else
        {
            //eventManager.captureEvents();
            eventManager.wait(20);
        }

        if (display.is_keyP())
        {
            std::cout << "Started performance monitor!\n";
            auto point = map.lngLatToMap(Point(30, 30));
            map.center(point);
            map.scale(0.3);
            auto report = perfMonitor.staticStartStop(5000);
            std::cout << report.toString() << "\n";
            report.save("/home/joar/BlueMarbleMaps/performance_reports/performance_report.pf");
        }
        if (display.is_keyD())
        {
            map.showDebugInfo() = !map.showDebugInfo();
        }

        // TODO: add as event in event manager instead
        if (display.is_keyF11())
        {
            display.set_fullscreen(!display.is_fullscreen(), false);
            display.resize(display.window_width(), display.window_height());
        }

        if (display.is_resized() || display.is_keyF11())
        {
            display.resize(display.window_width(), display.window_height());
            std::cout << "Resize: " << display.window_width() << ", " << display.window_height() << "\n";
            map.resize();
            requireUpdate = map.update(true);
        }
        else
        {
            requireUpdate = map.update();
        }
        
    }
    
    return 0;
}