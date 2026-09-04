#ifndef DEFAULTEVENTHANDLERS
#define DEFAULTEVENTHANDLERS

#include "BlueMarbleMaps/Event/EventHandler.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/Layer/StandardLayer.h"
#include "BlueMarbleMaps/Core/Layer/TileLayer.h"
#include "BlueMarbleMaps/Core/DataSets/ImageDataSet.h"
#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Event/PointerEvent.h"
#include "BlueMarbleMaps/Event/KeyEvent.h"
#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/AnimationFunctions.h"
#include "BlueMarbleMaps/Core/DataSets/MemoryDataSet.h"
#include "Keys.h"
#include "BlueMarbleMaps/Core/Camera/PlaneCameraController.h"
#include "gif-h/include/gif.h"

#include <fstream>
#include <regex>
#include <sstream>

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
                return m_editFeature != nullptr || !m_selectionRectangle.isUndefined();
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

            bool onMouseDown(const MouseDownEvent& event) override final
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

            bool onClick(const ClickEvent& event) override final
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

                    return false;
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

            bool onDrag(const DragEvent& event) override final
            {
                if (!m_selectionRectangle.isUndefined() || event.modificationKey & BlueMarble::ModificationKeyShift)
                {
                    selectArea(event);
                    return true;
                }

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

            bool onMouseUp(const MouseUpEvent& event) override final
            {
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
            void selectArea(const DragEvent& event)
            {
                if (event.phase == InteractionEvent::Phase::Started)
                {
                    m_map->events.onCustomDraw.subscribe(this, &EditFeatureTool::onCustomDraw);
                }
                else if (event.phase == InteractionEvent::Phase::Completed)
                {
                    m_map->events.onCustomDraw.unsubscribe(this);
                    m_selectionRectangle = Rectangle::undefined();
                    m_map->update();
                    return;
                }
                auto points = std::vector<Point>();
                points.push_back(Point{(double)event.pos.x, (double)event.pos.y});
                points.push_back(Point{(double)event.startPos.x, (double)event.startPos.y});
                m_selectionRectangle = Rectangle::fromPoints(points);

                auto selectionBounds = m_map->screenToMap(m_selectionRectangle);
                FeatureCollection featuresIn;
                m_map->featuresInside(selectionBounds, featuresIn);
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

            void onCustomDraw(Map& map)
            {
                if (!m_selectionRectangle.isUndefined())
                {
                    auto line = m_selectionRectangle.corners(true);
                    auto poly = std::make_shared<PolygonGeometry>(line);
                    auto linePtr = std::make_shared<LineGeometry>(line);
                    linePtr->isClosed(true);

                    auto pen = Pen();
                    pen.setColor(Color{255, 255, 255, 0.5});
                    auto brush = Brush();
                    brush.setColors(Color::colorRamp(Color::red(0.1), Color::green(0.5), line.size()));

                    auto drawable = map.drawable();
                    drawable->drawPolygon(poly, pen, brush);
                    drawable->drawLine(linePtr, pen);
                }
            }

            MapControlPtr m_control;
            MapPtr m_map;
            Rectangle m_selectionRectangle;
            bool m_active;
            FeaturePtr m_editFeature;
            int m_nodeIndex;

            bool m_autoSelect;
    };

    class CameraControllerTwoHalfD
        : public Tool
        , public IFeatureEventListener
    {
        public:
            CameraControllerTwoHalfD()
                : Tool()
                , m_map(nullptr)
                , m_mapControl(nullptr)
                , m_rectangle(BlueMarble::Rectangle::undefined())
                , m_orbitPoint(Point::undefined())
                , m_zoomPoint(Point::undefined())
                , m_zoomToRect(false)
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
                m_map->events.onCustomDraw.subscribe(this, &CameraControllerTwoHalfD::OnCustomDraw);
                m_map->setCameraController(&m_cameraController);
            }

            void onDisconnected() override final 
            {
                m_map->events.onCustomDraw.unsubscribe(this);
                m_map = nullptr;
                m_mapControl = nullptr;
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
                brush.setColors(Color::colorRamp(Color::blue(0.1), Color::red(0.5), line.size()));
                drawable->drawPolygon(poly, pen, brush);
                drawable->drawLine(linePtr, pen);
            }

            void OnCustomDraw(BlueMarble::Map& /*map*/)
            {
                // Pen pen;
                // pen.setColor(Color::red());
                // pen.setThickness(5.0);
                // pen.setAntiAlias(true);
                // auto lineGeom = std::make_shared<LineGeometry>(std::vector<Point>{Point(0, 0), Point(512, 512)});
                // auto polygonGeom = std::make_shared<PolygonGeometry>(std::vector<Point>{Point(0, 0), Point(512, 0), Point(512, 512), Point(0, 512)});
                // Brush brush;
                // brush.setColor(Color::blue());
                // m_map->drawable()->drawLine(lineGeom, pen);
                // m_map->drawable()->drawPolygon(polygonGeom, pen, brush);

                if (!m_rectangle.isUndefined())
                {
                    drawRect(m_rectangle);
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

                bool isOrbiting = !m_orbitPoint.isUndefined();
                bool isZooming  = !m_zoomPoint.isUndefined();

                if (isOrbiting || isZooming)
                {
                    static auto radiusProgressEval = AnimationFunctions::AnimationBuilder().bounce().build();
                    static auto rotationProgressEval = AnimationFunctions::AnimationBuilder().sigmoid(12.0).build();
                    
                    constexpr int animationTime = 1000; // Adjust this for animation duration (ms)
                    constexpr double symbolScale = 1.5; //1.3; // Adjust this for scaling the whole symbol
                    
                    const auto& orbitPoint = isOrbiting ? m_orbitPoint : m_zoomPoint;
                    double ratio = m_map->invertedScale() / m_interactionStartScale;
                    double orbitRotation = BMM_PI * std::log2(ratio);
                    auto screen = m_map->mapToScreen(orbitPoint);
                    Color colorAtPos = m_map->drawable()->readPixel((int)screen.x(), (int)screen.y());
                    double luminance = colorAtPos.luminance();

                    int64_t elapsed = getTimeStampMs()-m_startTsOrbit;
                    double progress = elapsed/double(animationTime);
                    progress = progress < 1.0 ? progress : 1.0;

                    // Radius
                    double radiusProgress = radiusProgressEval(progress);
                    double radius = 20.0*radiusProgress*symbolScale;
                    auto orbitView = m_map->camera()->worldToView(orbitPoint);
                    double unitsPerPixel = m_map->camera()->unitsPerPixelAtDistance(std::abs(orbitView.z()));
                    radius = radius * unitsPerPixel;

                    // Rotation
                    double rotationProgress = rotationProgressEval(progress);
                    double rotation = orbitRotation + BMM_PI * (1.0-rotationProgress);

                    // Z-displacement
                    double zProgress = radiusProgressEval(progress);
                    double zDisplacement = 3.0*unitsPerPixel*symbolScale*(zProgress + 5.0*(1-zProgress));

                    auto d = m_map->drawable();
                    d->endBatches();
                    d->beginBatches();
                    if (isOrbiting)
                    {
                        m_map->setDrawableFromCamera(m_map->camera());
                    }
                    else // zooming
                    {
                        // TODO: make the circle parallell to the image plane, isch
                        m_map->setDrawableFromCamera(m_map->camera());
                    }
                    Pen ppp;
                    ppp.setColor(luminance < 0.5 ? Color::white(0.5) : Color::black(0.5));
                    Brush bbb;;
                    bbb.setColor(Color(70, 50, 255, 0.5));
                    Brush bbb2;
                    bbb2.setColor(Color(70, 50, 70, 0.25));

                    double x = orbitPoint.x();
                    double y = orbitPoint.y();
                    
                    double off = rotation; // + BMM_PI * 0.5;
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
                    d->drawLine(inner1, ppp);
                    d->drawCircle(x,y,radius*0.3 - 1.0*unitsPerPixel,ppp, bbb2);

                    d->endBatches();
                    d->beginBatches();
                    // auto pol = std::make_shared<PolygonGeometry>(inner2->points());
                    // d->drawPolygon(pol, ppp, bbb);
                    
                    auto translation = m_map->camera()->translation();
                    
                    m_map->camera()->setTranslation(translation + Point(0,0,-zDisplacement));
                    m_map->setDrawableFromCamera(m_map->camera());
                    
                    d->drawCircle(x,y,radius*0.3 - 1.0*unitsPerPixel,ppp, bbb);
                    m_map->camera()->setTranslation(translation);
                    
                    d->endBatches();
                    d->beginBatches();
                    
                    m_map->setDrawableFromCamera(m_map->camera());
                    d->drawLine(inner2, ppp);
                    
                    d->endBatches();
                    d->beginBatches();

                    // Keep updating when drawing this
                    m_map->update();
                }
            }

            void onFeatureCreated(const FeaturePtr& /*feature*/) override final
            {}
            void onFeatureUpdated(const FeaturePtr& /*feature*/) override final
            {}
            void onFeatureDeleted(const Id& /*id*/) override final
            {}

            bool onKeyDown(const KeyDownEvent& event) override final
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

            bool onMouseDown(const BlueMarble::MouseDownEvent& event) override final
            {
                return false;
            }

            bool onMouseMove(const BlueMarble::MouseMoveEvent& event) override final
            {
                const bool hotTrackingEnabled = false;
                if (!hotTrackingEnabled) return false;

                auto pObjs = m_map->hitTest(event.pos.x, event.pos.y, 10.0);
                BlueMarble::FeaturePtr hoverFeature(nullptr);
                if (pObjs.size() > 0)
                {
                    auto p = m_map->screenToMap(Point(event.pos.x, event.pos.y));
                    
                    hoverFeature = pObjs[0].sourceFeature();
                    
                    m_map->update();
                    m_hoverFeature = hoverFeature;
                }

                if (!m_map->isHovered(hoverFeature))
                {
                    m_map->hover(hoverFeature);
                    m_map->update();
                }

                return true;
            }

            bool onDoubleClick(const BlueMarble::DoubleClickEvent& event) override final
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

            bool onDrag(const DragEvent& dragEvent) override final
            {
                if (dragEvent.phase == InteractionEvent::Phase::Started)
                {
                    m_map->quickUpdateEnabled(true); // TODO: make an interaction handler that manages if this is enabled or not?
                    m_startTsOrbit = m_mapControl->getGinotonicTimeStampMs();
                    
                    return true;
                }
                if (dragEvent.phase == InteractionEvent::Phase::Completed)
                {
                    m_map->quickUpdateEnabled(false);
                    m_orbitPoint = Point::undefined();
                    m_zoomPoint = Point::undefined();
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
                        m_zoomToRect = true;
                        auto points = std::vector<Point>(); 
                        points.push_back(m_map->screenToMap(Point{(double)dragEvent.pos.x, (double)dragEvent.pos.y}));
                        points.push_back(m_map->screenToMap(Point{(double)dragEvent.startPos.x, (double)dragEvent.startPos.y}));
                        m_rectangle = Rectangle::fromPoints(points);
                        m_map->update();
                    }
                    else
                    {
                        // New
                        auto screen1 = Point((double)dragEvent.pos.x, (double)dragEvent.pos.y);
                        auto screen2 = Point((double)dragEvent.lastPos.x, (double)dragEvent.lastPos.y);
                        auto offsetWorld = m_map->screenToMap(screen2) - m_map->screenToMap(screen1);
                        auto to = m_cameraController.center() + offsetWorld;
                        //m_cameraController.center(to);
                        m_cameraController.panBy(offsetWorld);
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
                        
                        auto mapPoint = m_map->screenToMap(m_map->pixelToScreen(Point{(double)dragEvent.startPos.x, 
                                                                                      (double)dragEvent.startPos.y}));
                        if (m_zoomPoint.isUndefined())
                        {
                            m_zoomPoint = mapPoint;
                            m_interactionStartScale = m_map->invertedScale();
                        }
                        
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
                    m_orbitPoint = m_map->screenToMap(m_map->screenCenter());
                    m_interactionStartScale = m_map->invertedScale();
                    constexpr double rotateFactor = 0.3;
                    constexpr double tiltFactor = 0.3; // TODO: should factor with fov

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

            bool onMouseWheel(const BlueMarble::MouseWheelEvent& wheelEvent) override final
            {
                const double wheelDelta = 5;
                double scale = 1.0 + abs(wheelEvent.delta)/wheelDelta;
                double zoomFactor = wheelEvent.delta > 0 ? scale : 1.0/scale;
                m_cameraController.zoomOn(m_map->screenToMap(m_map->pixelToScreen(Point(wheelEvent.pos.x, wheelEvent.pos.y))), zoomFactor);
                m_map->update();
                return true;
            }

        private:
            
            BlueMarble::MapPtr m_map;
            MapControlPtr m_mapControl;
            PlaneCameraController m_cameraController;
            BlueMarble::Rectangle m_rectangle;
            Point m_orbitPoint;
            Point m_zoomPoint;
            double m_interactionStartScale;
            int64_t m_startTsOrbit;
            bool m_zoomToRect;
            bool m_selectArea;
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
                , m_cutoff(170)
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
                auto drawable = map.drawable();
                auto c1 = drawable->readPixel(m_currPos.x-15.0, m_currPos.y-15.0);
                auto line = std::make_shared<LineGeometry>();
                for (const auto& tp : m_trace)
                {
                    const auto& screen = tp.first;
                    line->points().push_back(Point((double)screen.x, (double)screen.y));
                }

                // Draw hittest rectangle for debugging
                const double pointerRadius = 10.0;

                double scale = map.invertedScale() * drawable->pixelSize() / map.crs()->globalMetersPerUnit();
                double size = pointerRadius * 2.0 * scale;
                Point center = map.screenToMap(Point((double)m_currPos.x, (double)m_currPos.y));
                auto area = Rectangle(center, size, size);

                auto points = map.mapToScreen(area.corners(true));
                auto rectLine = std::make_shared<LineGeometry>(points);
                rectLine->isClosed(true);
                auto pen = Pen();
                pen.setColor(Color::blue());
                auto brush = Brush();
                brush.setColor(Color(0, 0, 255, 0.25));

                drawable->drawLine(rectLine, pen);
                drawable->drawPolygon(std::make_shared<PolygonGeometry>(points), pen, brush);

                Pen p;
                p.setAntiAlias(true);
                c1 = Color(c1.r(), c1.g(), c1.b(), 1.0);
                auto c2 = Color(255-c1.r(), 255-c1.g(), 255-c1.b(), 1.0);
                p.setColors(Color::colorRamp(c1, c2, line->points().size()));
                p.setThickness(3.0);

                // Draw the trace as a line
                drawable->drawLine(line, p);

                if (!m_trace.empty())
                {
                    Brush b;
                    b.setAntiAlias(true);
                    b.setColor(c1);

                    const auto& lastPos = m_trace[m_trace.size()-1].first; // Last added pos
                    double radius = AnimationFunctions::easeInCubic((double)line->points().size() / (double)m_traceSize);
                    radius = std::max(2.0, 10.0*radius);

                    Pen blackPen;
                    blackPen.setColor(Color::black());
                    drawable->drawCircle(lastPos.x, lastPos.y, radius, blackPen, b);

                    // We still have stuff left
                    map.update(); // TODO: This may trigger more updates than needed. Use timer instead
                }
            }

            bool onMouseMove(const MouseMoveEvent& event) override final
            {
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
            KeyActionTool(LayerSetPtr layerToDrop)
                : m_map(nullptr)
                , m_tileLayerToDrop(layerToDrop)
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

            bool onKeyDown(const KeyDownEvent& event) override final
            {
                Key keyStroke(event.keyCode);
            
                BMM_DEBUG() << "Key stroke: " << keyStroke << " (" << keyStroke.toString() << ")\n";
                BMM_DEBUG() << "Key action controller: " << event.keyCode << "\n";

                if (keyStroke == Key::S)
                    BMM_DEBUG() << "OHHH YAAS\n";

                
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

                    m_map->update();
                    return true;
                }

                if (keyStroke == Key::F &&
                    event.modificationKey && ModificationKeyCtrl)
                {
                    BMM_DEBUG() << "Flushing cache!!!\n";
                    m_map->flushCache();
                    m_map->update();
                }

                // TODO rendering enabled
                if (keyStroke == Key::R && //27 && // r
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
                
                if (keyStroke == Key::D && // d
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

            bool onDrop(const DropEvent& event) override final
            {
                BMM_DEBUG() << "On drop...\n";
                static std::mutex             mtx;
                static std::vector<DataSetId> addedDataSets;

                {
                    std::lock_guard lock(mtx);
                    addedDataSets.clear();
                }


                for (const auto& file : event.paths)
                {
                    auto backgroundImageDataSet = std::make_shared<ImageDataSet>(file);
                    try
                    {
                        backgroundImageDataSet->initialize(DataSetInitializationType::RightHereRightNow);
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << '\n';
                        return false;
                    }

                    static StandardLayerPtr backgroundLayer;
                    
                    m_tileLayerToDrop->flushCache();
                    //m_tileLayerToDrop->layers().clear();
                    if (backgroundLayer)
                    {
                        m_tileLayerToDrop->removeLayer(backgroundLayer);
                    }

                    backgroundLayer = std::make_shared<StandardLayer>(false);
                    backgroundLayer->name("DropLayer");
                    backgroundLayer->addDataSet(backgroundImageDataSet);
                    auto rasterVis = std::make_shared<RasterVisualizer>();
                    rasterVis->alpha(DirectDoubleAttributeVariable(1.0));
                    backgroundLayer->visualizers().push_back(rasterVis);

                    m_tileLayerToDrop->layers().insert(
                        m_tileLayerToDrop->layers().begin(),
                        backgroundLayer
                    );
                    
                    auto features = backgroundImageDataSet->getFeatures(FeatureQuery());

                    // Calculate bounds of the features and set the map view accordingly
                    std::vector<Rectangle> boundsList;
                    while (features->moveNext())
                    {
                        auto feature = features->current();
                        boundsList.push_back(feature->bounds());
                    }

                    m_map->zoomTo(backgroundImageDataSet->crs()->projectTo(
                        m_map->crs(), 
                        Rectangle::mergeBounds(boundsList)));

                    break;
                }

                m_map->update();

                return true;
            }

        private:
            MapPtr m_map;
            LayerSetPtr m_tileLayerToDrop;
    };
    typedef std::shared_ptr<KeyActionTool> KeyActionToolPtr;

    class NorthArrowTool : public Tool
    {
        static constexpr double ArrowLength = 20.0;
        static constexpr double ArrowWidth = 5.0;

        public:
            NorthArrowTool()
                : m_map(nullptr)
                , m_isHovered(false)
                , m_isSelected(false)
            {}

            bool isActive() { return false; }

            void onConnected(const MapControlPtr& control, const MapPtr& map) override final
            {
                m_map = map;
                m_map->events.onCustomDraw.subscribe(this, &NorthArrowTool::onCustomDraw);
            }

            void onDisconnected() override final
            {
                m_map = nullptr;
                m_map->events.onCustomDraw.unsubscribe(this);
            }
        private:

            bool onMouseMove(const MouseMoveEvent& event) override final
            {
                auto mouse = Point(event.pos.x, event.pos.y);
                auto pos = northArrowPos(m_map->drawable());
                
                m_isHovered = pos.distanceTo(mouse) < ArrowLength;
                if (!m_isHovered && m_isSelected)
                {
                    m_isSelected = false;
                }

                return false;
            }

            bool onMouseDown(const MouseDownEvent& event) override final
            {
                auto mouse = Point(event.pos.x, event.pos.y);
                auto pos = northArrowPos(m_map->drawable());
                
                m_isSelected = pos.distanceTo(mouse) < ArrowLength;

                if (m_isSelected)
                {
                    m_map->rotateTo(0.0);
                    return true;
                }

                return false;
            }

            bool onClick(const ClickEvent& event) override final
            {
                auto mouse = Point(event.pos.x, event.pos.y);
                auto pos = northArrowPos(m_map->drawable());
                
                m_isSelected = pos.distanceTo(mouse) < ArrowLength;

                if (m_isSelected)
                {
                    m_map->rotateTo(0.0);
                    return true;
                }

                return false;
            }

            bool onMouseUp(const MouseUpEvent& event) override final
            {
                m_isSelected = false;

                return false;
            }

            void onCustomDraw(Map& map)
            {
                drawNorthArrow(map.drawable());
            }

            void drawNorthArrow(const DrawablePtr& drawable)
            {
                auto pos = northArrowPos(drawable);

                Pen pen;
                pen.setAntiAlias(true);
                Brush brush;

                if (m_isSelected)
                {
                    brush.setColor(Color(50,50,255,0.5));
                }
                else if (m_isHovered)
                {
                    brush.setColor(Color(50,50,50,0.75));
                }
                else
                {
                    brush.setColor(Color(50,50,50,0.5));
                }
                drawable->drawCircle(pos.x(), pos.y(), ArrowLength, Pen::transparent(), brush);

                // Calculate direction of grid north
                auto screenCenter = m_map->screenCenter();
                auto mapCenter = m_map->screenToMap(screenCenter);
                auto screenOffset = m_map->mapToScreen(mapCenter + Point(0.0, 1.0));
                auto screenUpDirection = (screenOffset-screenCenter).norm();
                auto screenRightDirection = Point(-screenUpDirection.y(), screenUpDirection.x());

                auto arrowUp = calcArrowGeometry(pos, screenUpDirection, screenRightDirection);
                auto arrowDown = calcArrowGeometry(pos, screenUpDirection * (-1.0), screenRightDirection);


                brush.setColor(Color(255, 0, 0, 0.9));
                drawable->drawPolygon(std::make_shared<PolygonGeometry>(arrowUp->points()), pen, brush);
                brush.setColor(Color::white(0.9));
                drawable->drawPolygon(std::make_shared<PolygonGeometry>(arrowDown->points()), pen, brush);

            }

            LineGeometryPtr calcArrowGeometry(const Point& screenPos, const Point& screenUpDirection, const Point& screenRightDirection) const
            {
                auto points = std::vector<Point>();

                // Top of north arrow
                points.push_back(screenPos + (screenRightDirection * -ArrowWidth));
                points.push_back(screenPos + (screenUpDirection * ArrowLength));
                points.push_back(screenPos + (screenRightDirection * ArrowWidth));
                points.push_back(screenPos + (screenUpDirection * ArrowWidth));
                
                LineGeometryPtr line = std::make_shared<LineGeometry>(points);
                line->isClosed(true);

                // points.push_back(screenPos + (screenUpDirection * -ArrowLength));
                // points.push_back(screenPos + (screenRightDirection * ArrowWidth));
                // points.push_back(screenPos + (screenUpDirection * ArrowLength));
                // points.push_back(screenPos + (screenRightDirection * -ArrowWidth));



                return line;
            }

            Point northArrowPos(const DrawablePtr& drawable)
            {
                int x = drawable->width() - ArrowLength * 2.0;
                int y = ArrowLength * 2.0;

                return Point(x,y);
            }

            MapPtr m_map;
            bool m_isHovered;
            bool m_isSelected;
    };

    class GpxVisualizerTool : public Tool
    {
        public:
            GpxVisualizerTool()
                : m_map(nullptr)
            {}

            bool isActive() { return false; }

            void onConnected(const MapControlPtr& control, const MapPtr& map) override final
            {
                m_map = map;

                m_midnattsloppetDataSet = std::make_shared<BlueMarble::MemoryDataSet>();
                m_midnattsloppetDataSet->name("Midnattsloppet");
                m_midnattsloppetDataSet->initialize(DataSetInitializationType::RightHereRightNow);

                auto midnattsloppetLayer = BlueMarble::StandardLayerPtr(new BlueMarble::StandardLayer());
                midnattsloppetLayer->addDataSet(m_midnattsloppetDataSet);
                midnattsloppetLayer->selectable(false);
                // midnattsloppetLayer->minScale(1.0/100000.0);

                auto trackVis = std::make_shared<LineVisualizer>();
                trackVis->color([](const FeaturePtr& f, Attributes& updateAttributes)
                {
                    std::string type = f->attributes().get<std::string>("type");
                    double alpha = f->attributes().get<double>("alpha", 1.0);
                    return Color(255*(1-alpha), 0, 255*alpha);
                    
                });
                trackVis->width(DirectDoubleAttributeVariable(3));
                midnattsloppetLayer->visualizers().push_back(trackVis);

                // Add simple circle symbol visualizer for the track points
                auto symbolVis = std::make_shared<SymbolVisualizer>();
                symbolVis->condition([](const FeaturePtr& f, Attributes& updateAttributes)
                {
                    std::string type = f->attributes().get<std::string>("type");
                    return type == "ground";
                });
                symbolVis->symbol(SymbolVisualizer::Symbol(BuiltInSymbol::Circle));
                symbolVis->size(DirectDoubleAttributeVariable(5));
                symbolVis->color([](const FeaturePtr& f, Attributes& updateAttributes)
                {
                    std::string type = f->attributes().get<std::string>("type");
                    if (type == "ground")
                        return Color(255, 0, 0);
                    else if (type == "elevation")
                        return Color(0, 0, 255);
                    else
                        return Color(0, 255, 0);
                });
                midnattsloppetLayer->visualizers().push_back(symbolVis);

                map->addLayer(midnattsloppetLayer);
            }

            void onDisconnected() override final
            {
                m_map = nullptr;
            }

            bool onDrop(const DropEvent& event) override final
            {
                for (const auto& gpxPath : event.paths)
                {
                    std::ifstream gpxFile(gpxPath);
                    if (!gpxFile.is_open())
                    {
                        std::cout << "Could not open: " << gpxPath << std::endl;
                        continue;
                    }
                    else
                    {
                        // Check if it has gpx extension
                        if (gpxPath.substr(gpxPath.find_last_of(".") + 1) != "gpx")
                        {
                            std::cout << "Not a GPX file: " << gpxPath << std::endl;
                            return false;
                        }
                        m_midnattsloppetDataSet->clear();

                        std::ostringstream ss;
                        ss << gpxFile.rdbuf();
                        const std::string gpxContent = ss.str();

                        // Match each <wpt lat="..." lon="...">...</wpt> block
                        // Match each <trkpt lat="..." lon="..."><ele>...</ele></trkpt>
                        std::regex trkptRe(R"re(<trkpt\s[^>]*lat="([^"]+)"[^>]*lon="([^"]+)"[^>]*>([\s\S]*?)</trkpt>)re",
                                        std::regex::ECMAScript);
                        std::regex eleRe(R"re(<ele>([^<]+)</ele>)re");

                        std::vector<Point> trackPoints;
                        std::vector<Point> trackPointsWithElevation;
                        
                        auto begin = std::sregex_iterator(gpxContent.begin(), gpxContent.end(), trkptRe);
                        auto end   = std::sregex_iterator();
                        for (auto it = begin; it != end; ++it)
                        {
                            double lat = std::stod((*it)[1].str());
                            double lon = std::stod((*it)[2].str());
                            double ele = 0.0;
                            std::smatch eleMatch;
                            const std::string body = (*it)[3].str();
                            if (std::regex_search(body, eleMatch, eleRe))
                                ele = std::stod(eleMatch[1].str());

                            trackPoints.emplace_back(lon, lat, 0.0);
                            trackPointsWithElevation.emplace_back(lon, lat, ele*5.5); // Scale elevation for better visibility
                        }
                        std::cout << "Loaded " << trackPoints.size() << " GPX track points from " << gpxPath << "\n";

                        // Curtain polygon: forward along ground (z=0), backward along elevated track.
                        // The start and end points coincide in lat/lon, closing the ring naturally.
                        double maxElevation = 0.0;
                        std::vector<Point> curtainPoints;
                        curtainPoints.reserve(trackPoints.size() * 2);
                        for (const auto& p : trackPoints)
                            curtainPoints.push_back(p);
                        for (auto it = trackPointsWithElevation.rbegin(); it != trackPointsWithElevation.rend(); ++it)
                        {
                            curtainPoints.push_back(*it);
                            if (it->z() > maxElevation)
                                maxElevation = it->z();
                        }

                        auto curtainOutlineFeature = m_midnattsloppetDataSet->createFeature(
                            std::make_shared<PolygonGeometry>(curtainPoints));
                        curtainOutlineFeature->attributes().set("type", std::string("curtain"));
                        m_midnattsloppetDataSet->addFeature(curtainOutlineFeature);

                        // Create scaled polygons curtains with less and less alpha to create a fading effect
                        int nCurtainLayers = 20;
                        for (int i = 0; i < nCurtainLayers; ++i)
                        {
                            double scale = maxElevation > 0.0 ? 1.0 - (double)i / (double)nCurtainLayers : 1.0;
                            std::vector<Point> scaledCurtainPoints;
                            scaledCurtainPoints.reserve(curtainPoints.size());
                            for (const auto& p : curtainPoints)
                            {
                                Point scaledP = Point(p.x(), p.y(), p.z() * scale);
                                scaledCurtainPoints.push_back(scaledP);
                            }
                            auto curtainFeature = m_midnattsloppetDataSet->createFeature(
                                std::make_shared<PolygonGeometry>(scaledCurtainPoints));
                            curtainFeature->attributes().set("type", std::string("curtain"));
                            curtainFeature->attributes().set("alpha", 0.4 * (1.0 - (double)i / (double)nCurtainLayers));
                            m_midnattsloppetDataSet->addFeature(curtainFeature);
                        }   
                        // auto trackFeature = m_midnattsloppetDataSet->createFeature(
                        //     std::make_shared<LineGeometry>(trackPoints));
                        // trackFeature->attributes().set("type", std::string("ground"));
                        // m_midnattsloppetDataSet->addFeature(trackFeature);
                        // auto trackFeatureWithElevation = m_midnattsloppetDataSet->createFeature(
                        //     std::make_shared<LineGeometry>(trackPointsWithElevation));
                        // trackFeatureWithElevation->attributes().set("type", std::string("elevation"));
                        // m_midnattsloppetDataSet->addFeature(trackFeatureWithElevation);

                        

                        // Curtain polygon visualizer
                        // auto curtainVis = std::make_shared<PolygonVisualizer>();
                        // curtainVis->color(DirectColorAttributeVariable(Color(255, 80, 0, 0.4)));
                        // m_midnattsloppetLayer->visualizers().push_back(curtainVis);

                        //auto bounds = curtainOutlineFeature->bounds();
                        auto bounds = m_midnattsloppetDataSet->crs()->projectTo(
                            m_map->crs(),
                            curtainOutlineFeature->bounds());
                        m_map->zoomTo(bounds.scaled(1.2));

                        break;
                    }
                }

                m_map->update();
                return true;
            }

        private:
            MapPtr m_map;
            MemoryDataSetPtr m_midnattsloppetDataSet;
    };

    class GifRecorderTool : public Tool
    {
        public:
            static constexpr int64_t RECORD_INTERVAL_MS = 30; // Record every 30 ms
            
            GifRecorderTool()
                : m_map(nullptr)
            {}

            bool isActive() { return false; }

            void onConnected(const MapControlPtr& control, const MapPtr& map) override final
            {
                m_map = map;
                m_map->events.onCustomDraw.subscribe(this, &GifRecorderTool::onCustomDraw);
            }

            void onDisconnected() override final
            {
                m_map = nullptr;
                m_map->events.onCustomDraw.unsubscribe(this);
            }

            bool onKeyDown(const KeyDownEvent& event) override final
            {
                Key keyStroke(event.keyCode);
            
                if (keyStroke == Key::G &&
                    event.modificationKey && ModificationKeyCtrl)
                {
                    if (m_recorderState != GifRecorderState::Recording)
                    {
                        if (m_recorderState == GifRecorderState::Saving)
                        {
                            // Cancel current save
                            m_recorderState = GifRecorderState::Idle;
                            m_stopToken = true;
                            
                            BMM_DEBUG() << "Interrupting GIF saving...\n";
                            m_saveThread.join();
                            m_recordedFrames.clear(); // Clear after saving is interrupted
                        }
                        BMM_DEBUG() << "Starting GIF recording...\n";
                        m_recorderState = GifRecorderState::Recording;
                        
                        m_map->events.onUpdated.subscribe(this, &GifRecorderTool::onUpdated);
                    }
                    else
                    {
                        BMM_DEBUG() << "Stopping GIF recording...\n";

                        m_map->events.onUpdated.unsubscribe(this);

                        // Save the recorded frames to a GIF file on a background thread
                        m_recorderState = GifRecorderState::Saving;
                        m_stopToken = false;
                        m_nRecordedFrames = m_recordedFrames.size(); // Needed for progress bar

                        if (m_saveThread.joinable())
                        {
                            m_saveThread.join();
                        }
                        m_saveThread = std::thread([this]()
                        {
                            saveRecording(m_stopToken, m_recordedFrames, m_recordedFramesMutex);
                            m_recorderState = GifRecorderState::Idle;
                        });
                        BMM_DEBUG() << "Saving GIF in background thread...\n";
                    }
                }

                return false;
            }

            void onCustomDraw(Map& map)
            {
                const Point indicatorPos{20.0, 20.0};
                

                if (m_recorderState == GifRecorderState::Recording)
                {
                    static auto radiusProgressEval = AnimationFunctions::AnimationBuilder().subDivide(2).bounce().inverseAt(0.5).build();
                    // static auto radiusProgressEval = AnimationFunctions::AnimationBuilder().subDivide(2).easeInCubic().inverseAt(0.5).build();
                    auto drawable = map.drawable();
                    Brush b;
                    
                    
                    double radius = radiusProgressEval((double)(map.updateAttributes().get<int>(UpdateAttributeKeys::UpdateTimeMs) % 2000) / 2000.0) * 5.0 + 7.0;

                    b.setColor(Color::white());
                    drawable->drawCircle(indicatorPos.x(), indicatorPos.y(), radius*1.2, Pen::transparent(), b);
                    b.setColor(Color::red());
                    drawable->drawCircle(indicatorPos.x(), indicatorPos.y(), radius, Pen::transparent(), b);
                }
                if (m_recorderState == GifRecorderState::Saving)
                {
                    m_recordedFramesMutex.lock();
                    int currSize = m_recordedFrames.size();
                    m_recordedFramesMutex.unlock();

                    double progress = (double)(m_nRecordedFrames - currSize) / (double)m_nRecordedFrames;

                    auto drawable = map.drawable();
                    LineGeometryPtr outline = std::make_shared<LineGeometry>();
                    outline->points().push_back(Point(indicatorPos.x() - 15, indicatorPos.y() - 15));
                    outline->points().push_back(Point(indicatorPos.x() + 15, indicatorPos.y() - 15));
                    outline->points().push_back(Point(indicatorPos.x() + 15, indicatorPos.y() + 15));
                    outline->points().push_back(Point(indicatorPos.x() - 15, indicatorPos.y() + 15));
                    outline->isClosed(true);
                    Pen p;
                    p.setColor(Color::black());
                    drawable->drawLine(outline, p);
                    Brush b;
                    b.setColor(Color::green());
                    Rectangle progressRect(indicatorPos.x() - 15, 
                                           indicatorPos.y() - 15, 
                                           indicatorPos.x() - 15 + progress * 30, 
                                           indicatorPos.y() + 15);
                    drawable->drawPolygon(std::make_shared<PolygonGeometry>(progressRect), p, b);
                }
            }

            void onUpdated(Map& map)
            {
                static int64_t lastRecordTimeMs = 0;
                int64_t updateTimeMs = map.updateAttributes().get<int>(UpdateAttributeKeys::UpdateTimeMs);
                int64_t elapsedTimeMs = updateTimeMs - lastRecordTimeMs;
                if (m_recorderState == GifRecorderState::Recording 
                    && elapsedTimeMs > RECORD_INTERVAL_MS) // Record every 100 ms
                {
                    auto drawable = map.drawable();
                    m_recordedFrames.emplace_back(std::move(drawable->getRaster()));
                    // int width = drawable->width();
                    // int height = drawable->height();
                    // GifWriteFrame(&m_gifWriter, (uint8_t*)drawable->getRaster().data(), width, height, 100);
                    lastRecordTimeMs = updateTimeMs;
                }
            }

        private:

            // On background thread, save the recorded frames to a GIF file
            static void saveRecording(const std::atomic<bool>& stopToken, 
                                      std::deque<Raster>& recordedFrames, 
                                      std::mutex& recordedFramesMutex)
            {
                if (recordedFrames.empty())
                {
                    BMM_DEBUG() << "No frames to save.\n";
                    return;
                }

                auto fileName = "bmm_test.gif";
                int delay = RECORD_INTERVAL_MS / 10; // Delay in 1/100th of a second
                int width = recordedFrames[0].width();
                int height = recordedFrames[0].height();

                GifWriter g;
                GifBegin(&g, fileName, width, height, delay);
                
                while (!stopToken)
                {
                    recordedFramesMutex.lock();
                    if (recordedFrames.empty())
                    {
                        recordedFramesMutex.unlock();
                        break;
                    }
                    auto frame = std::move(recordedFrames.front());
                    recordedFrames.pop_front();
                    recordedFramesMutex.unlock();

                    GifWriteFrame(&g, (uint8_t*)frame.data(), width, height, delay);
                }

                BMM_DEBUG() << "Finished writing frames. Storing to " << fileName << "...\n";

                GifEnd(&g);

                BMM_DEBUG() << "Saved GIF to " << fileName << "\n";
            }

            enum class GifRecorderState
            {
                Idle,
                Recording,
                Saving
            };

            MapPtr m_map;
            std::atomic<GifRecorderState> m_recorderState = {GifRecorderState::Idle};
            std::thread m_saveThread;
            std::atomic<bool> m_stopToken{false};
            std::mutex m_recordedFramesMutex;
            std::deque<Raster> m_recordedFrames;
            int m_nRecordedFrames = 0;
    };

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

        bool onClick(const ClickEvent& event) override final
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
