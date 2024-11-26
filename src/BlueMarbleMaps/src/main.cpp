#include <iostream>
#include <CImg.h>

#include "Map.h"
#include "DataSet.h"
#include "Core.h"
#include "Feature.h"
#include "CImgEventManager.h"

#include "DefaultEventHandlers.h"

using namespace cimg_library;


int main()
{
    // New
    BlueMarble::Map map;
    CImgDisplay& display = *static_cast<CImgDisplay*>(map.drawable().getDisplay());
    
    auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/NE1_50M_SR_W/NE1_50M_SR_W.tif");
    
    backgroundDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);

    auto backgroundLayer = BlueMarble::Layer();
    backgroundLayer.addUpdateHandler(backgroundDataSet.get());

    
    map.addLayer(&backgroundLayer);

    // Setup event manager and event handlers
    BlueMarble::CImgEventManager eventManager(map.drawable());
    auto panHandler = BlueMarble::PanEventHandler(map);
    //auto polygonHandler = BlueMarble::PolygonEventHandler(map);
    //auto testEventHandler = BlueMarble::TestEventHandler();
    eventManager.addSubscriber(&panHandler);
    
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
    }
    
    return 0;
}