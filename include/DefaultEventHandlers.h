#ifndef DEFAULT_EVENT_HANDLERS
#define DEFAULT_EVENT_HANDLERS

#include "CImgEventManager.h" // TODO: remove, needed for a lot of types
#include "EventHandler.h"
#include "Map.h"
#include "Core.h"
#include "GeoGuessGame.h"



namespace BlueMarble
{
    class DrawSomeEventHandler
        : public BlueMarble::EventHandler
        , public BlueMarble::MapEventHandler
    {
        public:
            DrawSomeEventHandler(BlueMarble::Map& map) 
                : m_map(map)
            {

            }

            void OnAreaChanged(BlueMarble::Map& /*map*/) override final 
            {

            }

            void OnUpdating(BlueMarble::Map& /*map*/) override final 
            {

            }

            void drawRect(const BlueMarble::Rectangle& bounds)
            {

            }

            void OnCustomDraw(BlueMarble::Map& /*map*/) override final 
            {
                // auto& drawable = m_map.drawable(); // TODO: add reference again
                // auto lineColor = BlueMarble::Color{0, 0, 0};
                // auto fillColor = BlueMarble::Color{255, 255, 255, 0.5};
            }

            void OnUpdated(BlueMarble::Map& /*map*/) override final 
            {

            }

            bool OnClick(const ClickEvent& event)
            {

                return true;
            }

        private:
            BlueMarble::Map& m_map;
    };

    class PolygonEventHandler 
        : public BlueMarble::EventHandler
        , public BlueMarble::MapEventHandler
    {
        public:
            PolygonEventHandler(Map& map)
                : m_map(map)
            {
                m_map.addMapEventHandler(this);
            }

            void OnAreaChanged(BlueMarble::Map& /*map*/) override final 
            {

            }

            void OnUpdating(BlueMarble::Map& /*map*/) override final 
            {

            }

            void OnCustomDraw(BlueMarble::Map& /*map*/) override final 
            {
                for (auto& polygon : m_polygons)
                {
                    //std::cout << "OnCustomDraw\n";
                    if (polygon.size() > 1)
                    {
                        // Draw line
                        auto& drawable = m_map.drawable();

                        drawable.drawLine(m_map.mapToScreen(polygon), Color(255, 255, 255));
                    }

                    if (polygon.size() > 2)
                    {
                        // Draw polygon
                        auto& drawable = m_map.drawable();
                        drawable.drawPolygon(m_map.mapToScreen(polygon), Color(0, 0, 0, 0.5));
                    }
                }
                std::cout << "Drawing polygon\n";
            }

            void OnUpdated(BlueMarble::Map& /*map*/) override final 
            {

            }

            bool OnClick(const ClickEvent& event)
            {
                if (m_polygons.empty())
                    return false;

                auto& polygon = m_polygons[m_polygons.size()-1];
                Point p = m_map.screenToMap(event.pos.x, event.pos.y);
                polygon.push_back(p);
                m_map.update(true);
                return true;
            }

            bool OnKeyDown(const KeyDownEvent& event)
            {
                switch (event.keyButton)
                {
                case KeyButton::Space:
                    // add polygon
                    m_polygons.push_back(Polygon());
                    std::cout << "New polygon\n";
                    break;
                case KeyButton::BackSpace:
                    /* code */
                    if (!m_polygons.empty())
                    {
                        m_polygons.erase(m_polygons.end() - 1);
                        std::cout << "Delete polygon\n";
                        m_map.update(true);
                    }
                    break;
                default:
                    break;
                }
            }

        private:
            Map& m_map;
            typedef std::vector<Point> Polygon;
            std::vector<Polygon> m_polygons;
    };

    class PanEventHandler 
        : public BlueMarble::EventHandler
        , public BlueMarble::MapEventHandler
    {
        public:
            PanEventHandler(BlueMarble::Map& map, DataSet& geoGuessDataSet) 
                : m_map(map)
                , m_rectangle(BlueMarble::Rectangle::undefined())
                , m_zoomToRect(false)
                , m_inertiaOption{0.002, 0.1}
                , m_funnyDudeRaster("/home/joar/BlueMarbleMaps/geodata/symbols/funny_dude.png")
                , m_geoGuessGame(map, geoGuessDataSet)
            {
                m_cutoff = 300;
                int reserveAmount = (int)(m_cutoff / 16.0) + 1;
                m_positions.reserve(reserveAmount);
                m_timeStamps.reserve(reserveAmount);

                m_map.addMapEventHandler(this);
            }
            
            void OnAreaChanged(BlueMarble::Map& /*map*/) override final 
            {

            }

            void OnUpdating(BlueMarble::Map& /*map*/) override final 
            {

            }

            void drawRect(const BlueMarble::Rectangle& bounds)
            {
                auto& drawable = m_map.drawable(); // TODO: add reference again
                auto lineColor = BlueMarble::Color{0, 0, 0};
                auto fillColor = BlueMarble::Color{255, 255, 255, 0.5};
                

                std::cout << "drawRect\n";
                drawable.drawRect(bounds, fillColor);
                std::cout << "drawRect exit\n";

                drawable.drawLine(bounds.corners(), lineColor); 
            }

            void OnCustomDraw(BlueMarble::Map& /*map*/) override final 
            {
                
                // Geo guess game
                if (!m_geoGuessGame.isStarted())
                {
                    auto bounds = m_map.area();
                    bounds.scale(0.5); // TODO: this should not be hardcoded here, should be part of game
                    int x = m_map.drawable().width() / 2.0;
                    std::string info;
                    if (m_geoGuessGame.isFinnished())
                        info += "Finished in " + std::to_string(m_geoGuessGame.elapsedMs() / 1000.0) + " s!\n";
                    info += "Press Enter to start";
                    m_map.drawable().drawText(x, 10, info, Color::blue());
                    m_map.drawable().drawRect(m_map.mapToScreen(bounds), Color::red(0.1));

                    return;
                }

                // Draw the bounds of the features in the game
                auto gameBounds = m_geoGuessGame.bounds();
                gameBounds = m_map.mapToScreen(gameBounds);
                int lineWidth = 10;
                gameBounds.extend(lineWidth, lineWidth);
                auto boundingLine = gameBounds.corners();
                m_map.drawable().drawLine(boundingLine, Color::red(0.5), lineWidth);

                // Draw informational text
                auto currCountry = m_geoGuessGame.currentCountryName();
                std::string info = "Guesses: " + std::to_string(m_geoGuessGame.nCorrect()) + "/" + std::to_string(m_geoGuessGame.nTot()) + "\n";
                info += "Find '" + currCountry + "'";
                int x = m_map.drawable().width() / 2.0;
                m_map.drawable().drawText(x, 10, info, Color::blue());

                // Zoom to rect
                //std::cout << "OnCustomDraw\n";
                
                // FIXME: Funny dude: Very slow when zoomed in?!?!
                // auto temp = m_funnyDudeRaster;
                // double s = m_map.scale()*0.01;
                // temp.resize((double)temp.width()*s, (double)temp.height()*s);
                // auto p = Point(13.57, 47.56);
                // p = m_map.lngLatToScreen(p);
                // m_map.drawable().drawRaster(p.x(), p.y(), temp, 0.1);

                if (m_rectangle.isUndefined())
                    return;

                std::cout << "Drawing rectangle\n";
                drawRect(m_map.mapToScreen(m_rectangle));
            }

            void OnUpdated(BlueMarble::Map& /*map*/) override final 
            {

            }

            bool OnKeyDown(const BlueMarble::KeyDownEvent& event)
            {
                BlueMarble::Point direction;
                switch (event.keyButton)
                {
                
                case BlueMarble::KeyButton::ArrowDown:
                    direction = BlueMarble::Point(0, 1);
                    break;

                case BlueMarble::KeyButton::ArrowUp:
                    direction = BlueMarble::Point(0, -1);
                    break;
                
                case BlueMarble::KeyButton::ArrowLeft:
                    direction = BlueMarble::Point(-1, 0);
                    break;
                
                case BlueMarble::KeyButton::ArrowRight:
                    direction = BlueMarble::Point(1, 0);
                    break;
                
                case BlueMarble::KeyButton::Space:
                {
                    if (m_map.selected().size() > 0)
                    {
                        auto boundsList = std::vector<Rectangle>();
                        for (auto id : m_map.selected()) 
                        { 
                            auto f = m_map.getFeature(id);
                            if (f && !f->bounds().isUndefined())
                                boundsList.push_back(f->bounds()); 
                        }
                        Rectangle bounds = Rectangle::mergeBounds(boundsList);
                        if (!bounds.isUndefined())
                        {
                            auto rect = m_map.lngLatToMap(bounds);
                            m_map.zoomToArea(rect, true);
                        }
                    }
                    return true;
                }

                case KeyButton::Enter:
                {
                    if (!m_geoGuessGame.isStarted())
                    {
                        auto bounds = m_map.area();
                        bounds.scale(0.5);
                        m_geoGuessGame.start(m_map.mapToLngLat(bounds));
                        std::cout << "Started game!\n";
                    }
                    else
                    {
                        m_geoGuessGame.stop();
                    }
                    std::cout << "Stopped game!\n";
                    m_map.update(true);
                }

                default:
                    if (BlueMarble::isNumberKey(event.keyButton))
                    {
                        int i = numberKeyToInt(event.keyButton);
                        std::cout << "Layers: " << m_map.layers().size() << "\n";
                        std::cout << "Number: " << i << "\n";
                        if (i <= m_map.layers().size())
                        {
                            bool enabled = m_map.layers()[i-1]->enabled();
                            enabled = !enabled;
                            m_map.layers()[i-1]->enabled(enabled);
                            std::cout << "Enabled layer " << i << " (" << enabled << "\n";
                            m_map.update(true);
                            return true;
                        }
                    }
                    return false;
                }

                if (event.modificationKey & ModificationKeyCtrl)
                {
                    m_map.rotation(m_map.rotation() + direction.x()*10.0);
                }
                else
                {
                    m_map.panBy(direction * 50.0, true);
                }
                

                return true;
            }

            bool OnMouseDown(const BlueMarble::MouseDownEvent& /*mouseDownEvent*/) override final
            {
                m_map.stopAnimation();
                m_map.update(true);
                return true;
            }

            bool OnMouseMove(const BlueMarble::MouseMoveEvent& event) override final
            {
                auto pObjs = m_map.hitTest(event.pos.x, event.pos.y, 10.0);
                BlueMarble::Id hoverId(0,0);
                if (pObjs.size() > 0)
                {
                    hoverId = pObjs[0].sourceFeature()->id();
                }

                if (!m_map.isHovered(hoverId))
                {
                    m_map.hover(hoverId);
                    m_map.update(true);
                }
                return true;
            }

            bool OnClick(const BlueMarble::ClickEvent& event) override final
            {
                if (m_geoGuessGame.isStarted())
                {
                    m_geoGuessGame.onClick(event.pos.x, event.pos.y);
                    return true;
                }

                auto pObjs = m_map.hitTest(event.pos.x, event.pos.y, 10.0);
                Id selId(0,0);
                FeaturePtr selFeat(nullptr);
                if (pObjs.size() > 0)
                {
                    selId = pObjs[0].sourceFeature()->id();
                    selFeat = pObjs[0].sourceFeature();
                }
                else
                {
                    m_map.deSelectAll();
                    m_map.update(true);

                    return true;
                }

                auto mode = (event.modificationKey == ModificationKeyCtrl) 
                        ? SelectMode::Add : SelectMode::Replace;
                if (!m_map.isSelected(selFeat))
                {   
                    m_map.select(selFeat, mode);
                }
                else if (mode == SelectMode::Replace)
                {
                    if (m_map.selected().size() == 1)
                    {
                        m_map.deSelect(selFeat);
                    }
                    else if (m_map.selected().size() > 1)
                    {
                        m_map.select(selFeat, SelectMode::Replace);
                    }
                }
                else // SelectMode::Add
                {
                    m_map.deSelect(selFeat);
                }
                
                m_map.update(true);

                return true;
            }

            bool OnDoubleClick(const BlueMarble::DoubleClickEvent& event) override final
            {
                if (m_geoGuessGame.isStarted())
                {
                    m_geoGuessGame.onClick(event.pos.x, event.pos.y);
                    return true;
                }
                    
                double zoomFactor = 2.5;
                auto mapPoint = m_map.screenToMap(event.pos.x, event.pos.y);

                auto hitFeatures = m_map.featuresAt(event.pos.x, event.pos.y, 10.0);
                
                if (hitFeatures.size() > 0)
                {
                    auto f = hitFeatures[0];
                    m_map.select(hitFeatures[0], SelectMode::Add);
                    if (!f->bounds().isUndefined())
                    {
                        auto rect = m_map.lngLatToMap(f->bounds());
                        m_map.zoomToArea(rect, true);
                        return true;
                    }
                    mapPoint = m_map.lngLatToMap(hitFeatures[0]->center());
                    zoomFactor *= 2;
                }

                if (!m_rectangle.isUndefined() && m_rectangle.isInside(mapPoint))
                {
                    // animate to rectangle
                    // auto to = m_rectangle.center();
                    
                    // auto animation = BlueMarble::Animation::Create(m_map, m_map.center(), to, m_map.scale(), m_map.scale()*zoomFactor, 500, false);
                    // m_map.startAnimation(animation);
                    m_map.zoomToArea(m_rectangle, true);
                }
                else
                {
                    m_map.zoomOn(mapPoint, zoomFactor, true);
                }

                return true;
            }

            bool OnDragBegin(const BlueMarble::DragBeginEvent& dragBeginEvent) override final
            {
                // std::cout << dragBeginEvent.toString()<< " (" << dragBeginEvent.pos.x << ", " << dragBeginEvent.pos.y << ")\n";
                addMousePos(dragBeginEvent.pos, dragBeginEvent.timeStampMs);
                m_debugTime = dragBeginEvent.timeStampMs;
                return true;
            }

            bool OnDrag(const BlueMarble::DragEvent& dragEvent) override final
            {
                // std::cout << dragEvent.toString()<< " (" << dragEvent.pos.x << ", " << dragEvent.pos.y << ") Elapsed: " << dragEvent.timeStampMs - m_debugTime << "\n";
                m_debugTime = dragEvent.timeStampMs;

                addMousePos(dragEvent.pos, dragEvent.timeStampMs);
                prunePositions(dragEvent.timeStampMs);

                switch (dragEvent.mouseButton)
                {
                case BlueMarble::MouseButtonLeft:
                    {
                        if (dragEvent.modificationKey & BlueMarble::ModificationKeyCtrl)
                        {
                            // zoom to rect
                            std::cout << "Mod key: " << dragEvent.modificationKey << "\n";
                            m_zoomToRect = true;
                            auto rect = BlueMarble::Rectangle(dragEvent.startPos.x, dragEvent.startPos.y, dragEvent.pos.x, dragEvent.pos.y);
                            m_rectangle = m_map.screenToMap(rect);
                            m_map.update(true);
                        }
                        else
                        {
                            m_map.panBy({(double)(dragEvent.lastPos.x - dragEvent.pos.x), 
                                        (double)(dragEvent.lastPos.y - dragEvent.pos.y)});
                        }
                        
                        break;
                    }
                case BlueMarble::MouseButtonRight:
                    {
                        const double ZOOM_SCALE = 0.01;
                        auto mapPoint = m_map.screenToMap(BlueMarble::Point(dragEvent.startPos.x, dragEvent.startPos.y));
                        double deltaY = dragEvent.pos.y - dragEvent.lastPos.y;
                        double scale = 1 + abs(deltaY)*ZOOM_SCALE;
                        double zoomFactor = deltaY > 0 ? scale : 1.0/scale;
                        m_map.zoomOn(mapPoint, zoomFactor);
                        break;
                    }
                case BlueMarble::MouseButtonMiddle:
                    {
                        auto corner1 = m_map.screenToMap(dragEvent.pos.x, dragEvent.pos.y);
                        auto corner2 = m_map.screenToMap(dragEvent.startPos.x, dragEvent.startPos.y);
                        m_rectangle = BlueMarble::Rectangle(corner1.x(), corner1.y(), corner2.x(), corner2.y());
                        std::cout << "Rectangle set: " << std::to_string(m_rectangle.xMin()) << ", " << std::to_string(m_rectangle.yMin()) << ", " << std::to_string(m_rectangle.xMax()) << ", " << std::to_string(m_rectangle.yMax()) << "\n";
                        m_map.update(true);
                        break;
                    }
                default:
                    break;
                }

                return true;
            }

            bool OnDragEnd(const BlueMarble::DragEndEvent& dragEndEvent) override final
            {
                // std::cout << dragEndEvent.toString()<< " (" << dragEndEvent.pos.x << ", " << dragEndEvent.pos.y << ")\n";

                if (m_zoomToRect)
                {
                    m_zoomToRect = false;
                    auto topLeft = m_map.screenToMap(BlueMarble::Point(dragEndEvent.startPos.x, dragEndEvent.startPos.y));
                    auto bottomRight = m_map.screenToMap(BlueMarble::Point(dragEndEvent.pos.x, dragEndEvent.pos.y));
                    auto rect = BlueMarble::Rectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
                    m_map.zoomToArea(rect, true);
                    m_rectangle = BlueMarble::Rectangle::undefined();
                    
                    return true;
                }

                switch (dragEndEvent.mouseButton)
                {
                case BlueMarble::MouseButtonLeft:
                    {
                        prunePositions(dragEndEvent.timeStampMs);
                        auto velocity = calculateVelocity();
                        m_timeStamps.clear();
                        m_positions.clear();

                        std::cout << "Velocity: " << std::to_string(velocity.x()) << ", " << std::to_string(velocity.y()) << "\n";

                        double linearity = m_inertiaOption.linearity;
                        double alpha = m_inertiaOption.alpha;
                        velocity = velocity * linearity * 2.0; // * 2.0 is Temp test
                        double speed = velocity.length();
                        if (speed <= 0)
                        {
                            return true;
                        }
                            
                        double animationDuration = speed / (alpha * linearity);
                        auto offset = velocity * (-animationDuration / 2.0);
                        auto to = m_map.screenToMap(m_map.screenCenter() + offset);

                        auto animation = BlueMarble::Animation::Create(m_map, 
                                                                    m_map.center(), 
                                                                    to, 
                                                                    animationDuration, 
                                                                    true, 
                                                                    m_inertiaOption);

                        m_map.startAnimation(animation);
                    }
                    break;
                
                default:
                    break;
                }


                return true;
            }

            bool OnMouseWheel(const BlueMarble::MouseWheelEvent& wheelEvent) override final
            {
                const double WHEEL_DELTA = 5;

                double scale = 1.0 + abs(wheelEvent.delta)/WHEEL_DELTA;
                double zoomFactor = wheelEvent.delta > 0 ? scale : 1.0/scale;
                std::cout << "Wheel delta: " << wheelEvent.delta << "\n";
                std::cout << "Zoom factor: " << zoomFactor << "\n";
                m_map.zoomOn(m_map.screenToMap(BlueMarble::Point(wheelEvent.pos.x, wheelEvent.pos.y)), zoomFactor, true);

                return true;
            }

            void addMousePos(const BlueMarble::ScreenPos& pos, int timeStamp)
            {
                m_positions.push_back(pos);
                m_timeStamps.push_back(timeStamp);
            }

            void prunePositions(int timeStamp)
            {
                int i = 0;
                while (i < (int)m_timeStamps.size() && timeStamp - m_timeStamps[0] > m_cutoff)
                {
                    m_timeStamps.erase(m_timeStamps.begin() + i);
                    m_positions.erase(m_positions.begin() + i);
                }
            }

            BlueMarble::Point calculateVelocity()
            {   
                if (m_timeStamps.size() < 2)
                {
                    std::cout << "Not enough recorded positions (" << m_timeStamps.size() << ")\n";
                    return BlueMarble::Point(0, 0);
                }
                    
                int size = m_timeStamps.size();
                int deltaTime = m_timeStamps[size -1] - m_timeStamps[0];
                BlueMarble::Point deltaPos = BlueMarble::Point(m_positions[size-1].x, m_positions[size-1].y) 
                                            - BlueMarble::Point(m_positions[0].x, m_positions[0].y);

                return deltaPos * (1.0/deltaTime);
            }

            bool handleEvent(const BlueMarble::Event& event) override
            {
                // Debug output event info
                // const BlueMarble::PointerEvent& pointerEvent = static_cast<const BlueMarble::PointerEvent&>(event);
                // std::string button = BlueMarble::mouseButtonToString(pointerEvent.mouseButton);
                // std::cout << button << event.toString()<< " (" << pointerEvent.pos.x << ", " << pointerEvent.pos.y << ")\n";

                return EventHandler::handleEvent(event);
            }
        private:
            BlueMarble::Map& m_map;
            std::vector<int> m_timeStamps;
            std::vector<BlueMarble::ScreenPos> m_positions;
            int m_cutoff;
            BlueMarble::InertiaOptions m_inertiaOption;
            BlueMarble::Rectangle m_rectangle;
            int m_debugTime;
            bool m_zoomToRect;
            Raster m_funnyDudeRaster;
            GeoGuesGame m_geoGuessGame;
    };

}
#endif /* DEFAULT_EVENT_HANDLERS */
