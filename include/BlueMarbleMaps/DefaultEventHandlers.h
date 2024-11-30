#ifndef DEFAULTEVENTHANDLERS
#define DEFAULTEVENTHANDLERS

#include "CImgEventManager.h" // TODO: remove, needed for a lot of types
#include "EventHandler.h"
#include "Map.h"
#include "Core.h"

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
                        auto& drawable = *m_map.drawable();

                        drawable.drawLine(m_map.mapToScreen(polygon), Color(255, 255, 255));
                    }

                    if (polygon.size() > 2)
                    {
                        // Draw polygon
                        auto& drawable = *m_map.drawable();
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
        , public IFeatureEventListener
    {
        public:
            PanEventHandler(BlueMarble::Map& map) 
                : m_map(map)
                , m_rectangle(BlueMarble::Rectangle::undefined())
                , m_zoomToRect(false)
                , m_inertiaOption{0.002, 0.1}
                , m_dataSetsInitialized(false)
            {
                m_cutoff = 150;
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
                auto& drawable = *m_map.drawable(); // TODO: add reference again
                auto lineColor = BlueMarble::Color{0, 0, 0};
                auto fillColor = BlueMarble::Color{255, 255, 255, 0.2};
                

                std::cout << "drawRect\n";
                drawable.drawRect(bounds, fillColor);
                std::cout << "drawRect exit\n";

                auto line = bounds.corners();
                line.push_back(line[0]);
                drawable.drawLine(line, lineColor); 
            }

            void OnCustomDraw(BlueMarble::Map& /*map*/) override final 
            {
                if (!m_rectangle.isUndefined())
                    drawRect(m_map.mapToScreen(m_rectangle));
            }

            void OnUpdated(BlueMarble::Map& /*map*/) override final 
            {

            }

            void onFeatureCreated(const FeaturePtr& /*feature*/) override final {}
            void onFeatureUpdated(const FeaturePtr& feature) override final
            {
                // Perform zoom/pan in OnUpdated instead
            }
            void onFeatureDeleted(const Id& id) override final
            {

            }

            bool OnResize(const BlueMarble::ResizeEvent& event) override
            {
                std::cout << "Fucking resize!: " << event.width << ", " << event.height << "\n";
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
                
                case BlueMarble::KeyButton::BackSpace:
                    if (m_map.undoCommand())
                        return true;
                    break;
                case BlueMarble::KeyButton::Space:
                {                    
                    if (m_map.selected().size() > 0)
                    {
                        // Get a list of all boundary points for all features 
                        auto pointList = std::vector<Point>();
                        for (auto id : m_map.selected()) 
                        { 
                            auto f = m_map.getFeature(id);
                            if (f && !f->bounds().isUndefined())
                            {
                                for (auto p: f->bounds().corners())
                                {
                                    pointList.push_back(p);
                                }
                            }
                            else if (auto pointGeometry = f->geometryAsPoint())
                            {
                                pointList.push_back(pointGeometry->point());
                            }
                        }

                        Rectangle bounds = Rectangle::undefined();
                        if (pointList.size() == 1)
                        {
                            auto area = m_map.mapToLngLat(m_map.area());
                            area.center(pointList[0]);
                            bounds = m_map.lngLatToMap(area);
                        }
                        else if (pointList.size() > 1)
                        {
                            bounds = m_map.lngLatToMap(Rectangle::fromPoints(pointList));
                            // Add some margin
                            bounds.extend(50.0/m_map.scale(), 50.0/m_map.scale());
                        }

                        
                        if (!bounds.isUndefined())
                        {
                            //m_map.zoomToArea(rect, true);
                            // TODO
                            m_map.doCommand([this, bounds]() 
                            {
                                this->m_map.zoomToArea(bounds, true);
                            }
                            );
                            
                        }
                    }
                    return true;
                }

                case KeyButton::Enter:
                {   
                    //m_map.update(true);
                    return true;
                }

                case KeyButton::Add:
                {
                    if (event.modificationKey & ModificationKeyCtrl)
                    {
                        m_inertiaOption.linearity += 0.01;
                    }
                    else
                    {
                        if (m_inertiaOption.alpha < 0.001)
                            m_inertiaOption.alpha += 0.0001;
                        else
                            m_inertiaOption.alpha += 0.001;
                    }
                    m_inertiaOption.alpha = std::max(m_inertiaOption.alpha, 0.0001);
                    m_inertiaOption.linearity = std::max(m_inertiaOption.linearity, 0.01);
                    std::cout << "Linearity: " << m_inertiaOption.linearity << "\n";
                    std::cout << "Alpha: " << m_inertiaOption.alpha << "\n";
                    return true;
                }
                case KeyButton::Subtract:
                {
                    if (event.modificationKey & ModificationKeyCtrl)
                    {
                        m_inertiaOption.linearity -= 0.01;
                    }
                    else
                    {
                        if (m_inertiaOption.alpha < 0.001)
                            m_inertiaOption.alpha -= 0.0001;
                        else
                            m_inertiaOption.alpha -= 0.001;
                    }

                    m_inertiaOption.alpha = std::max(m_inertiaOption.alpha, 0.0001);
                    m_inertiaOption.linearity = std::max(m_inertiaOption.linearity, 0.01);
                    std::cout << "Linearity: " << m_inertiaOption.linearity << "\n";
                    std::cout << "Alpha: " << m_inertiaOption.alpha << "\n";
                    return true;
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
                BlueMarble::FeaturePtr hoverFeature(nullptr);
                if (pObjs.size() > 0)
                {
                    hoverFeature = pObjs[0].sourceFeature();
                }

                if (!m_map.isHovered(hoverFeature))
                {
                    // std::cout << "Not hovered\n";
                    m_map.hover(hoverFeature);
                    m_map.update(true);
                }
                return true;
            }

            bool OnClick(const BlueMarble::ClickEvent& event) override final
            {
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
                std::cout << dragBeginEvent.toString()<< " (" << dragBeginEvent.pos.x << ", " << dragBeginEvent.pos.y << ")\n";
                addMousePos(dragBeginEvent.pos, dragBeginEvent.timeStampMs);
                m_debugTime = dragBeginEvent.timeStampMs;
                m_map.quickUpdateEnabled(true); // TODO: make an interaction handler that manages if this is enabled or not?
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
                            m_map.update(true);
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
                        m_map.update(true);
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
                std::cout << dragEndEvent.toString()<< " (" << dragEndEvent.pos.x << ", " << dragEndEvent.pos.y << ")\n";
                m_map.quickUpdateEnabled(false);
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

                        // std::cout << "Velocity: " << std::to_string(velocity.x()) << ", " << std::to_string(velocity.y()) << "\n";

                        double linearity = m_inertiaOption.linearity;
                        double alpha = m_inertiaOption.alpha;
                        double maxSpeed = m_inertiaOption.maxSpeed;
                        velocity = velocity * linearity; // * 2.0 is Temp test
                        std::cout << "Speed: " << velocity.length() << "\n";
                        double speed = std::min(velocity.length(), maxSpeed);
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
                m_map.update(true);

                return true;
            }

            bool OnMouseWheel(const BlueMarble::MouseWheelEvent& wheelEvent) override final
            {
                const double WHEEL_DELTA = 5;

                double scale = 1.0 + abs(wheelEvent.delta)/WHEEL_DELTA;
                double zoomFactor = wheelEvent.delta > 0 ? scale : 1.0/scale;
                bool animate = false; //abs(wheelEvent.delta) > 1; // only animate if wheel delta is large enough
                m_map.zoomOn(m_map.screenToMap(BlueMarble::Point(wheelEvent.pos.x, wheelEvent.pos.y)), zoomFactor, animate);
                m_map.update(true);
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
                    
                int size = (int)m_timeStamps.size();
                int diffIdx = size-1;
                int deltaTime = m_timeStamps[diffIdx] - m_timeStamps[0];
                BlueMarble::Point deltaPos = BlueMarble::Point(m_positions[diffIdx].x, m_positions[diffIdx].y) 
                                            - BlueMarble::Point(m_positions[0].x, m_positions[0].y);

                return deltaPos * (1.0/deltaTime);
            }

            bool handleEvent(const BlueMarble::Event& event) override
            {
                // Debug output event info
                // const BlueMarble::PointerEvent& pointerEvent = static_cast<const BlueMarble::PointerEvent&>(event);
                // std::string button = BlueMarble::mouseButtonToString(pointerEvent.mouseButton);
                // std::cout << button << event.toString()<< " (" << pointerEvent.pos.x << ", " << pointerEvent.pos.y << ")\n";

                if (event.getType() == EventType::Resize)
                {
                    std::cout << "Resize!\n";
                }

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
            bool m_dataSetsInitialized;
    };

}
#endif /* DEFAULTEVENTHANDLERS */
