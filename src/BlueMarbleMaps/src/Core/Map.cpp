#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Utility/Utils.h"
#include "BlueMarbleMaps/Core/DataSets/DataSet.h"
#include "BlueMarbleMaps/Core/Layer/LayerSet.h"
#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/SoftwareDrawable.h"
#include "BlueMarbleMaps/Logging/Logging.h"

#include "BlueMarbleMaps/Core/OpenGLDrawable.h"
#include "Platform/OpenGL/Mesh.h"

#include <cmath>
#include <iostream>
#include <vector>
#include <set>


using namespace BlueMarble;

Map::Map()
    : m_crs(Crs::wgs84LngLat())
    , m_surfaceModel(std::make_shared<PlaneSurfaceModel>(Point{0,0,0}, Point{0,0,1}))
    , m_updateRequired(true)
    , m_updateEnabled(true)
    , m_updateAttributes()
    , m_cameraController(nullptr)
    , m_lastUpdateTimeStamp(-1)
    , m_presentationObjects()
    , m_selectedFeatures()
    , m_hoveredFeatures()
    , m_showDebugInfo(false)
    , m_isUpdating(false)
    , m_renderingEnabled(true)
{
    m_drawable = std::make_shared<SoftwareBitmapDrawable>(500, 500, 4);
    m_camera = Camera::perspectiveCamera(m_drawable->width(), m_drawable->height(), 0.1, 1.0, 45.0);
    // TODO: set reasonable start position of the camera

    m_presentationObjects.reserve(100000); // Reserve a good amount for efficiency

    m_lastUpdateTimeStamp = getTimeStampMs();
    updateUpdateAttributes(m_lastUpdateTimeStamp);
}

bool Map::update(bool forceUpdate)
{
    if (!forceUpdate && m_mapControl)
    {
        // Let MapControl schedule update
        // Note: this is allowed to be called within an update
        m_mapControl->updateView();
        return true;
    }

    assert(!m_isUpdating);
    
    m_isUpdating = true;
    // TODO: should be handled as int64_t
    int timeStampMs = getTimeStampMs();
    int64_t deltaMs = timeStampMs - m_lastUpdateTimeStamp;
    m_lastUpdateTimeStamp = timeStampMs;

    if (!m_updateEnabled || (!forceUpdate && !m_updateRequired))
    {
        std::cout << "Map::update() No update required!\n";
        return false;
    }

    m_updateRequired = false;
    updateUpdateAttributes(timeStampMs); // Set update attributes that contains useful information about the update

    // Update camera using the camera controller
    if (m_cameraController)
    {
        ICameraController::ControllerStatus status = m_cameraController->updateCamera(m_camera, deltaMs);
        if (hasFlag(status, ICameraController::ControllerStatus::Updated)
            || hasFlag(status, ICameraController::ControllerStatus::NeedsUpdate))
        {
            events.onCameraChanged.notify(*this);
        }
        if (hasFlag(status, ICameraController::ControllerStatus::NeedsUpdate))
        {
            m_updateRequired = true;
        }
        else
        {
            // BMM_DEBUG() << "No camera update needed\n";
        }
    }

    events.onUpdating.notify(*this);

    // Set camera frustom and call clearBuffer
    
    beforeRender();
    renderLayers(); // Let layers do their work
    
    auto proj = ScreenCameraProjection(m_drawable->width(), m_drawable->height());

    // Each onCustomDraw notification should have the transform set to "screen".
    // Since handlers are allowed to modify the transform, we need to make sure to reset it
    // each time a handler is called.
    auto preNotifyAction = [this, &proj]()
    {
        m_drawable->setProjectionMatrix(proj.projectionMatrix());
        m_drawable->setViewMatrix(glm::mat4(1.0));
        m_drawable->beginBatches();
    };
    auto postNotifyAction = [this]()
    {
        m_drawable->endBatches();
    };
    events.onCustomDraw.notify(*this, preNotifyAction, postNotifyAction);

    if (m_showDebugInfo)
    {
        drawDebugInfo(getTimeStampMs() - timeStampMs);
    }

    // Swap buffers
    afterRender();

    events.onUpdated.notify(*this);

    m_updateRequired |= m_updateAttributes.get<bool>(UpdateAttributeKeys::UpdateRequired); // Someone in the operator chain needs more updates (e.g. Visualization evaluations)

    m_isUpdating = false;

    bool updateRequired = m_updateRequired;
    if (!updateRequired)
    {
        events.onIdle.notify(*this);
    }

    return updateRequired;
}

void Map::renderLayer(const LayerPtr& layer, const FeatureQuery& featureQuery)
{
    auto prepared = layer->prepare(crs(), featureQuery);
    m_drawable->makeCurrent();
    layer->update(shared_from_this(), prepared, featureQuery);
}

void Map::renderLayers()
{
    m_presentationObjects.clear(); // Clear presentation objects, layers will add new

    FeatureQuery featureQuery = produceUpdateQuery();

    // Switch drawable to offscreen for layers
    auto originalDrawable = m_drawable;
    // m_drawable = m_offscreenDrawable; // // TODO: add back

    for (const auto& l : m_layers)
    {
        // TODO add "ViewInfo" as parameter to Layer::update()?
        //l->update(shared_from_this(), getCrs(), featureQuery);
        renderLayer(l, featureQuery);
    }
    
    m_drawable = originalDrawable;

    // drawTestElevationMesh(); // TODO: add back

    m_drawable->makeCurrent();
    // Debug draw update area
    m_drawable->beginBatches();
    auto line = std::make_shared<LineGeometry>(featureQuery.area());
    Pen p;
    p.setColor(Color::red());
    p.setThickness(5.0);
    //m_drawable->setTransform(Transform::screenTransform(m_drawable->width(), m_drawable->height()));
    m_drawable->drawLine(line, p);
    m_drawable->endBatches();
}

FeatureQuery Map::produceUpdateQuery()
{
    int w = m_drawable->width();
    int h = m_drawable->height();
    auto screenArea = Rectangle(0,0,w,h);
    screenArea.scale(0.99); // TODO: this scaling is for debugging querying, remove

    return produceUpdateQuery(screenArea);
}

FeatureQuery BlueMarble::Map::produceUpdateQuery(const Rectangle& screenArea)
{
    // Map to camera
    auto centerMap = screenToMap(screenArea.center());
    if (centerMap.isUndefined())
    {
        BMM_DEBUG() << "Map::produceUpdateQuery() Failed to get map position at center of screen!\n";
        throw std::runtime_error("GG");
    }

    // Map to camera
    auto centerCam = m_camera->worldToView(centerMap);
    double zCam = centerCam.z();
    // double unitsPerPixel = m_camera->projection()->unitsPerPixelAtDistanceNumerical(std::abs(zCam));
    double unitsPerPixel = m_camera->unitsPerPixelAtDistance(std::abs(zCam));
    double queryScale = 1.0 / unitsPerPixel * m_drawable->pixelSize() / m_crs->globalMetersPerUnit();
    
    FeatureQuery featureQuery;
    featureQuery.scale(queryScale);
    featureQuery.resolution(unitsPerPixel);
    featureQuery.area(screenToMap(screenArea).cropped(crs()->bounds()));
    featureQuery.quickUpdate(quickUpdateEnabled());
    featureQuery.updateAttributes(&updateAttributes());

    return featureQuery;
}

double Map::invertedScale() const
{
    auto centerMap = screenToMap(screenCenter());
    auto centerCam = m_camera->worldToView(centerMap);
    double zCam = centerCam.z();
    // double unitsPerPixel = m_camera->projection()->unitsPerPixelAtDistanceNumerical(std::abs(zCam));
    double unitsPerPixel = m_camera->unitsPerPixelAtDistance(std::abs(zCam));
    double aprroximateScale = unitsPerPixel * m_crs->globalMetersPerUnit() / m_drawable->pixelSize();

    return aprroximateScale;
}

double Map::scale() const
{
    return 1.0 / invertedScale();
}

void Map::crs(const CrsPtr& newCrs)
{
    auto lngLatCrs = Crs::wgs84LngLat();

    auto oldCrs = crs();
    // auto centerLngLat = crs()->projectTo(lngLatCrs, center());
    // double oldScale = scale();

    m_crs = newCrs;

    // center(lngLatCrs->projectTo(newCrs, centerLngLat));
    // scale(oldScale);

    // m_surfaceModel = std::make_shared<PlaneSurfaceModel>(Point{0,0,0}, Point{0,0,1});

    if (m_cameraController)
    {
        m_cameraController->onCrsChanged(m_crs);
    }

    flushCache(); // We need to flush layer caches since the crs has changed

    events.onCrsChanged.notify(*this, oldCrs, newCrs);
}

void Map::setSurfaceModel(const SurfaceModelPtr &model)
{
    auto oldSurfaceModel = m_surfaceModel;
    m_surfaceModel = model;

    if (m_cameraController)
    {
        m_cameraController->onSurfaceModelChanged(m_surfaceModel);
    }

    flushCache(); // We need to flush layer caches since the surface model has changed

    events.onSurfaceModelChanged.notify(*this, oldSurfaceModel, m_surfaceModel);
}

void Map::setCameraController(ICameraController* controller)
{
    if (m_cameraController)
    {
        m_cameraController->onDeactivated();
    }

    if (controller)
    {
        m_cameraController = controller;
        m_cameraNavigator = dynamic_cast<ICameraNavigator*>(controller);
        m_camera = m_cameraController->onActivated(m_camera, m_crs, m_surfaceModel);
    }
    else
    {
        m_cameraController = nullptr;
        m_cameraNavigator = nullptr;
    }
}

void Map::panTo(const Point &target)
{
    if (m_cameraNavigator)
    {
        m_cameraNavigator->panTo(target);
    }
}

void Map::rotateTo(double angle)
{
    if (m_cameraNavigator)
    {
        m_cameraNavigator->rotateTo(angle);
    }
}

void Map::zoomTo(const Rectangle &bounds)
{
    if (m_cameraNavigator)
    {
        m_cameraNavigator->zoomTo(bounds);
    }
}

Point Map::pixelToScreen(const Point &pixel) const
{
    return pixelToScreen((int)std::round(pixel.x()), (int)std::round(pixel.y()));
}

Point Map::pixelToScreen(int px, int py) const
{
    return Point(double(px)+0.5, double(py)+0.5);
}

Point Map::screenToPixel(const Point& screen) const
{
    return screenToPixel(screen.x(), screen.y());
}

Point Map::screenToPixel(double x, double y) const
{
    return Point(std::floor(x), std::floor(y));
}

Point Map::screenToMap(const Point& screenPos) const
{
    return screenToMap(screenPos.x(), screenPos.y());
}

Point Map::screenToMap(double x, double y) const
{
    double xNdc,yNdc;
    screenToNDC(x, y, xNdc, yNdc);

    auto ray = m_camera->ndcToWorldRay(Point(xNdc, yNdc, -1.0));
    Point rayOriginWorldPoint = ray.origin;
    Point rayDirWorldPoint = ray.direction;

    Point surfacePoint = Point::undefined();
    Point surfaceNormal = Point::undefined();
    if (!m_surfaceModel->rayIntersection(rayOriginWorldPoint, 
                                         rayDirWorldPoint.norm3D(), 
                                         0.0,
                                         surfacePoint, 
                                         surfaceNormal))
    {
        // BMM_DEBUG() << "Map::screenToMap() No map intersection!\n";
    }
    
    return surfacePoint;
}

Point Map::screenToMapAtHeight(const Point& screenPos, double heightMeters) const
{
    double xNdc,yNdc;
    screenToNDC(screenPos.x(), screenPos.y(), xNdc, yNdc);

    auto ray = m_camera->ndcToWorldRay(Point(xNdc, yNdc, -1.0));
    Point rayOriginWorldPoint = ray.origin;
    Point rayDirWorldPoint = ray.direction;

    Point surfacePoint = Point::undefined();
    Point surfaceNormal = Point::undefined();
    if (!m_surfaceModel->rayIntersection(rayOriginWorldPoint, 
                                         rayDirWorldPoint.norm3D(), 
                                         heightMeters/m_crs->globalMetersPerUnit(),
                                         surfacePoint, 
                                         surfaceNormal))
    {
        // BMM_DEBUG() << "Map::screenToMapAtHeight() No map intersection!\n";
    }
    
    return surfacePoint;
}

Point Map::mapToScreen(const Point& point) const
{   
    Point ndc = m_camera->worldToNdc(point);
    double x,y;
    ndcToScreen(ndc.x(), ndc.y(), x, y);

    return Point(x, y);
}

Ray Map::screenToViewRay(double pixelX, double pixelY) const
{
    double xNdc,yNdc;
    screenToNDC(pixelX, pixelY, xNdc, yNdc);

    return m_camera->projection()->ndcToViewRay(Point(xNdc, yNdc));
}

Ray Map::screenToMapRay(double x, double y) const
{
    double ndcX, ndcY;
    screenToNDC(x, y, ndcX, ndcY);

    return m_camera->ndcToWorldRay({ndcX, ndcY});
}

void Map::screenToNDC(double x, double y, double &ndcX, double &ndcY) const
{
    // TODO: "ndc" should be owned by the drawable
    ndcX = double(x * 2.0 / double(m_drawable->width()) - 1.0);
    ndcY = double(1.0 - y * 2.0 / double(m_drawable->height()));
}

void Map::ndcToScreen(double ndcx, double ndcy, double &x, double &y) const
{
    // TODO: "ndc" should be owned by the drawable
    x = (ndcx + 1.0) * 0.5 * m_drawable->width();
    y = (1.0- ndcy) * 0.5 * m_drawable->height();
}

std::vector<Point> Map::screenToMap(const std::vector<Point> &points) const
{
    std::vector<Point> mapPoints;
    for (auto& p : points)
    {
        mapPoints.push_back(screenToMap(p));
    }

    return mapPoints;
}

std::vector<Point> Map::mapToScreen(const std::vector<Point>& points) const
{
    std::vector<Point> screenPoints;
    for (auto& p : points)
    {
        screenPoints.push_back(mapToScreen(p));
    }
    return screenPoints;
}

std::vector<Point> Map::lngLatToMap(const std::vector<Point> &points) const
{
    std::vector<Point> mapPoints;
    for (auto& p : points)
    {
        mapPoints.push_back(lngLatToMap(p));
    }

    return mapPoints;
}

Rectangle Map::screenToMap(const Rectangle& rect) const
{
    return Rectangle::fromPoints(screenToMap(rect.corners()));
}

Rectangle BlueMarble::Map::mapToScreen(const Rectangle& rect) const
{
    return Rectangle::fromPoints(mapToScreen(rect.corners()));
}

Point Map::screenCenter() const
{
    return Point(m_drawable->width()*0.5, m_drawable->height()*0.5); // Screen center
    //return Point((m_drawable->width()-1)*0.5, (m_drawable->height()-1)*0.5); // Pixel center
}

void Map::addLayer(const LayerPtr& layer)
{
    assert(layer != nullptr);
    m_layers.push_back(layer);
}

std::vector<LayerPtr>& Map::layers()
{
    return m_layers;
}

std::vector<FeaturePtr> Map::featuresAt(int X, int Y, double pointerRadius)
{
    std::vector<FeaturePtr> hitFeatures;
    for (auto& po : hitTest(X, Y, pointerRadius))
    {
        // Only push unique features. NOTE: we don't use a set since it does not retain the order
        bool alreadyExists = false;
        for(auto f : hitFeatures)
        {
            if (f == po.sourceFeature())
                alreadyExists = true;
        }
        if(!alreadyExists)
            hitFeatures.push_back(po.sourceFeature());
    }

    return hitFeatures;
}

void Map::featuresInside(const Rectangle& bounds, FeatureCollection& hitFeatures)
{
    assert(hitFeatures.size() == 0);

    for (auto& po : hitTest(bounds))
    {
        // Only push unique features. NOTE: we don't use a set since it does not retain the order
        bool alreadyExists = false;
        for(auto f : hitFeatures)
        {
            if (f == po.sourceFeature())
                alreadyExists = true;
        }
        if(!alreadyExists)
            hitFeatures.add(po.sourceFeature());
    }
}

const std::vector<PresentationObject>& Map::hitTest(int x, int y, double pointerRadius)
{
    double scale = invertedScale() * m_drawable->pixelSize() / m_crs->globalMetersPerUnit();
    double size = pointerRadius * 2.0 * scale;
    Point center = screenToMap(Point((double)x, (double)y));
    auto area = Rectangle(center, size, size);
    // area = screenToMap(area);

    if (area.isUndefined())
    {
        BMM_DEBUG() << "Map::hitTest() Failed to get map position at screen position (" << x << ", " << y << ")!\n";
        m_presentationObjects.clear();
        return m_presentationObjects;
    }

    return hitTest(area);
}

const std::vector<PresentationObject>& Map::hitTest(const Rectangle& bounds)
{
    //FeatureQuery featureQuery = produceUpdateQuery();

    // TODO: this is to limit hittesting within the visible view area
    // auto boundsNew = Rectangle(
    //     std::max(featureQuery.area().xMin(), bounds.xMin()),
    //     std::max(featureQuery.area().yMin(), bounds.yMin()),
    //     std::min(featureQuery.area().xMax(), bounds.xMax()),
    //     std::min(featureQuery.area().yMax(), bounds.yMax())
    // );

    m_presentationObjects.clear();

    // Iterate in reverse such that the last rendered layer is hittested first
    for (auto iter = m_layers.rbegin(); iter!=m_layers.rend(); ++iter)
    {
        auto l = *iter;
        if (l->selectable())
        {
            l->hitTest(shared_from_this(), bounds, m_presentationObjects);
        }
    }

    return m_presentationObjects;
}

void Map::select(FeaturePtr feature, SelectMode mode)
{
    if(!feature)
        return;

    switch (mode)
    {
    case SelectMode::Replace:
        m_selectedFeatures.clear();
        break;

    case SelectMode::Add:
        break;

    default:
        std::cout << "Map::select() Unhandled select mode: " << (int)mode << "\n";
        break;
    }

    if (!isSelected(feature))
    {
        m_selectedFeatures.push_back(feature->id());

        // Testing restartVisualizationAnimation
        if (auto dataSet = DataSet::getDataSetById(feature->id().dataSetId()))
        {
            dataSet->restartVisualizationAnimation(feature, getTimeStampMs());
        }

        // // std::cout << "Selected feature, Id: " << "Id(" << feature->id().dataSetId() << ", " << feature->id().featureId() << ")\n";
        // std::cout << feature->prettyString();
    }
    auto ids = std::make_shared<IdCollection>();
    ids->addRange(m_selectedFeatures.begin(), m_selectedFeatures.end());
    events.onSelectionChanged.notify(*this, ids);
}

void Map::select(const PresentationObject& presentationObject)
{
    m_selectedPresentationObjects.push_back(presentationObject);
}

const std::vector<PresentationObject>& Map::selectedPresentationObjects()
{
    return m_selectedPresentationObjects;
}

void Map::deSelect(const Id& id)
{
    for (auto it = m_selectedFeatures.begin(); it!= m_selectedFeatures.end(); it++)
    {
        if(*it == id)
        {
            m_selectedFeatures.erase(it);
            return;
        }
    }
}

void Map::deSelect(FeaturePtr feature)
{
    deSelect(feature->id());
}

void Map::deSelectAll()
{
    m_selectedFeatures.clear();
    m_selectedPresentationObjects.clear();
}

bool Map::isSelected(const Id& id)
{
    for (auto id2 : m_selectedFeatures)
    {
        if (id == id2)
            return true;
    }

    return false;
}

bool Map::isSelected(FeaturePtr feature)
{
    return isSelected(feature->id());
}

void Map::hover(const Id& id)
{
    if (isHovered(id))
        return;
    m_hoveredFeatures.clear();
    if (id != Id(0,0))
    {
        // TESTING animated visualization
        if (!isSelected(id))
        {
            if (auto dataSet = DataSet::getDataSetById(id.dataSetId()))
            {
                dataSet->restartVisualizationAnimation(dataSet->getFeature(id), getTimeStampMs());
            }
        }
        m_hoveredFeatures.push_back(id);
    }
        
    auto notifyId = Id(0,0);
    if (!m_hoveredFeatures.empty())
    {
        notifyId = m_hoveredFeatures[0];
    }
    events.onHoverChanged.notify(*this, notifyId);
}

void Map::hover(FeaturePtr feature)
{
    Id id(0,0);
    if (feature)
    {
        id = feature->id();
    }

    // Testing visualization animation
    // if (!isHovered(id) && !isSelected(id))
    // {
    //     if (auto dataSet = DataSet::getDataSetById(id.dataSetId()))
    //     {
    //         dataSet->restartVisualizationAnimation(feature, getTimeStampMs());
    //         BMM_DEBUG() << "Hover restart (id: " << id.toString() << ")\n";
    //     }
    // }
    hover(id);
}

void Map::hover(const std::vector<Id>& ids)
{
    m_hoveredFeatures = ids;
}

void Map::hover(const std::vector<FeaturePtr>& features)
{
    m_hoveredFeatures.clear();
    for (auto f : features)
    {
        m_hoveredFeatures.push_back(f->id());
    }
}

bool Map::isHovered(const Id& id)
{
    for (auto id2 : m_hoveredFeatures)
    {
        if(id == id2)
            return true;
    }
    return false;
}

bool Map::isHovered(FeaturePtr feature)
{
    if (!feature)
        return m_hoveredFeatures.empty();
    return isHovered(feature->id());
}

DrawablePtr Map::drawable()
{
    // TODO: should be own m_drawable
    return m_drawable;
}

void Map::drawable(const DrawablePtr& drawable)
{
    bool sizeChanged = false;
    if (m_drawable)
    {
        if (drawable->width() != m_drawable->width() || drawable->height() != m_drawable->height())
        {
            sizeChanged = true;
        }
    }

    m_drawable = drawable;
    m_offscreenDrawable = drawable->createCompatibleOffscreenDrawable(m_drawable->width(), m_drawable->height());
    drawable->makeCurrent();

    if (sizeChanged)
        resize(drawable->width(), drawable->height()); 
}

void Map::resize(int width, int height)
{
    if (m_offscreenDrawable)
    {
        m_offscreenDrawable->makeCurrent();
        m_offscreenDrawable->resize(width, height);
    }
    
    m_drawable->makeCurrent();
    m_drawable->resize(width, height);
    m_camera->setViewPortSize(width, height);
    if (m_cameraController)
    {
        m_cameraController->onViewportSizeChanged(width, height);
    }
}

void Map::flushCache()
{
    if (m_offscreenDrawable)
    {
        m_offscreenDrawable->makeCurrent();
        m_offscreenDrawable->flushCache();
    }
    
    m_drawable->makeCurrent();
    m_drawable->flushCache();
    for (const auto& l : m_layers)
    {
        l->flushCache();
    }
    m_drawable->makeCurrent();
}

void Map::renderingEnabled(bool enabled)
{
    m_renderingEnabled = enabled;

    std::function<void(const LayerPtr&, bool)> enableLayerRendering;
    enableLayerRendering = [&enableLayerRendering](const LayerPtr& layer, bool enabled)
    {
        layer->renderingEnabled(enabled);
        if (auto layerSet = std::dynamic_pointer_cast<LayerSet>(layer))
        {
            // LayerSet needs to be handled differently since it does not have its own rendering, but relies on its children
            for (const auto& l : layerSet->layers())
            {
                enableLayerRendering(l, enabled);
            }
        }
    };

    for (const auto& l : m_layers)
    {
        enableLayerRendering(l, enabled);
    }
}

void Map::updateUpdateAttributes(int64_t timeStampMs)
{
    // FIXME: this is a big issue for animations and time stamps.
    // The int truncation may completely fuck upp stuff
    m_updateAttributes.set(UpdateAttributeKeys::UpdateTimeMs, (int)timeStampMs);

    m_updateAttributes.set(UpdateAttributeKeys::UpdateViewScale, scale());
    m_updateAttributes.set(UpdateAttributeKeys::QuickUpdate, false);
    m_updateAttributes.set(UpdateAttributeKeys::SelectionUpdate, false);
    m_updateAttributes.set(UpdateAttributeKeys::HoverUpdate, false);
    if (!m_selectedFeatures.empty())
        m_updateAttributes.set(UpdateAttributeKeys::SelectionUpdate, true);
    if (!m_hoveredFeatures.empty())
    {
        m_updateAttributes.set(UpdateAttributeKeys::HoverUpdate, true);
    }

    // For others in the operator chain to set (or elsewhere).
    // These are reset in the beginning of each update
    m_updateAttributes.set(UpdateAttributeKeys::UpdateRequired, false);

}

void Map::beforeRender()
{
    constexpr double maxRatio = 10000.0;
    
    double near = std::numeric_limits<double>::max();
    double far = 0.0f;
    
    double w = m_drawable->width();
    double h = m_drawable->height();
    auto worldBounds = m_crs->bounds();
    // TODO: becomes weird when tilting alot
    // TODO: add some sort of "nearestHorizon" method to surfaceModel?
    std::vector<Point> visibleRegionWorld = m_crs->bounds().corners();
    visibleRegionWorld.push_back(screenToMap(0,0));
    visibleRegionWorld.push_back(screenToMap(w,0));
    visibleRegionWorld.push_back(screenToMap(w,h));
    visibleRegionWorld.push_back(screenToMap(0,h));
    
    bool ok = false;
    for (auto& point : visibleRegionWorld)
    {
        if (point.isUndefined())
            continue;
        double d = -m_camera->worldToView(point).z();
        if (d <= 0.0f)
            continue; // behind camera, ignore

        near = std::min(near, d);
        far = std::max(far, d);
        ok = true;
    }
    if (!ok) BMM_DEBUG() << "COULDNT CALCULATE FRUSTIM!!!\n"; // THIS IS REALLY BAD IF IT HAPPENS
    near *= 0.5; 
    far *= 1.5;

    double precision = far/near;
    
    if (precision > maxRatio)
    {
        far = near*maxRatio;    // Favor near
        // near = far/maxRatio; // Favor far
    }

    // BMM_DEBUG() << "NEAR: " << near << "\n";
    // BMM_DEBUG() << "FAR: " << far << "\n";
    // BMM_DEBUG() << "PRECISION: " << far/near << "\n";

    m_camera->setFrustum(near, far);

    m_drawable->makeCurrent();
    m_drawable->clearBuffer();

    if (m_offscreenDrawable)
    {
        m_offscreenDrawable->makeCurrent();
        m_offscreenDrawable->clearBuffer();
        m_offscreenDrawable->resize(m_drawable->width(), m_drawable->height());
    }

    setDrawableFromCamera(m_camera);

    // Give the offscreen drawable a flat, straight-down orthographic camera instead of the live
    // (possibly tilted/rotated) m_camera -- same pattern TileLayer::renderTile() uses for its
    // per-tile offscreen renders. Otherwise the offscreen texture already bakes the live camera's
    // tilt/rotation into its pixels once, and then drawing the elevation mesh through that same
    // tilted camera again applies the tilt a second time -- that's the "offset and rotated" bug.
    if (m_offscreenDrawable)
    {
        double unitsPerPixel = Drawable::pixelSize() / crs()->globalMetersPerUnit() / scale();

        auto orthoProj = OrthographicCameraProjection((int)w, (int)h, -1.0, 1.0, unitsPerPixel);
        m_offscreenDrawable->setProjectionMatrix(orthoProj.projectionMatrix());
        m_offscreenDrawable->setViewMatrix(glm::dmat4(1.0));
        m_offscreenDrawable->setRenderOrigin(Point(crs()->bounds().center().x(), crs()->bounds().center().y(), 0.0));
    }
    
    // // Temporary: force a fixed, straight-down 2D view for this offscreen render, independent of
    // // whatever the live camera is currently doing (tilt/rotation/orbit). The orthographic projection
    // // above already encodes the tile's scale (unitsPerPixel); an identity view matrix means no
    // // rotation/tilt gets applied on top of it, so panning/tilting the live camera shouldn't change
    // // how this layer's own content looks once composited back in via blitTo.
    // offscreenDrawable->setProjectionMatrix(proj);
    // offscreenDrawable->setViewMatrix(glm::dmat4(1.0));
    // offscreenDrawable->setRenderOrigin(Point(area.center().x(), area.center().y(), 0.0)); // must come after setViewMatrix — it resets renderOrigin to (0,0,0) as a side effect

}

void Map::afterRender()
{
    // m_drawable->swapBuffers();
}

void Map::drawTestElevationMesh()
{
    auto offd = std::dynamic_pointer_cast<OpenGLDrawable>(m_offscreenDrawable);
    if (!offd) return;

    static MeshPtr m_testMesh;
    if (true)
    {
        int gridSize = 64;
        
        auto heights = Mesh::generateProceduralHeights(gridSize, gridSize, 500000.0 / crs()->globalMetersPerUnit());
        Rectangle bounds = crs()->bounds();
        m_testMesh = std::make_shared<Mesh>(gridSize, gridSize, bounds.xMin(), bounds.yMin(), bounds.width(), bounds.height(), heights, offd->getFboTexture());
    }
    m_testMesh->setTexture(offd->getFboTexture());
    
    auto d = m_drawable;
    auto c = camera();

    // setDrawableFromCamera(camera());
    d->setProjectionMatrix(c->projectionMatrix());
    d->setViewMatrix(c->viewMatrix());

    d->makeCurrent();
    glm::mat4 viewProj = glm::mat4(d->getProjectionMatrix() * d->getViewMatrix());
    m_testMesh->draw(viewProj);

    //     auto offd = std::dynamic_pointer_cast<OpenGLDrawable>(m_offscreenDrawable);
    // if (!offd) return;

    // auto c = camera();
    // auto renderOrigin = c->translation(); // same render origin OpenGLDrawable::createPoint() subtracts for every layer

    // static MeshPtr m_testMesh;
    // if (true)
    // {
    //     int gridSize = 64;

    //     auto heights = Mesh::generateProceduralHeights(gridSize, gridSize, 500000.0 / crs()->globalMetersPerUnit());
    //     Rectangle bounds = crs()->bounds();
    //     // Build vertices relative to the current render origin instead of raw absolute world
    //     // coordinates: Web Mercator coordinates run into the tens of millions of meters, and
    //     // stuffing that directly into float32 vertices/matrices (as this used to do, combined
    //     // with the full translation-inclusive c->viewMatrix() below) loses enough precision to
    //     // visibly jitter/shear as the camera moves -- looks like the texture "rotates" relative
    //     // to the mesh when panning/zooming. Every other draw path in this codebase avoids this
    //     // by subtracting the render origin in double precision on the CPU first; do the same here.
    //     double originX = bounds.xMin() - renderOrigin.x();
    //     double originY = bounds.yMin() - renderOrigin.y();
    //     m_testMesh = std::make_shared<Mesh>(gridSize, gridSize, originX, originY, bounds.width(), bounds.height(), heights, offd->getFboTexture());
    // }
    // m_testMesh->setTexture(offd->getFboTexture());

    // auto d = m_drawable;

    // // Rotation-only, matching setDrawableFromCamera()/createPoint()'s convention -- translation
    // // is already baked into the mesh's vertices above via renderOrigin, so applying it again here
    // // (as the old c->viewMatrix() did) would double-count it.
    // d->setProjectionMatrix(c->projectionMatrix());
    // d->setViewMatrix(glm::transpose(c->rotationMatrix()));

    // d->makeCurrent();
    // glm::mat4 viewProj = glm::mat4(d->getProjectionMatrix() * d->getViewMatrix());
    // m_testMesh->draw(viewProj);
}

void Map::drawDebugInfo(int elapsedMs)
{
    ScreenPos mousePos;
    if (m_mapControl)
        m_mapControl->getMousePos(mousePos);
    auto mouseMapPos = screenToMap(mousePos.x, mousePos.y);
    auto mouseLngLat = mapToLngLat(mouseMapPos);
    // auto centerLngLat = mapToLngLat(center());
    auto center = m_camera->translation();
    auto screenPos = mapToScreen(mouseMapPos).round();
    auto screenError = Point(mousePos.x-(int)screenPos.x(), mousePos.y-(int)screenPos.y());
    std::string info = "------ Debug -------\n";
    info += "Center: " + std::to_string(center.x()) + ", " + std::to_string(center.y());
    // info += "\nCenter LngLat: " + std::to_string(centerLngLat.x()) + ", " + std::to_string(centerLngLat.y());
    info += "\nScale: " + std::to_string(scale());
    info += "\nScale inv: " + std::to_string(invertedScale());
    info += "\nScale (crs)): " + std::to_string(scale());

    if (std::abs(screenError.x()) > 0 || std::abs(screenError.y()) > 0)
    {
        // Only display this when error occurs
        info += "\nMouse: " + std::to_string(mousePos.x) + ", " + std::to_string(mousePos.y);
        info += "\nMouseToMap: " + std::to_string(mouseMapPos.x()) + ", " + std::to_string(mouseMapPos.y());
        info += "\nMouseToMapToMouse: " + std::to_string((int)screenPos.x()) + ", " + std::to_string((int)screenPos.y());
        info += "\nScreen error: " + std::to_string(mousePos.x-(int)screenPos.x()) + ", " + std::to_string(mousePos.y-(int)screenPos.y());
        info += "\nMouseLngLat: " + std::to_string(mouseLngLat.x()) + ", " + std::to_string(mouseLngLat.y());
    }

    info += "\nUpdate time: " + std::to_string(elapsedMs);
    info += "\nFPS: " + std::to_string(1000.0/elapsedMs);
    info += "\nPresentationObjects: " + std::to_string(m_presentationObjects.size());

    info += "\n";
    // auto presentationObjects = hitTest(mousePos.x, mousePos.y, 10.0);
    // for (auto& p : presentationObjects)
    // {
    //     info += "\t";
    //     info += "Geometry: " + typeToString(p.feature()->geometryType());
    //     info += ", Source: " + typeToString(p.sourceFeature()->geometryType());
    //     info += ", Node: " + std::to_string(p.nodeIndex());
    //     info += "\n";
    // }


    int fontSize = 16;
    m_drawable->drawText(0, 0, info.c_str(), Color(0, 0, 0), fontSize);

    BMM_DEBUG() << info << "\n\n";
}

// const Point Map::screenToLngLat(const Point& screenPoint)
// {
//     return mapToLngLat(screenToMap(screenPoint));
// }

const Point Map::mapToLngLat(const Point& mapPoint, bool normalize) const
{
    static CrsPtr wgs84 = Crs::wgs84LngLat();
    // FIXME: I dont know if normalize is needed, it might be
    
    return crs()->projectTo(wgs84, mapPoint);
}

const Point BlueMarble::Map::lngLatToMap(const Point& lngLat) const
{
    static CrsPtr wgs84 = Crs::wgs84LngLat();
    
    return wgs84->projectTo(m_crs, lngLat);
}


void Map::setDrawableFromCamera(const CameraPtr& camera)
{
    m_drawable->setProjectionMatrix(camera->projectionMatrix());
    //m_drawable->setViewMatrix(camera->viewMatrix());
    
    m_drawable->setViewMatrix(glm::transpose(camera->rotationMatrix()));
    auto o = camera->translation();
    m_drawable->setRenderOrigin(o);
}
