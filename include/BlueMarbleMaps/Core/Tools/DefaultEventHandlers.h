#ifndef DEFAULTEVENTHANDLERS
#define DEFAULTEVENTHANDLERS

#include "BlueMarbleMaps/Event/EventHandler.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Event/PointerEvent.h"
#include "BlueMarbleMaps/Event/KeyEvent.h"
#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/AnimationFunctions.h"
#include "BlueMarbleMaps/Core/DataSets/MemoryDataSet.h"
#include "Keys.h"
#include "BlueMarbleMaps/Core/Camera/PlaneCameraController.h"

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
                    fromPos = m_map->crs()->projectTo(m_editFeature->crs(), fromPos);
                    toPos = m_map->crs()->projectTo(m_editFeature->crs(), toPos);
                    m_editFeature->move(toPos-fromPos);
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
                , m_orbitPoint(Point::undefined())
                , m_zoomToRect(false)
                , m_hitTestLine(std::make_shared<LineGeometry>())
                , m_hoverFeature(nullptr)
            {
            }

            bool isActive() override final
            {
                return false; // Don't halt any other tools
            }

            void onConnected(const MapControlPtr& control, const MapPtr& map) override final 
            {
                m_mapControl = control;
                m_map = map;
                m_map->events.onUpdating.subscribe(this, &PanEventHandler::OnUpdating);
                m_map->events.onCustomDraw.subscribe(this, &PanEventHandler::OnCustomDraw);
                m_map->events.onUpdated.subscribe(this, &PanEventHandler::OnUpdated);
                m_map->events.onIdle.subscribe(this, &PanEventHandler::OnIdle);
                m_map->setCameraController(&m_cameraController);
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

            void OnUpdating(BlueMarble::Map& map)
            {
            }

            void drawRect(const BlueMarble::Rectangle& bounds)
            {
                auto drawable = m_map->drawable();
                m_map->setDrawableFromCamera(m_map->camera());
                auto pen = BlueMarble::Pen();
                pen.setColor(BlueMarble::Color{255, 255, 255, 0.5});
                auto brush = Brush();

                auto line = bounds.corners(true);
                auto poly = std::make_shared<PolygonGeometry>(line);
                auto linePtr = std::make_shared<LineGeometry>(line);
                linePtr->isClosed(true);
                brush.setColors(Color::colorRamp(Color::red(0.1), Color::green(0.5), line.size()));
                drawable->drawPolygon(poly, pen, brush);
                drawable->drawLine(linePtr, pen);
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
                    drawRect(m_rectangle);
                    // BMM_DEBUG() << "Draw rect: " << m_rectangle.toString() << "\n";
                }

                auto generateArcLine = [](double r, double startAngle, double endAngle)
                {
                    auto line = std::make_shared<LineGeometry>();

                    int pointsPerRev = 30;
                    double diff = Utils::normalizeValue(endAngle-startAngle, 0.0, BMM_PI*2.0);
                    int nPoints = diff / (BMM_PI*2.0) * pointsPerRev;

                    for (int i(0); i<nPoints; ++i)
                    {
                        double a = startAngle + i*diff/(double)nPoints;
                        double x = r*std::cos(a);
                        double y = r*std::sin(a);
                        auto p = Point(x,y);
                        line->points().push_back(p);
                    }

                    return line;
                };

                if (!m_orbitPoint.isUndefined())
                {
                    // static auto radiusProgressEval = AnimationFunctions::AnimationBuilder().easeOut(5.5).build();
                    static auto radiusProgressEval = AnimationFunctions::AnimationBuilder().bounce().build();
                    static auto rotationProgressEval = AnimationFunctions::AnimationBuilder().sigmoid(15.0).build();
                    constexpr int animationTime = 1000;
                    constexpr double symbolScale = 1.3;
                    
                    const auto& orbitPoint = m_orbitPoint;

                    int64_t elapsed = getTimeStampMs()-m_startTsOrbit;
                    double progress = elapsed/double(animationTime);
                    progress = progress < 1.0 ? progress : 1.0;

                    double radiusProgress = radiusProgressEval(progress);
                    double radius = 20.0*radiusProgress*symbolScale;
                    auto orbitView = m_map->camera()->worldToView(orbitPoint);
                    double unitsPerPixel = m_map->camera()->unitsPerPixelAtDistance(std::abs(orbitView.z()));
                    radius = radius * unitsPerPixel;
                    double zDisplacement = 5*unitsPerPixel*radiusProgress*symbolScale;

                    double rotationProgress = rotationProgressEval(progress);
                    double rotation = BMM_PI * (1.0-rotationProgress);

                    auto d = m_map->drawable();
                    d->endBatches();
                    d->beginBatches();
                    m_map->setDrawableFromCamera(m_map->camera());
                    Pen ppp;
                    ppp.setColor(Color::white(0.5));
                    Brush bbb = Brush::transparent();
                    bbb.setColor(Color::blue(0.5));

                    double x = orbitPoint.x();
                    double y = orbitPoint.y();
                    
                    double off = BMM_PI * 0.5 + rotation;
                    double gap = 0.1;     // 0.1
                    double stretch = 0.12; // 0.2
                    auto louter1 = generateArcLine(radius, gap+off, BMM_PI+off-gap);                    
                    auto louter2 = generateArcLine(radius*0.95, stretch+off+gap, BMM_PI+stretch-gap+off);  
                    auto louter3 = generateArcLine(radius*0.9, stretch*2+off+gap, BMM_PI+stretch*2-gap+off);
                    auto outer11 = std::make_shared<LineGeometry>();
                    auto outer22 = std::make_shared<LineGeometry>();
                    auto outer33 = std::make_shared<LineGeometry>();
                    outer11->points() = Utils::rotatePoints(louter1->points(), BMM_PI, {0,0}); 
                    outer22->points() = Utils::rotatePoints(louter2->points(), BMM_PI, {0,0});
                    outer33->points() = Utils::rotatePoints(louter3->points(), BMM_PI, {0,0});

                    auto inner1 = generateArcLine(radius*0.3, 0.01, 2.0*BMM_PI);
                    auto inner2 = generateArcLine(radius*0.3, 0.01, 2.0*BMM_PI);
                    
                    louter1->move({x,y,0.0});
                    louter2->move({x,y,0.0});
                    louter3->move({x,y,0.0});
                    outer11->move({x,y,0.0});
                    outer22->move({x,y,0.0});
                    outer33->move({x,y,0.0});
                    inner1->move({x,y,0.0});
                    inner2->move({x,y,zDisplacement});

                    d->drawLine(louter1, ppp);
                    d->drawLine(louter2, ppp);
                    d->drawLine(louter3, ppp);
                    d->drawLine(outer11, ppp);
                    d->drawLine(outer22, ppp);
                    d->drawLine(outer33, ppp);


                    //ppp.setColor(Color::blue(0.5));
                    d->drawLine(inner1, ppp);
                    d->drawLine(inner2, ppp);
                    // inner2->move({0,0,zDisplacement});
                    auto pol = std::make_shared<PolygonGeometry>(inner2->points());
                    d->endBatches();
                    d->beginBatches();
                    d->drawPolygon(pol, ppp, bbb);
                    
                    d->endBatches();
                    d->beginBatches();

                    m_map->update();
                }
            }

            void OnUpdated(BlueMarble::Map& /*map*/)
            {
                
            }

            void onFeatureCreated(const FeaturePtr& /*feature*/) override final
            {}
            void onFeatureUpdated(const FeaturePtr& /*feature*/) override final
            {}
            void onFeatureDeleted(const Id& /*id*/) override final
            {}

            bool OnKeyDown(const KeyDownEvent& event) override final
            {
                if (event.keyCode == 86) // +
                {
                    m_cameraController.changeFovBy(-5.0);
                    m_map->update();
                }
                else if (event.keyCode == 82)  // -
                {
                    m_cameraController.changeFovBy(5.0);
                    m_map->update();
                }
                return true;
            }

            bool OnMouseDown(const BlueMarble::MouseDownEvent& event) override final
            {
                return false;
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
                }
                
                m_map->update();

                return true;
            }

            bool OnDoubleClick(const BlueMarble::DoubleClickEvent& event) override final
            {                    
                auto mapPoint = m_map->screenToMap(event.pos.x, event.pos.y);

                if (event.mouseButton == MouseButton::MouseButtonRight)
                {
                    // m_map->center({0,0}); // Recenter, there is a bug in this shiet
                    m_cameraController.center({0,0});
                }
                else
                {
                    //m_map->zoomOn(mapPoint, zoomFactor, true);
                    m_cameraController.zoomOn(mapPoint, 2.0);
                }

                m_map->update();

                return true;
            }

            bool OnDrag(const DragEvent& dragEvent) override final
            {
                if (dragEvent.phase == InteractionEvent::Phase::Started)
                {
                    std::cout << dragEvent.toString()<< " (" << dragEvent.pos.x << ", " << dragEvent.pos.y << ")\n";
                    m_map->quickUpdateEnabled(true); // TODO: make an interaction handler that manages if this is enabled or not?
                    m_startTsOrbit = m_mapControl->getGinotonicTimeStampMs();
                    
                    return true;
                }
                if (dragEvent.phase == InteractionEvent::Phase::Completed)
                {
                    std::cout << dragEvent.toString()<< " (" << dragEvent.pos.x << ", " << dragEvent.pos.y << ")\n";
                    m_map->quickUpdateEnabled(false);
                    m_orbitPoint = Point::undefined();
                    if (m_zoomToRect)
                    {
                        m_zoomToRect = false;
                        auto rect = Rectangle(dragEvent.startPos.x, dragEvent.startPos.y, 
                                              dragEvent.pos.x, dragEvent.pos.y);
                        
                        m_cameraController.zoomTo(m_map->screenToMap(rect));
                        // m_map->zoomToArea(m_map->screenToMap(rect), false);
                        
                        m_map->update();
                        m_rectangle = BlueMarble::Rectangle::undefined();
                        
                        return true;
                    }
                    if (m_selectArea)
                    {
                        m_selectArea = false;
                        m_map->update();
                        m_rectangle = BlueMarble::Rectangle::undefined();
                        return true;
                    }
                    m_map->update();

                    return true;
                }

                switch (dragEvent.mouseButton)
                {
                case BlueMarble::MouseButtonLeft:
                    {
                        if (dragEvent.modificationKey & BlueMarble::ModificationKeyCtrl)
                        {
                            // zoom to rect
                            std::cout << "Mod key: " << dragEvent.modificationKey << "\n";
                            m_zoomToRect = true;
                            auto points = std::vector<Point>(); 
                            points.push_back(m_map->screenToMap(Point{dragEvent.pos.x, dragEvent.pos.y}));
                            points.push_back(m_map->screenToMap(Point{dragEvent.startPos.x, dragEvent.startPos.y}));
                            m_rectangle = Rectangle::fromPoints(points);
                            m_map->update();
                        }
                        else if (dragEvent.modificationKey & BlueMarble::ModificationKeyShift)
                        {
                            // zoom to rect
                            std::cout << "Mod key: " << dragEvent.modificationKey << "\n";
                            m_selectArea = true;
                            auto points = std::vector<Point>(); 
                            points.push_back(m_map->screenToMap(Point{dragEvent.pos.x, dragEvent.pos.y}));
                            points.push_back(m_map->screenToMap(Point{dragEvent.startPos.x, dragEvent.startPos.y}));
                            m_rectangle = Rectangle::fromPoints(points);
                            
                            FeatureCollection featuresIn;
                            m_map->featuresInside(m_rectangle, featuresIn);
                            for (auto id : m_map->selected())
                            {
                                if (!featuresIn.contains(id))
                                {
                                    m_map->deSelect(id);
                                }
                            }
                            for (auto f : featuresIn)
                            {
                                m_map->select(f, SelectMode::Add);
                                
                            }
                            

                            m_map->update();
                        }
                        else
                        {
                            // New
                            auto screen1 = Point(dragEvent.pos.x, dragEvent.pos.y);
                            auto screen2 = Point(dragEvent.lastPos.x, dragEvent.lastPos.y);
                            auto offsetWorld = m_map->screenToMap(screen2) - m_map->screenToMap(screen1);
                            auto to = m_cameraController.center() + offsetWorld;
                            //m_cameraController.center(to);
                            m_cameraController.panBy(offsetWorld);
                            //m_map->center(to);
                            // Old
                            // m_map->panBy({(double)(dragEvent.lastPos.x - dragEvent.pos.x), 
                            //             (double)(dragEvent.lastPos.y - dragEvent.pos.y)});
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

                            // m_map->rotation(m_map->rotation() + deltaAngle*RAD_TO_DEG);
                            m_cameraController.rotateBy(deltaAngle*RAD_TO_DEG);
                            m_map->update();
                        }
                        else
                        {
                            // Zoom
                            const double ZOOM_SCALE = 0.01;
                            auto mapPoint = m_map->screenToMap(m_map->pixelToScreen(Point{dragEvent.startPos.x, dragEvent.startPos.y}));
                            double deltaY = dragEvent.pos.y - dragEvent.lastPos.y;
                            double scale = 1 + abs(deltaY)*ZOOM_SCALE;
                            double zoomFactor = deltaY > 0 ? scale : 1.0/scale;
                            // m_map->zoomOn(mapPoint, zoomFactor);
                            m_cameraController.zoomOn(mapPoint, zoomFactor);
                            m_map->update();
                        }
                        
                        break;
                    }
                case BlueMarble::MouseButtonMiddle:
                    {
                        // auto rayCurr = m_map->screenToViewRay(dragEvent.pos.x, dragEvent.pos.y);
                        // auto rayLast = m_map->screenToViewRay(dragEvent.lastPos.x, dragEvent.lastPos.y);
                        
                        // // TODO: For orthographic wee need to offset instead
                        // // Point delta = rayCurr.origin - rayLast.origin;

                        // auto xzCurr = Point(rayCurr.direction.x(), 0.0, rayCurr.direction.z()).norm3D();
                        // auto xzLast = Point(rayLast.direction.x(), 0.0, rayLast.direction.z()).norm3D();
                        // auto yzCurr = Point(0.0, rayCurr.direction.y(), rayCurr.direction.z()).norm3D();
                        // auto yzLast = Point(0.0, rayLast.direction.y(), rayLast.direction.z()).norm3D();
                        
                        // double yaw = std::atan2(xzCurr.crossProduct(xzLast).y(), xzCurr.dotProduct(xzLast));
                        // double pitch = std::atan2(yzCurr.crossProduct(yzLast).x(), yzCurr.dotProduct(yzLast));
                        
                        // m_cameraController.rotateBy(-RAD_TO_DEG*yaw*2);
                        // m_cameraController.tiltBy(-RAD_TO_DEG*pitch*2);
                        
                        m_orbitPoint = m_map->screenToMap(m_map->screenCenter());
                        constexpr double rotateFactor = 0.3;
                        constexpr double tiltFactor = 0.3;

                        double deltaRot = (dragEvent.lastPos.x - dragEvent.pos.x) * rotateFactor;
                        double deltaTilt = (dragEvent.lastPos.y - dragEvent.pos.y) * tiltFactor;
                        
                        // m_cameraController.stop();
                        m_cameraController.rotateBy(deltaRot);
                        m_cameraController.tiltBy(deltaTilt);

                        m_map->update();

                        break;
                    }
                default:
                    break;
                }

                return true;
            }

            bool OnMouseWheel(const BlueMarble::MouseWheelEvent& wheelEvent) override final
            {
                const double wheelDelta = 5;
                double scale = 1.0 + abs(wheelEvent.delta)/wheelDelta;
                double zoomFactor = wheelEvent.delta > 0 ? scale : 1.0/scale;
                m_cameraController.zoomOn(m_map->screenToMap(m_map->pixelToScreen(Point(wheelEvent.pos.x, wheelEvent.pos.y))), zoomFactor);
                m_map->update();
                return true;
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
            PlaneCameraController m_cameraController;
            BlueMarble::Rectangle m_rectangle;
            Point m_orbitPoint;
            int64_t m_startTsOrbit;
            bool m_zoomToRect;
            bool m_selectArea;
            std::vector<int> m_timeStamps;
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
                    b.setColor(Color::gray(0.5));
                    
                    // double timeStamp = map.updateAttributes().get<int>(UpdateAttributeKeys::UpdateTimeMs);

                    const auto& lastPos = m_trace[m_trace.size()-1].first; // Last added pos
                    
                    //double radius = AnimationFunctions::easeOut((double)line->points().size() / (double)m_traceSize, 4.0);
                    double radius = AnimationFunctions::easeInCubic((double)line->points().size() / (double)m_traceSize);
                    radius = 10.0*std::min(1.0, radius);
                    //double radius = 10.0*std::min(1.0, line->length() / 200.0);

                    drawable->drawCircle(lastPos.x, lastPos.y, radius, Pen::transparent(), b);

                    // Testing map at height
                    // auto mapPointAtHeight = map.screenToMapAtHeight(Point(lastPos.x, lastPos.y), 10000.0);
                    
                    // auto screenP = map.mapToScreen(Point(mapPointAtHeight.x(), mapPointAtHeight.y()));
                    // Pen pennis;
                    // pennis.setColor(Color::green());
                    // drawable->drawCircle(lastPos.x, lastPos.y, 5, pennis, Brush::transparent());
                    // pennis.setColor(Color::red());
                    // drawable->drawCircle(screenP.x(), screenP.y(), 5, pennis, Brush::transparent());
                    // auto lll = std::make_shared<LineGeometry>();
                    // lll->points().push_back(Point(lastPos.x, lastPos.y));
                    // lll->points().push_back(screenP);
                    // drawable->drawLine(lll, pennis);

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
                Key keyStroke(event.keyCode);
            
                if (keyStroke == Key::S)
                    BMM_DEBUG() << "OHHH YAAS\n";

                BMM_DEBUG() << "Key action controller: " << event.keyCode << "\n";
                if (keyStroke == Key::S &&
                    event.modificationKey && ModificationKeyCtrl)
                {
                    BMM_DEBUG() << "Saving drawable buffer to file...\n";
                    auto raster = m_map->drawable()->getRaster();
                    raster.save("temporary.png");
                    return true;
                }

                if (keyStroke == Key::P &&
                    event.modificationKey && ModificationKeyCtrl)
                {
                    BMM_DEBUG() << "Changing crs...";
                    auto crs = m_map->crs();
                    // auto centerLngLat = m_map->crs()->projectTo(Crs::wgs84LngLat(), m_map->center());
                    // double scale = m_map->scale();
                    if (auto temp = std::dynamic_pointer_cast<MercatorWebProjection>(crs->projection()))
                    {
                        m_map->crs(Crs::wgs84LngLat());
                        BMM_DEBUG() << "... to long lat!\n";
                    }
                    else
                    {
                        m_map->crs(Crs::wgs84MercatorWeb());
                        BMM_DEBUG() << "... to Web  Mercator!\n";
                    }
                    // m_map->flushCache();
                    // m_map->center(Crs::wgs84LngLat()->projectTo(m_map->crs(), centerLngLat));
                    // m_map->scale(scale);
                    m_map->update();
                    return true;
                }

                // TODO rendering enabled
                if (keyStroke == 27 && // r
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
                
                if (keyStroke == 40 && // d
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

    class DebugEventHandler : public Tool
    {
    public:
        DebugEventHandler()
            : Tool()
            , m_map(nullptr)
            , m_mapControl(nullptr)
            , m_isActive(false)
        {
            m_hitTestLine = std::make_shared<LineGeometry>();
            m_hitTestLinePixel = std::make_shared<LineGeometry>();
        }
        void onConnected(const MapControlPtr& control, const MapPtr& map) override
        {
            m_map = map;
            m_mapControl = control;
            m_map->events.onCustomDraw.subscribe(this,&DebugEventHandler::drawCallback);
        }
        void onDisconnected() override
        {
            m_map->events.onCustomDraw.unsubscribe(this);
            m_map = nullptr;
            m_mapControl = nullptr;
        }
        bool isActive() override
        {
            return m_isActive;
        }

        bool OnClick(const ClickEvent& event) override final
        {
            auto ray = m_map->screenToMapRay(event.pos.x, event.pos.y);
            auto rayOrig = ray.origin;
            auto mapPos = m_map->screenToMap(event.pos.x, event.pos.y);

            m_hitTestLine->points().clear();
            m_hitTestLine->points().push_back(rayOrig);
            m_hitTestLine->points().push_back(mapPos);

            auto mapPosPixel = m_map->screenToMap(event.pos.x, event.pos.y);
            m_hitTestLinePixel->points().clear();
            m_hitTestLinePixel->points().push_back(rayOrig);
            m_hitTestLinePixel->points().push_back(mapPosPixel);

            m_map->update();

            return false;
        }

        void drawCallback(Map& map)
        {
            if (m_hitTestLine)
            {
                // auto screenLine = std::make_shared<LineGeometry>();
                // screenLine->points().assign({{ 1, 0 }, {20, 0.0}});
                // Pen pen;
                // pen.setColor(Color::red());
                // map.drawable()->drawLine(screenLine, pen);

                map.setDrawableFromCamera(map.camera());
                Pen p;
                p.setColor(Color::red());
                map.drawable()->drawLine(m_hitTestLine, p);

                Pen p2;
                p2.setColor(Color::green());
                map.drawable()->drawLine(m_hitTestLinePixel, p2);
            }
        }

    private: 
        MapControlPtr m_mapControl;
        MapPtr m_map;
        bool m_isActive;

        LineGeometryPtr m_hitTestLine;
        LineGeometryPtr m_hitTestLinePixel;
    };
}

#endif /* DEFAULTEVENTHANDLERS */
