#ifndef DEFAULTEVENTHANDLERS
#define DEFAULTEVENTHANDLERS

#include "Event/EventHandler.h"
#include "Core/Map.h"
#include "Core/Core.h"
#include "Event/PointerEvent.h"
#include "Event/KeyEvent.h"
#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/AnimationFunctions.h"
#include "BlueMarbleMaps/Core/DataSets/MemoryDataSet.h"

namespace BlueMarble
{

    class EditFeatureTool
        : public Tool
    {
        public:
            EditFeatureTool()
                : Tool()
                , m_active(false)
                , m_editFeature(nullptr)
                , m_autoSelect(false)
            {}
        protected:
            bool isActive() override final
            {
                return m_editFeature != nullptr;
            }

            void onConnected(const MapControlPtr& control, const MapPtr& map) override final 
            {
                m_control = control;
                m_map = map;
            }

            void onDisconnected() override final 
            {
                m_map = nullptr;
                m_control = nullptr;
            }

            bool& autoSelect()
            {
                return m_autoSelect;
            }

            bool OnMouseDown(const MouseDownEvent& event) override final
            {
                auto& presentationObjects = m_map->hitTest(event.pos.x, event.pos.y, 10);
                for (auto& p : presentationObjects)
                {
                    if (!m_autoSelect && !m_map->isSelected(p.sourceFeature()))
                    {
                        // Not selected, continue to find a selected feature
                        continue;
                    }
                    BMM_DEBUG() << "We hit something! I am stealing events!\n";
                    m_editFeature = p.sourceFeature();
                    m_nodeIndex = p.nodeIndex();
                    return true;
                }

                return false;
            }

            bool OnDrag(const DragEvent& event) override final
            {
                if (!m_editFeature)
                {
                    return false;
                }
                BMM_DEBUG() << "I am editing!\n";

                // If auto select is enabled, select the feature
                if (m_autoSelect && !m_map->isSelected(m_editFeature))
                {
                    m_map->select(m_editFeature);
                }

                // TODO: should be projected to the feature coordinate system
                auto fromPos = m_map->screenToMap(Point{(double)event.lastPos.x, 
                                                        (double)event.lastPos.y});
                auto toPos = m_map->screenToMap(Point{(double)event.pos.x, 
                                                      (double)event.pos.y});
                auto delta = toPos-fromPos;

                if (m_nodeIndex != -1)
                {
                    BMM_DEBUG() << "Edit Node: " << m_nodeIndex << "\n";
                    auto& point = getNodePoint(m_editFeature, m_nodeIndex);
                    point = toPos;
                }
                else
                {
                    m_editFeature->move(delta);
                }

                m_map->update();

                return true;
            }

            bool OnMouseUp(const MouseUpEvent& event) override final
            {
                BMM_DEBUG() << "Mouse up! Deactivate\n";
                m_editFeature = nullptr;
                return false;
            }

        private:
            Point& getNodePoint(const FeaturePtr& feature, int nodeIndex)
            {
                if (feature->geometryType() == GeometryType::Point)
                {
                    return feature->geometryAsPoint()->point();
                }
                else if (feature->geometryType() == GeometryType::Polygon)
                {
                    // TODO: could be inner ring
                    return feature->geometryAsPolygon()->outerRing()[nodeIndex];
                }
                else if (feature->geometryType() == GeometryType::Line)
                {
                    return feature->geometryAsLine()->points()[nodeIndex];
                }
                else
                {
                    BMM_DEBUG() << "Something went wrong...\n";
                    throw std::exception();
                }
            }

            MapControlPtr m_control;
            MapPtr m_map;
            bool m_active;
            FeaturePtr m_editFeature;
            int m_nodeIndex;

            bool m_autoSelect;
    };

    class PanEventHandler 
        : public Tool
        , public IFeatureEventListener
    {
        public:
            PanEventHandler() 
                : Tool()
                , m_map(nullptr)
                , m_mapControl(nullptr)
                , m_rectangle(BlueMarble::Rectangle::undefined())
                , m_zoomToRect(false)
                , m_inertiaOption{0.002, 0.1}
                , m_drawDataSetInfo(false)
                , m_hitTestLine(std::make_shared<LineGeometry>())
                , m_hoverFeature(nullptr)
            {
                m_cutoff = 150;
                int reserveAmount = (int)(m_cutoff / 16.0) + 1;
                m_positions.reserve(reserveAmount);
                m_timeStamps.reserve(reserveAmount);
            }

            bool isActive() override final
            {
                return true;
            }

            void onConnected(const MapControlPtr& control, const MapPtr& map) override final 
            {
                m_mapControl = control;
                m_map = map;
                m_map->events.onUpdating.subscribe(this, &PanEventHandler::OnUpdating);
                m_map->events.onCustomDraw.subscribe(this, &PanEventHandler::OnCustomDraw);
                m_map->events.onUpdated.subscribe(this, &PanEventHandler::OnUpdated);
                m_map->events.onIdle.subscribe(this, &PanEventHandler::OnIdle);
            }

            void onDisconnected() override final 
            {
                m_map->events.onUpdating.unsubscribe(this);
                m_map->events.onCustomDraw.unsubscribe(this);
                m_map->events.onUpdated.unsubscribe(this);
                m_map->events.onIdle.unsubscribe(this);
                m_map = nullptr;
                m_mapControl = nullptr;
            }

            void OnIdle(BlueMarble::Map& /*map*/)
            {
                //BMM_DEBUG() << "OnIdle!\n";
            }

            void OnAreaChanged(BlueMarble::Map& /*map*/)
            {

            }

            void OnUpdating(BlueMarble::Map& /*map*/)
            {
                
            }

            void drawRect(const BlueMarble::Rectangle& bounds)
            {
                auto drawable = m_map->drawable();
                auto pen = BlueMarble::Pen();
                pen.setColor(BlueMarble::Color{0, 0, 0});
                auto brush = Brush();
                brush.setColor(BlueMarble::Color{255, 255, 255, 0.2});

                auto line = bounds.corners();
                auto linePtr = std::make_shared<PolygonGeometry>(line);
                drawable->drawPolygon(linePtr, pen, brush); 

                auto l2 = Rectangle(0,0,100,100).corners();
                drawable->drawPolygon(std::make_shared<PolygonGeometry>(l2), pen, brush);
            }

            void OnCustomDraw(BlueMarble::Map& /*map*/)
            {
                ScreenPos pos;
                m_mapControl->getMousePos(pos);
                int offset = 10;
                auto pixelColor = m_map->drawable()->readPixel(pos.x, pos.y-offset);
                m_map->drawable()->setPixel(pos.x, pos.y-offset, Color::red());
                std::string posString = std::to_string(pos.x) + ", " + std::to_string(pos.y);
                m_map->drawable()->drawText(0, m_map->drawable()->height()-45, posString, Color::red());
                m_map->drawable()->drawText(0, m_map->drawable()->height()-30, pixelColor.toString(), Color::red());

                if (!m_rectangle.isUndefined())
                {
                    auto screenRect = m_map->mapToScreen(m_rectangle);
                    screenRect.floor();
                    drawRect(screenRect);
                    BMM_DEBUG() << "Draw rect: " << screenRect.toString() << "\n";
                }

                if (m_drawDataSetInfo)
                {
                    std::cout << "Draw data set info\n";
                    auto dataSets = DataSet::getDataSets();
                    int offset = 0;
                    for (auto it : dataSets)
                    {
                        auto dataSet = it.second;
                        std::string str = !dataSet->name().empty() ? dataSet->name() : "DataSet";
                        str = dataSet->isInitialized() ? str : str + " (initializing)"; 
                        m_map->drawable()->drawText(0, offset, str, Color::red(), 20, Color::white());
                        offset += 15;
                    }
                }

                Pen pen;
                pen.setAntiAlias(true);
                pen.setThickness(3.0);
                pen.setColor(Color::red(0.9));

                Brush brush;
                brush.setAntiAlias(false);
                brush.setColor(Color::white(0.25));
                //brush.setColor(Color::blue());
                
                auto c = m_map->screenCenter();
                m_map->drawable()->drawCircle(c.x(), c.y()+40, 20, pen, brush);
                pen.setAntiAlias(false);
                m_map->drawable()->drawCircle(c.x()+40, c.y()+40, 20, pen, brush);
                //m_map->drawable()->drawArc(c.x(), c.y(), 10, 20, 0.5, pen, brush);
            }

            void OnUpdated(BlueMarble::Map& /*map*/)
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
                return false;
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
                    if (m_map->undoCommand())
                        return true;
                    break;
                case BlueMarble::KeyButton::Space:
                {                    
                    if (m_map->selected().size() > 0)
                    {
                        // Get a list of all boundary points for all features 
                        auto pointList = std::vector<Point>();
                        for (auto id : m_map->selected()) 
                        { 
                            auto dataSet = DataSet::getDataSetById(id.dataSetId());
                            auto f = dataSet->getFeature(id);
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
                            auto area = m_map->mapToLngLat(m_map->area());
                            area.reCenter(pointList[0]);
                            bounds = m_map->lngLatToMap(area);
                        }
                        else if (pointList.size() > 1)
                        {
                            bounds = m_map->lngLatToMap(Rectangle::fromPoints(pointList));
                            // Add some margin
                            bounds.extend(50.0/m_map->scale(), 50.0/m_map->scale());
                        }

                        
                        if (!bounds.isUndefined())
                        {
                            //m_map->zoomToArea(rect, true);
                            // TODO
                            m_map->doCommand([this, bounds]() 
                            {
                                this->m_map->zoomToArea(bounds, true);
                            }
                            );
                            
                        }
                    }
                    return true;
                }

                case KeyButton::Enter:
                {   
                    //m_map->update();
                    std::cout << "Enter, modkey: " << event.modificationKey << "\n";
                    if (event.modificationKey & ModificationKeyCtrl)
                    {
                        m_drawDataSetInfo = true;
                        m_map->update();
                    }
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
                    //if (event.key.isNumberKey())
                    {
                        int i = numberKeyToInt(event.keyButton);
                        std::cout << "Layers: " << m_map->layers().size() << "\n";
                        std::cout << "Number: " << i << "\n";
                        if (i <= (int)m_map->layers().size())
                        {
                            bool enabled = m_map->layers()[i-1]->enabled();
                            enabled = !enabled;
                            m_map->layers()[i-1]->enabled(enabled);
                            std::cout << "Enabled layer " << i << " (" << enabled << "\n";
                            m_map->update();
                            return true;
                        }
                    }
                    return false;
                }

                if (event.modificationKey & ModificationKeyCtrl)
                {
                    m_map->rotation(m_map->rotation() + direction.x()*10.0);
                    m_map->update();
                }
                else
                {
                    m_map->panBy(direction * 50.0, true);
                    m_map->update();
                }
                

                return true;
            }

            bool OnKeyUp(const BlueMarble::KeyUpEvent& event)
            {
                switch (event.keyButton)
                {
                    case KeyButton::Enter:
                    {
                        m_drawDataSetInfo = false;
                        m_map->update();
                    }
                    default:
                        break;
                }
                return true;
            }

            bool OnMouseDown(const BlueMarble::MouseDownEvent& event) override final
            {
                m_map->stopAnimation();
                m_map->update();
                // m_lastX = event.pos.x;
                // m_lastY = event.pos.y;
                return true;
            }

            bool OnMouseMove(const BlueMarble::MouseMoveEvent& event) override final
            {
                auto pObjs = m_map->hitTest(event.pos.x, event.pos.y, 10.0);
                BlueMarble::FeaturePtr hoverFeature(nullptr);
                if (pObjs.size() > 0)
                {
                    auto p = m_map->screenToMap(Point(event.pos.x, event.pos.y));
                    
                    m_hitTestLine->points().push_back(p);
                    hoverFeature = pObjs[0].sourceFeature();
                    
                    m_map->update();
                    m_hoverFeature = hoverFeature;
                }

                if (!m_map->isHovered(hoverFeature))
                {
                    // std::cout << "Not hovered\n";
                    m_map->hover(hoverFeature);
                    m_map->update();
                }

                // if (event.mouseButton == MouseButtonLeft)
                // {
                //     m_map->panBy({(double)(m_lastX - event.pos.x), 
                //               (double)(m_lastY - event.pos.y)});
                //     m_map->update();
                //     m_lastX = event.pos.x;
                //     m_lastY = event.pos.y;
                // }

                return true;
            }

            bool OnClick(const BlueMarble::ClickEvent& event) override final
            {
                auto pObjs = m_map->hitTest(event.pos.x, event.pos.y, 10.0);
                Id selId(0,0);
                FeaturePtr selFeat(nullptr);
                if (pObjs.size() > 0)
                {
                    selId = pObjs[0].sourceFeature()->id();
                    selFeat = pObjs[0].sourceFeature();
                }
                else
                {
                    m_map->deSelectAll();
                    m_map->update();

                    return true;
                }

                auto mode = (event.modificationKey == ModificationKeyCtrl) 
                        ? SelectMode::Add : SelectMode::Replace;
                if (!m_map->isSelected(selFeat))
                {   
                    m_map->select(selFeat, mode);
                }
                else if (mode == SelectMode::Replace)
                {
                    if (m_map->selected().size() == 1)
                    {
                        m_map->deSelect(selFeat);
                    }
                    else if (m_map->selected().size() > 1)
                    {
                        m_map->select(selFeat, SelectMode::Replace);
                    }
                }
                else // SelectMode::Add
                {
                    m_map->deSelect(selFeat);
                    std::cout << "Clicked " << m_map->center().toString() << "\n";
                }
                
                m_map->update();

                return true;
            }

            bool OnDoubleClick(const BlueMarble::DoubleClickEvent& event) override final
            {                    
                double zoomFactor = 2.5;
                auto mapPoint = m_map->screenToMap(event.pos.x, event.pos.y);

                auto hitFeatures = m_map->featuresAt(event.pos.x, event.pos.y, 10.0);
                
                if (hitFeatures.size() > 0)
                {
                    auto f = hitFeatures[0];
                    m_map->select(hitFeatures[0], SelectMode::Add);
                    if (!f->bounds().isUndefined())
                    {
                        auto rect = m_map->lngLatToMap(f->bounds());
                        m_map->zoomToArea(rect, true);
                        m_map->update();
                        return true;
                    }
                    mapPoint = m_map->lngLatToMap(hitFeatures[0]->center());
                    zoomFactor *= 2;
                }

                if (!m_rectangle.isUndefined() && m_rectangle.isInside(mapPoint))
                {
                    // animate to rectangle
                    // auto to = m_rectangle.center();
                    
                    // auto animation = BlueMarble::Animation::Create(m_map, m_map->center(), to, m_map->scale(), m_map->scale()*zoomFactor, 500, false);
                    // m_map->startAnimation(animation);
                    m_map->zoomToArea(m_rectangle, true);
                }
                else
                {
                    if (event.mouseButton == MouseButton::MouseButtonRight)
                    {
                        m_map->center({0,0}); // Recenter, there is a bug in this shiet
                    }
                    else
                    {
                        m_map->zoomOn(mapPoint, zoomFactor, true);
                    }
                }
                m_map->update();

                return true;
            }

            bool OnDragBegin(const BlueMarble::DragBeginEvent& dragBeginEvent) override final
            {
                std::cout << dragBeginEvent.toString()<< " (" << dragBeginEvent.pos.x << ", " << dragBeginEvent.pos.y << ")\n";
                addMousePos(dragBeginEvent.pos, dragBeginEvent.timeStampMs);
                m_debugTime = dragBeginEvent.timeStampMs;
                m_map->quickUpdateEnabled(true); // TODO: make an interaction handler that manages if this is enabled or not?
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
                            m_rectangle = m_map->screenToMap(rect);
                            m_map->update();
                        }
                        else
                        {
                            m_map->panBy({(double)(dragEvent.lastPos.x - dragEvent.pos.x), 
                                        (double)(dragEvent.lastPos.y - dragEvent.pos.y)});
                            m_map->update();
                        }
                        
                        break;
                    }
                case BlueMarble::MouseButtonRight:
                    {
                        if (dragEvent.modificationKey & ModificationKeyCtrl)
                        {
                            // Rotate
                            auto center = m_map->screenCenter();
                            auto prev = Point(dragEvent.lastPos.x, dragEvent.lastPos.y);
                            auto curr = Point(dragEvent.pos.x, dragEvent.pos.y);
                            
                            auto prevOffset = prev-center;
                            auto currOffset = curr-center;

                            double startAngle = std::atan2(prevOffset.y(), prevOffset.x());
                            double currAngle = std::atan2(currOffset.y(), currOffset.x());

                            double deltaAngle = currAngle-startAngle;

                            m_map->rotation(m_map->rotation() + deltaAngle*RAD_TO_DEG);
                            m_map->update();
                        }
                        else
                        {
                            // Zoom
                            const double ZOOM_SCALE = 0.01;
                            auto mapPoint = m_map->screenToMap(BlueMarble::Point(dragEvent.startPos.x, dragEvent.startPos.y));
                            double deltaY = dragEvent.pos.y - dragEvent.lastPos.y;
                            double scale = 1 + abs(deltaY)*ZOOM_SCALE;
                            double zoomFactor = deltaY > 0 ? scale : 1.0/scale;
                            m_map->zoomOn(mapPoint, zoomFactor);
                            m_map->update();
                        }
                        
                        break;
                    }
                case BlueMarble::MouseButtonMiddle:
                    {
                        // Pan navigation thing
                        m_map->panBy({-(double)(dragEvent.lastPos.x - dragEvent.pos.x), 
                                      -(double)(dragEvent.lastPos.y - dragEvent.pos.y)});
                        m_map->update();

                        // Draw a rectangle
                        // auto corner1 = m_map->screenToMap(dragEvent.pos.x, dragEvent.pos.y);
                        // auto corner2 = m_map->screenToMap(dragEvent.startPos.x, dragEvent.startPos.y);
                        // m_rectangle = BlueMarble::Rectangle(corner1.x(), corner1.y(), corner2.x(), corner2.y());
                        // std::cout << "Rectangle set: " << std::to_string(m_rectangle.xMin()) << ", " << std::to_string(m_rectangle.yMin()) << ", " << std::to_string(m_rectangle.xMax()) << ", " << std::to_string(m_rectangle.yMax()) << "\n";
                        // m_map->update();
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
                m_map->quickUpdateEnabled(false);
                if (m_zoomToRect)
                {
                    m_zoomToRect = false;
                    auto topLeft = m_map->screenToMap(BlueMarble::Point(dragEndEvent.startPos.x, dragEndEvent.startPos.y));
                    auto bottomRight = m_map->screenToMap(BlueMarble::Point(dragEndEvent.pos.x, dragEndEvent.pos.y));
                    auto rect = BlueMarble::Rectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
                    m_map->zoomToArea(rect, true);
                    m_map->update();
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
                        auto to = m_map->screenToMap(m_map->screenCenter() + offset);

                        auto animation = BlueMarble::Animation::Create(*m_map,
                                                                    m_map->center(), 
                                                                    to, 
                                                                    animationDuration, 
                                                                    true, 
                                                                    m_inertiaOption);

                        m_map->startAnimation(animation);
                    }
                    break;
                
                default:
                    break;
                }
                m_map->update();

                return true;
            }

            bool OnMouseWheel(const BlueMarble::MouseWheelEvent& wheelEvent) override final
            {
                const double wheelDelta = 5;

                double scale = 1.0 + abs(wheelEvent.delta)/wheelDelta;
                double zoomFactor = wheelEvent.delta > 0 ? scale : 1.0/scale;
                bool animate = true; //abs(wheelEvent.delta) > 1; // only animate if wheel delta is large enough
                if (!animate)
                {
                    m_map->stopAnimation(); // Need to stop animation for this to have an affect when not animating
                }
                m_map->zoomOn(m_map->screenToMap(BlueMarble::Point(wheelEvent.pos.x, wheelEvent.pos.y)), zoomFactor, animate);
                m_map->update();
                return true;
            }

            void addMousePos(const BlueMarble::ScreenPos& pos, int timeStamp)
            {
                m_positions.push_back(pos);
                m_timeStamps.push_back(timeStamp);
            }

            void prunePositions(int timeStamp)
            {
                while (m_timeStamps.size() > 0 && timeStamp - m_timeStamps[0] > m_cutoff)
                {
                    m_timeStamps.erase(m_timeStamps.begin());
                    m_positions.erase(m_positions.begin());
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
                if (deltaTime == 0)
                {
                    BMM_DEBUG() << "WARNING: delta time evaluated to 0!\n";
                    return Point{0.0};
                }
                BlueMarble::Point deltaPos = BlueMarble::Point(m_positions[diffIdx].x, m_positions[diffIdx].y) 
                                            - BlueMarble::Point(m_positions[0].x, m_positions[0].y);

                return deltaPos * (1.0/deltaTime);
            }

            bool onEvent(const BlueMarble::Event& event) override final
            {
                // Debug output event info
                // const BlueMarble::PointerEvent& pointerEvent = static_cast<const BlueMarble::PointerEvent&>(event);
                // std::string button = BlueMarble::mouseButtonToString(pointerEvent.mouseButton);
                // std::cout << button << event.toString()<< " (" << pointerEvent.pos.x << ", " << pointerEvent.pos.y << ")\n";

                if (event.getType() == EventType::Resize)
                {
                    std::cout << "Resize!\n";
                }

                return Tool::onEvent(event);
            }
        private:
            
            BlueMarble::MapPtr m_map;
            MapControlPtr m_mapControl;
            BlueMarble::Rectangle m_rectangle;
            bool m_zoomToRect;
            std::vector<int> m_timeStamps;
            BlueMarble::InertiaOptions m_inertiaOption;
            std::vector<BlueMarble::ScreenPos> m_positions;
            int m_cutoff;
            int m_debugTime;
            bool m_drawDataSetInfo;
            int m_lastX;
            int m_lastY;
            LineGeometryPtr m_hitTestLine;
            FeaturePtr m_hoverFeature;
    };

    class PointerTracerTool : public Tool
    {
        public:
            PointerTracerTool()
                : m_control(nullptr)
                , m_map(nullptr)
                , m_trace()
                , m_currPos{-1, -1}
                , m_traceSize(100)
                , m_cutoff(150)
            {
            }
            
            bool isActive() override final
            {
                return false;
            }

            void onConnected(const MapControlPtr& mapControl, const MapPtr& map)
            {
                m_control = mapControl;
                m_map = map;
                m_map->events.onCustomDraw.subscribe(this, &PointerTracerTool::OnCustomDraw);
                m_map->events.onUpdating.subscribe(this, &PointerTracerTool::OnUpdating);
            }

            void onDisconnected()
            {
                m_map->events.onCustomDraw.unsubscribe(this);
                m_map->events.onUpdating.unsubscribe(this);
                m_control = nullptr;
                m_map = nullptr;
            }

            void OnUpdating(Map& map)
            {
                auto timeStamp = map.updateAttributes().get<int>(UpdateAttributeKeys::UpdateTimeMs);
                prunePositions(timeStamp);
            }

            void OnCustomDraw(Map& map)
            {
                //if (m_trace.size() < 2)
                //    return;

                auto drawable = map.drawable();

                auto line = std::make_shared<LineGeometry>();
                
                

                auto c1 = drawable->readPixel(m_currPos.x, m_currPos.y);

                for (const auto& tp : m_trace)
                {
                    const auto& screen = tp.first;
                    line->points().push_back(Point((double)screen.x, (double)screen.y));
                }

                Pen p;
                p.setAntiAlias(true);
                p.setColors(Color::colorRamp(Color::black(0.0), Color::gray(0.5), line->points().size()));
                p.setThickness(3.0);

                // Draw the trace as a line
                drawable->drawLine(line, p);

                //if (m_currPos.x != -1)
                if (!m_trace.empty())
                {
                    Brush b;
                    b.setAntiAlias(true);
                    b.setColor(p.getColor());
                    
                    // double timeStamp = map.updateAttributes().get<int>(UpdateAttributeKeys::UpdateTimeMs);

                    const auto& lastPos = m_trace[m_trace.size()-1].first; // Last added pos
                    
                    //double radius = AnimationFunctions::easeOut((double)line->points().size() / (double)m_traceSize, 4.0);
                    double radius = AnimationFunctions::easeInCubic((double)line->points().size() / (double)m_traceSize);
                    radius = 10.0*std::min(1.0, radius);
                    //double radius = 10.0*std::min(1.0, line->length() / 200.0);

                    drawable->drawCircle(lastPos.x, lastPos.y, radius, Pen::transparent(), b);

                    // We still have stuff left
                    map.update(); // TODO: This may trigger more updates than needed. Use timer instead
                }
            }

            bool OnMouseMove(const MouseMoveEvent& event) override final
            {
                // while ((int)m_trace.size() >= m_traceSize)
                // {
                //     m_trace.erase(m_trace.begin());
                // }
                prunePositions(event.timeStampMs);

                m_trace.push_back({event.pos, event.timeStampMs});
                m_currPos = event.pos;

                m_map->update();

                return true;
            }

        private:
            void prunePositions(int64_t timeStamp)
            {
                if (m_trace.empty()) return;
                auto it = m_trace.begin();
                
                while ((it != m_trace.end() && timeStamp - it->second > m_cutoff) ||
                        m_trace.size() > m_traceSize)
                {
                    it = m_trace.erase(it);
                }
            }

            double calculateAverageSurvivalTime(int64_t timeStamp)
            {
                double tot = 0.0;
                for (const auto& tp : m_trace)
                {
                    tot += std::max(0.0, (double)m_cutoff-(double)(timeStamp-tp.second));
                }

                return tot / m_trace.size();
            }

            MapControlPtr           m_control;
            MapPtr                  m_map;
            std::vector<std::pair<ScreenPos, int64_t>>  m_trace;
            ScreenPos               m_currPos;
            int                     m_traceSize;
            int                     m_cutoff;
    };
    typedef std::shared_ptr<PointerTracerTool> PointerTracerToolPtr;

    class KeyActionTool : public Tool
    {
        public:
            KeyActionTool()
                : m_map(nullptr)
            {}

            bool isActive() { return false; }

            void onConnected(const MapControlPtr& control, const MapPtr& map) override final
            {
                m_map = map;
            }

            void onDisconnected() override final
            {
                m_map = nullptr;
            }

            bool OnKeyDown(const KeyDownEvent& event) override final
            {
                BMM_DEBUG() << "Key action controller: " << event.keyCode << "\n";
                if (event.keyCode == 39 && // s
                    event.modificationKey && ModificationKeyCtrl)
                {
                    BMM_DEBUG() << "Saving drawable buffer to file...\n";
                    auto raster = m_map->drawable()->getRaster();
                    raster.save("temporary.png");
                    return true;
                }

                // TODO rendering enabled
                if (event.keyCode == 27 && // r
                    event.modificationKey && ModificationKeyCtrl)
                {
                    bool enabled = !m_map->renderingEnabled();
                    m_map->renderingEnabled(enabled);
                    m_map->update();

                    if (enabled)
                        BMM_DEBUG() << "Enabled rendering!\n";
                    else
                        BMM_DEBUG() << "Disabled rendering!\n";
                }
                
                if (event.keyCode == 40 && // d
                    event.modificationKey && ModificationKeyCtrl)
                {
                    bool enabled = !m_map->showDebugInfo();
                    m_map->showDebugInfo() = enabled;
                    m_map->update();

                    if (enabled)
                        BMM_DEBUG() << "Debug info enabled!\n";
                    else
                        BMM_DEBUG() << "Debug info disabled!\n";
                }

                return false;
            }
        private:
            MapPtr m_map;
    };
    typedef std::shared_ptr<KeyActionTool> KeyActionToolPtr;

}
#endif /* DEFAULTEVENTHANDLERS */
