#include <iostream>
#include <CImg.h>

#include "Map.h"
#include "DataSet.h"
#include "Core.h"
#include "Feature.h"
#include "CImgEventManager.h"

#include "DefaultEventHandlers.h"
#include "test_eventHandler.h"

using namespace cimg_library;

class DebugDataSet : public BlueMarble::DataSet
{
    public:
        void init(DataSetInitializationType initType = DataSetInitializationType::RightHereRightNow) override final
        {}

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
};

void setupMarkerLayerVisualization(BlueMarble::Layer& layer)
{
    auto symVis1 = std::make_shared<BlueMarble::SymbolVisualizer>();
    symVis1->color([](auto, auto) { return BlueMarble::Color::black(0.5); });
    symVis1->size([] (auto, auto) { return 15.0; });
    auto symVis2 = std::make_shared<BlueMarble::SymbolVisualizer>();
    symVis2->color([](auto, auto) { return BlueMarble::Color::blue(); });
    symVis2->size([] (auto, auto) { return 5.0; });
    auto textVis = std::make_shared<BlueMarble::TextVisualizer>();
    textVis->text([] (FeaturePtr f, auto) 
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

int main()
{
    // Create a display
    CImg<unsigned char> dummy; //("/home/joar/BlueMarbleMaps/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.tif");
    CImgDisplay display(dummy,"BlueMarbleMaps Demo", 3, true, true);
    //display.resize(3373, 1412, true); // Full screen doesnt work, using this temporarily
    display.resize(1500, 900, true);

    // New
    BlueMarble::Map map(display);
    
    auto backgroundDataSet = BlueMarble::ImageDataSet("/home/joar/BlueMarbleMaps/geodata/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.tif");
    auto backgroundDataSet2 = BlueMarble::ImageDataSet("/home/joar/BlueMarbleMaps/geodata/BlueMarble.jpeg");
    auto shapeFileDataSet = BlueMarble::ShapeFileDataSet("");
    auto csvFileDataSet = BlueMarble::CsvFileDataSet("/home/joar/BlueMarbleMaps/geodata/svenska_stader/svenska-stader.csv");
    auto northAmerica = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/world_geojson/northamerica_high_fixed.geo.json");
    auto southAmerica = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/world_geojson/southamerica_high.geo.json");
    auto world = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/world_geojson/world_high.geo.json");
    auto continents = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/continents/continents.json");
    auto sverigeRoadsDataSet = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/svenska_vagar/hotosm_swe_roads_lines_geojson.geojson");
    auto roadsDataSet = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/roads_geojson/europe-road.geojson");
    auto svenskaLandskapDataSet = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/svenska_landskap/svenska-landskap-klippt.geo.json");
    auto markerDataSet = BlueMarble::MemoryDataSet();
    auto debugDataSet = DebugDataSet();
    sverigeRoadsDataSet.name("Roads sverige");
    //sverigeRoadsDataSet.init();
    roadsDataSet.name("Roads"); roadsDataSet.init();
    backgroundDataSet.init(DataSetInitializationType::RightHereRightNow);
    backgroundDataSet2.init();
    shapeFileDataSet.init();
    northAmerica.init();
    southAmerica.init();
    world.init();
    continents.name("Continents"); continents.init();
    csvFileDataSet.init();
    svenskaLandskapDataSet.init();
    markerDataSet.name("MarkerDataSet"); markerDataSet.init();
    debugDataSet.init();

    auto backgroundLayer = BlueMarble::Layer();
    auto backgroundLayer2 = BlueMarble::Layer();
    auto geoJsonLayer = BlueMarble::Layer();
    auto continentsLayer = BlueMarble::Layer();
    auto roadsGeoJsonLayer = BlueMarble::Layer();
    auto shapeFileLayer = BlueMarble::Layer();
    auto csvLayer = BlueMarble::Layer();
    auto sverigeLayer = BlueMarble::Layer();
    auto markerLayer = BlueMarble::Layer(false); setupMarkerLayerVisualization(markerLayer);
    auto debugLayer = BlueMarble::Layer();
    backgroundLayer.addUpdateHandler(&backgroundDataSet);
    backgroundLayer2.addUpdateHandler(&backgroundDataSet2);
    
    double minScaleCountries = 0.25;
    geoJsonLayer.minScale(minScaleCountries);
    geoJsonLayer.addUpdateHandler(&northAmerica); geoJsonLayer.addUpdateHandler(&southAmerica); geoJsonLayer.addUpdateHandler(&world);

    continentsLayer.maxScale(minScaleCountries);
    continentsLayer.addUpdateHandler(&continents);

    roadsGeoJsonLayer.addUpdateHandler(&roadsDataSet);
    roadsGeoJsonLayer.addUpdateHandler(&sverigeRoadsDataSet);
    roadsGeoJsonLayer.minScale(5.0);
    roadsGeoJsonLayer.enabledDuringQuickUpdates(true);
    shapeFileLayer.addUpdateHandler(&shapeFileDataSet);
    csvLayer.addUpdateHandler(&csvFileDataSet);
    sverigeLayer.addUpdateHandler(&svenskaLandskapDataSet);
    markerLayer.addUpdateHandler(&markerDataSet);
    debugLayer.addUpdateHandler(&debugDataSet);
    
    map.addLayer(&backgroundLayer);
    map.addLayer(&backgroundLayer2);
    map.addLayer(&geoJsonLayer);
    map.addLayer(&continentsLayer);
    map.addLayer(&sverigeLayer);
    map.addLayer(&roadsGeoJsonLayer);
    //map.addLayer(&shapeFileLayer);
    map.addLayer(&csvLayer);
    map.addLayer(&markerLayer);
    map.addLayer(&debugLayer);

    // Setup event manager and event handlers
    BlueMarble::CImgEventManager eventManager(display);
    auto panHandler = BlueMarble::PanEventHandler(map, world);
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
        else if (display.is_event())
        {
            eventManager.captureEvents();
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