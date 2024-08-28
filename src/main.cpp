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
        void init() override final
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

int main()
{
    // Create a display
    CImg<unsigned char> dummy; //("/home/joar/BlueMarbleMaps/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.tif");
    CImgDisplay display(dummy,"BlueMarbleMaps Demo", 3, true, true);
    //display.resize(3373, 1412, true); // Full screen doesnt work, using this temporarily
    display.resize(500, 500, true);

    // New
    BlueMarble::Map map(display);
    
    auto backgroundDataSet = BlueMarble::ImageDataSet("/home/joar/BlueMarbleMaps/geodata/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.tif");
    auto backgroundDataSet2 = BlueMarble::ImageDataSet("/home/joar/BlueMarbleMaps/geodata/BlueMarble.jpeg");
    auto shapeFileDataSet = BlueMarble::ShapeFileDataSet("");
    auto csvFileDataSet = BlueMarble::CsvFileDataSet("/home/joar/BlueMarbleMaps/geodata/svenska_stader/svenska-stader.csv");
    auto northAmerica = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/world_geojson/northamerica_high_fixed.geo.json");
    auto southAmerica = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/world_geojson/southamerica_high.geo.json");
    auto world = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/world_geojson/world_high.geo.json");
    auto roadsDataSet = BlueMarble::GeoJsonFileDataSet("/home/joar/BlueMarbleMaps/geodata/roads_geojson/europe-road.geojson");
    auto debugDataSet = DebugDataSet();
    roadsDataSet.init();
    backgroundDataSet.init();
    backgroundDataSet2.init();
    shapeFileDataSet.init();
    northAmerica.init();
    southAmerica.init();
    world.init();
    csvFileDataSet.init();
    debugDataSet.init();

    auto backgroundLayer = BlueMarble::Layer();
    auto backgroundLayer2 = BlueMarble::Layer();
    auto geoJsonLayer = BlueMarble::Layer();
    auto roadsGeoJsonLayer = BlueMarble::Layer();
    auto shapeFileLayer = BlueMarble::Layer();
    auto csvLayer = BlueMarble::Layer();
    auto debugLayer = BlueMarble::Layer();
    backgroundLayer.addUpdateHandler(&backgroundDataSet);
    backgroundLayer2.addUpdateHandler(&backgroundDataSet2);
    geoJsonLayer.addUpdateHandler(&northAmerica);
    geoJsonLayer.addUpdateHandler(&southAmerica);
    geoJsonLayer.addUpdateHandler(&world);
    roadsGeoJsonLayer.addUpdateHandler(&roadsDataSet);
    roadsGeoJsonLayer.minScale(2.0);
    shapeFileLayer.addUpdateHandler(&shapeFileDataSet);
    csvLayer.addUpdateHandler(&csvFileDataSet);
    debugLayer.addUpdateHandler(&debugDataSet);
    
    map.addLayer(&backgroundLayer);
    map.addLayer(&backgroundLayer2);
    map.addLayer(&geoJsonLayer);
    map.addLayer(&roadsGeoJsonLayer);
    //map.addLayer(&shapeFileLayer);
    map.addLayer(&csvLayer);
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
    feature.attributes().set("attribute3", "hello");

    std::cout << "attribute1: " << feature.attributes().get<int>("attribute1") << "\n";
    std::cout << "attribute2: " << feature.attributes().get<double>("attribute2") << "\n";
    std::cout << "attribute3: " << feature.attributes().get<std::string>("attribute3") << "\n";

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