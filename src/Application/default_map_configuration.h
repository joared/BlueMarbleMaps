#ifndef MAP_CONFIGURATION_20COPY
#define MAP_CONFIGURATION_20COPY
#ifndef MAP_CONFIGURATION
#define MAP_CONFIGURATION

#include "Map.h"

using namespace BlueMarble;

void setupAirPlaneLayerVisualization(BlueMarble::Layer& layer)
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


    layer.visualizers().push_back(symVis1);
    layer.hoverVisualizers().push_back(textSelHoverViz);
    layer.selectionVisualizers().push_back(textSelViz);
    //layer.effects().push_back(std::make_shared<BlueMarble::DropShadowEffect>(0.0, 10, 10, 0.5));
}

void configureMap(const MapPtr& map)
{
    ////////////////////////////////////////////////////////
    static auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/BlueMarbleMaps/geodata/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.tif");
    static auto backgroundDataSet2 = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/BlueMarbleMaps/geodata/BlueMarble.jpeg");

    static auto svenskaStader = std::make_shared<BlueMarble::CsvFileDataSet>("/home/joar/BlueMarbleMaps/geodata/svenska_stader/svenska-stader.csv");
    static auto northAmerica = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/world_geojson/northamerica_high_fixed.geo.json");
    static auto southAmerica = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/world_geojson/southamerica_high.geo.json");
    static auto world = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/world_geojson/world_high.geo.json");
    static auto continents = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/continents/continents.json");
    continents->name("Continents");
    static auto sverigeRoadsDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/svenska_vagar/hotosm_swe_roads_lines_geojson.geojson"); sverigeRoadsDataSet->name("Roads sverige");
    static auto roadsDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/roads_geojson/europe-road.geojson"); roadsDataSet->name("Roads");
    static auto svenskaLandskapDataSet = std::make_shared<BlueMarble::GeoJsonFileDataSet>("/home/joar/BlueMarbleMaps/geodata/svenska_landskap/svenska-landskap-klippt.geo.json");
    static auto markerDataSet = std::make_shared<BlueMarble::MemoryDataSet>(); markerDataSet->name("MarkerDataSet");
    static auto airPlaneDataSet = std::make_shared<BlueMarble::MemoryDataSet>(); airPlaneDataSet->name("AirPlanesDataSet");
    
    //sverigeRoadsDataSet->initialize(); // Takes very long to initialize (1.4 GB large)
    //roadsDataSet->initialize();  // very large too
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

    static auto backgroundLayer = BlueMarble::Layer();
    static auto backgroundLayer2 = BlueMarble::Layer();
    static auto geoJsonLayer = BlueMarble::Layer();
    static auto continentsLayer = BlueMarble::Layer();
    static auto roadsGeoJsonLayer = BlueMarble::Layer();
    static auto shapeFileLayer = BlueMarble::Layer();
    static auto airPlaneLayer = BlueMarble::Layer(false); setupAirPlaneLayerVisualization(airPlaneLayer);
    static auto csvLayer = BlueMarble::Layer();
    static auto sverigeLayer = BlueMarble::Layer();
    static auto debugLayer = BlueMarble::Layer();
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
    roadsGeoJsonLayer.enabledDuringQuickUpdates(false);
    csvLayer.addUpdateHandler(svenskaStader.get());
    sverigeLayer.addUpdateHandler(svenskaLandskapDataSet.get());
    airPlaneLayer.addUpdateHandler(airPlaneDataSet.get());
    
    
    map->addLayer(&backgroundLayer);
    map->addLayer(&backgroundLayer2);
    map->addLayer(&geoJsonLayer);
    map->addLayer(&continentsLayer);
    map->addLayer(&sverigeLayer);
    map->addLayer(&roadsGeoJsonLayer);
    map->addLayer(&csvLayer);
    map->addLayer(&airPlaneLayer);
    map->addLayer(&debugLayer);
    ////////////////////////////////////////////////////////OLD
}

#endif /* MAP_CONFIGURATION */


#endif /* MAP_CONFIGURATION_20COPY */
