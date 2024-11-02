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
    // Create a display
    CImg<unsigned char> dummy; //("/home/joar/BlueMarbleMaps/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.tif");
    CImgDisplay display(dummy,"BlueMarbleMaps Demo", 3, true, true);
    //display.resize(3373, 1412, true); // Full screen doesnt work, using this temporarily
    display.resize(1500, 900, true);

    // New
    BlueMarble::Map map(display);
    
    auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/NE1_50M_SR_W/NE1_50M_SR_W.tif");
    
    backgroundDataSet->initialize(DataSetInitializationType::RightHereRightNow);

    auto backgroundLayer = BlueMarble::Layer();
    backgroundLayer.addUpdateHandler(backgroundDataSet.get());

    
    map.addLayer(&backgroundLayer);

    // Setup event manager and event handlers
    BlueMarble::CImgEventManager eventManager(display);
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