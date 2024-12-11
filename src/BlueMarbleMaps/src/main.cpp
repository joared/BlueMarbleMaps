#include <iostream>
#include <CImg.h>

#include "Map.h"
#include "DataSet.h"
#include "Core.h"
#include "Feature.h"
#include "MapControl.h"

#include "DefaultEventHandlers.h"

using namespace cimg_library;
using namespace BlueMarble;

class CImgMapControl : public MapControl
{
    public:
        CImgMapControl(CImgDisplay& display)
            : m_disp(display)
        {}

        bool getResize(int& width, int& height) override final
        {
            if (m_disp.is_resized() || m_disp.is_keyF11())
            {
                std::cout << "Resize: " << m_disp.window_width() << ", " << m_disp.window_height() << "\n";
                width = m_disp.window_width();
                height = m_disp.window_height();
                
                return true;
            }

            return false;
        }

        int getWheelDelta() override final
        {
            int delta = m_disp.wheel();
            m_disp.set_wheel();
            return delta;
        }

        // TODO: remove this and use keyCode instead
        bool captureKeyEvents() override final
        {
            handleKey(m_disp.is_keyARROWDOWN(), KeyButton::ArrowDown);
            handleKey(m_disp.is_keyARROWUP(), KeyButton::ArrowUp);
            handleKey(m_disp.is_keyARROWLEFT(), KeyButton::ArrowLeft);
            handleKey(m_disp.is_keyARROWRIGHT(), KeyButton::ArrowRight);
            handleKey(m_disp.is_keyENTER(), KeyButton::Enter);
            handleKey(m_disp.is_keySPACE(), KeyButton::Space);
            handleKey(m_disp.is_keyBACKSPACE(), KeyButton::BackSpace);
            handleKey(m_disp.is_key1(), KeyButton::One);
            handleKey(m_disp.is_key2(), KeyButton::Two);
            handleKey(m_disp.is_key3(), KeyButton::Three);
            handleKey(m_disp.is_key4(), KeyButton::Four);
            handleKey(m_disp.is_key5(), KeyButton::Five);
            handleKey(m_disp.is_key6(), KeyButton::Six);
            handleKey(m_disp.is_key7(), KeyButton::Seven);
            handleKey(m_disp.is_key8(), KeyButton::Eight);
            handleKey(m_disp.is_key9(), KeyButton::Nine);
            handleKey(m_disp.is_keyPADADD(), KeyButton::Add);
            handleKey(m_disp.is_keyPADSUB(), KeyButton::Subtract);

            return true;
        }
        void getMousePos(ScreenPos &pos) const override final
        {
            pos.x = m_disp.mouse_x();
            pos.y = m_disp.mouse_y();
        }

        ModificationKey getModificationKeyMask() const override final
        {
            ModificationKey keyMask = ModificationKeyNone;
            if (m_disp.is_keySHIFTLEFT() || m_disp.is_keySHIFTRIGHT())
            {
                keyMask = keyMask | ModificationKeyShift;
            }

            if (m_disp.is_keyCTRLLEFT() || m_disp.is_keyCTRLRIGHT())
            {
                keyMask = keyMask | ModificationKeyCtrl;
            }

            if (m_disp.is_keyALT())
            {
                keyMask = keyMask | ModificationKeyAlt;
            }

            return keyMask;

        }
        MouseButton getMouseButton() override final
        {
            unsigned int button = m_disp.button();
            MouseButton mouseButton = MouseButtonNone;
            if (button & 0x1)
            {
                // Left mouse button
                mouseButton = MouseButtonLeft;
            }
            else if (button & 0x2)
            {
                // Right mouse button
                mouseButton = MouseButtonRight;
            }
            else if (button & 0x4)
            {
                // Middle mouse button
                mouseButton = MouseButtonMiddle;
            }

            return mouseButton;
        }

        void* getWindow() override final
        {
            return (void*)&m_disp;
        }
    private:
        CImgDisplay& m_disp;
};
typedef std::shared_ptr<CImgMapControl> CImgMapControlPtr;


int main()
{
    // Set up window/display
    BlueMarble::MapPtr map = std::make_shared<Map>();
    int normalization = 0;
    CImgDisplay display(cimg_library::CImg<unsigned char>(), "BlueMarbleMaps Demo", normalization, true, true); //*static_cast<CImgDisplay*>(map->drawable()->getDisplay());
    display.resize(500, 500, true);

    // Test RasterVisualizer
    auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/NE1_50M_SR_W/NE1_50M_SR_W.tif");
    backgroundDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
    auto backgroundLayer = BlueMarble::Layer(false);
    auto rasterVis1 = std::make_shared<RasterVisualizer>();
    rasterVis1->alpha(DirectDoubleAttributeVariable(1.0));
    backgroundLayer.visualizers().push_back(rasterVis1);
    backgroundLayer.addUpdateHandler(backgroundDataSet.get());
    map->addLayer(&backgroundLayer);

    // auto elevationDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/elevation/LARGE_elevation.jpg");
    // elevationDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
    // auto elevationLayer = BlueMarble::Layer(false);
    // elevationLayer.addUpdateHandler(elevationDataSet.get());
    // auto rasterVis = std::make_shared<RasterVisualizer>();
    // rasterVis->alpha(DirectDoubleAttributeVariable(0.5));
    // elevationLayer.visualizers().push_back(rasterVis);
    // map->addLayer(&elevationLayer);

    // Test Polygon/Line/Symbol visualizers
    auto vectorDataSet = std::make_shared<BlueMarble::MemoryDataSet>();
    vectorDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
    auto vectorLayer = BlueMarble::Layer(true);
    vectorLayer.addUpdateHandler(vectorDataSet.get());
    
    std::vector<Point> points({{16, 56}, {17, 57}, {15, 58}});
    auto poly = std::make_shared<PolygonGeometry>(points);
    vectorDataSet->addFeature(vectorDataSet->createFeature(poly));

    map->addLayer(&vectorLayer);

    // Setup MapControl and event handlers
    CImgMapControlPtr mapControl = std::make_shared<CImgMapControl>(display);
    auto panHandler = BlueMarble::PanEventHandler(*map, mapControl);
    mapControl->addSubscriber(&panHandler);
    mapControl->setView(map);
    
    // Main loop
    map->startInitialAnimation();
    map->update();
    while (!display.is_closed() && !display.is_keyESC()) 
    {
        mapControl->captureEvents();
        if (display.is_resized() || display.is_keyF11())
        {
            display.resize(false);
        }


        if (mapControl->updateRequired())
        {
            mapControl->updateViewInternal();
        }
        else
        {
            display.wait();
        }
    }
    
    return 0;
}