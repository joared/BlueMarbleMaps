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

void configureMap(const MapPtr& map, bool includeBackground=false, bool includeRoads=false, bool includeAirPlanes=false)
{
    const bool asyncBackgroundReading = true;
    const std::string commonIndexPath = "../../../bluemarble_index"; // Relative to the build/bin/<debug/release>/ folder
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
    continents->initialize();
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
    auto backgroundLayer2 = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    auto geoJsonLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer()); //geoJsonLayer->asyncRead(asyncBackgroundReading);
    auto continentsLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer()); //continentsLayer->asyncRead(asyncBackgroundReading);
    auto roadsGeoJsonLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    auto shapeFileLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    auto airPlaneLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer(false)); 
    if(includeAirPlanes) setupAirPlaneLayerVisualization(airPlaneLayer);
    auto csvLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    auto sverigeLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    auto debugLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
    if (includeBackground)
    {
        backgroundDataSet->initialize();
        backgroundDataSet2->initialize();
        backgroundLayer->addDataSet(backgroundDataSet);
        backgroundLayer2->addDataSet(backgroundDataSet2);
    }
    
    double minScaleCountries = 1.0/20000000.0;
    geoJsonLayer->minScale(minScaleCountries);

    bool includeCountryPolygons = true; // TODO add parameter
    if (includeCountryPolygons) 
    {
        northAmerica->indexPath(commonIndexPath);
        southAmerica->indexPath(commonIndexPath);
        world->indexPath(commonIndexPath);
        northAmerica->initialize();
        southAmerica->initialize();
        world->initialize();
        geoJsonLayer->addDataSet(northAmerica); 
        geoJsonLayer->addDataSet(southAmerica); 
        geoJsonLayer->addDataSet(world);
    }

    continentsLayer->maxScale(minScaleCountries);
    continentsLayer->addDataSet(continents);
    if (includeRoads)
    {
        roadsDataSet->indexPath(commonIndexPath);
        roadsDataSet->initialize();

        // FeatureQuery featureQuery;
        // featureQuery.area(Rectangle(-180, -90, 180, 90));
        // auto features = roadsDataSet->getFeatures(featureQuery);
        // int n = features->size();
        // BMM_DEBUG() << "NUMBER OF ROAD FEATURES: " << n << "\n";

        roadsGeoJsonLayer->addDataSet(roadsDataSet);
        //sverigeRoadsDataSet->indexPath(commonIndexPath);
        //sverigeRoadsDataSet->initialize(); // Takes very long to initialize (1.4 GB large)
        //roadsGeoJsonLayer->addDataSet(sverigeRoadsDataSet);
        
        roadsGeoJsonLayer->minScale(1.0/2000000.0);
        //roadsGeoJsonLayer->enabledDuringQuickUpdates(false);
    }
    
    csvLayer->addDataSet(svenskaStader);
    sverigeLayer->addDataSet(svenskaLandskapDataSet);
    airPlaneLayer->addDataSet(airPlaneDataSet);
    
    
    map->addLayer(backgroundLayer);
    map->addLayer(backgroundLayer2);
    map->addLayer(geoJsonLayer);
    map->addLayer(continentsLayer);
    //map->addLayer(sverigeLayer);
    map->addLayer(roadsGeoJsonLayer);
    //map->addLayer(csvLayer);
    map->addLayer(airPlaneLayer);
    //map->addLayer(debugLayer);
    ////////////////////////////////////////////////////////OLD
}

#endif /* MAP_CONFIGURATION */
