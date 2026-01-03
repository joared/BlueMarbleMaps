#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/MapConstraints.h"
#include "BlueMarbleMaps/Utility/Utils.h"
#include "BlueMarbleMaps/Core/DataSets/DataSet.h"
#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/SoftwareDrawable.h"
#include "BlueMarbleMaps/Logging/Logging.h"

#include "BlueMarbleMaps/Core/OpenGLDrawable.h"

#include <cmath>
#include <iostream>
#include <vector>
#include <set>


using namespace BlueMarble;

constexpr double xPixLen = 0.02222222222222;
constexpr double yPixLen = -0.02222222222222;
constexpr double xTopLeft = -179.98888888888889;
constexpr double yTopLeft = 89.98888888888889;

// constexpr double xPixLen = 0.03333333333333;
// constexpr double yPixLen = -0.03333333333333;
// constexpr double xTopLeft = -179.98333333333333;
// constexpr double yTopLeft = 89.98333333333333;

Map::Map()
    : m_center()
    , m_scale(1.0)
    , m_rotation(0.0)
    , m_tilt(0.0)
    , m_constraints(5000.0, 0.0001, Rectangle(0, 0, 100000, 100000))
    , m_crs(Crs::wgs84LngLat())
    , m_surfaceModel(std::make_shared<PlaneSurfaceModel>(Point{0,0,0}, Point{0,0,1}, m_crs->bounds()))
    , m_updateRequired(true)
    , m_updateEnabled(true)
    , m_animation(nullptr)
    , m_animationStartTimeStamp(0)
    , m_updateAttributes()
    , m_centerChanged(false)
    , m_scaleChanged(false)
    , m_rotationChanged(false)
    , m_cameraController(nullptr)
    , m_lastUpdateTimeStamp(-1)
    , m_presentationObjects()
    , m_selectedFeatures()
    , m_hoveredFeatures()
    , m_showDebugInfo(true)
    , m_isUpdating(false)
    , m_renderingEnabled(true)
{
    m_center = lngLatToMap(Point(30, 36));
    m_drawable = std::make_shared<SoftwareBitmapDrawable>(500, 500, 4);
    setCamera(); // TODO remove

    m_presentationObjects.reserve(1000000); // Reserve a good amount for efficiency
    resetUpdateFlags();
    m_constraints.bounds().scale(3.0);

    m_lastUpdateTimeStamp = getTimeStampMs();
    updateUpdateAttributes(m_lastUpdateTimeStamp);

    setCamera(); // TODO remove
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

    if (!m_updateEnabled || (!forceUpdate && !m_animation && !m_updateRequired))
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
        if (hasFlag(status, ICameraController::ControllerStatus::Updated))
        {
            events.onAreaChanged.notify(*this);
        }
        if (hasFlag(status, ICameraController::ControllerStatus::NeedsUpdate))
        {
            m_updateRequired = true;
        }
        else
        {
            // BMM_DEBUG() << "To camera update needed\n";
        }
    }
    // FIXME: should animations be before onUpdating?
    stopAnimation(); // TODO: remove, use camera controller instead
    if(m_animation && m_animation->update(timeStampMs - m_animationStartTimeStamp))
    {
        // Animation finished
        stopAnimation();
    }

    events.onUpdating.notify(*this);

    // Constrain map pose
    m_constraints.constrainMap(*this);

    // Rendering
    beforeRender();
    renderLayers(); // Let layers do their work

    // FIXME camera stuff:
    if (auto glDrawable = std::dynamic_pointer_cast<OpenGLDrawable>(m_drawable))
    {
        auto proj = ScreenCameraProjection(m_drawable->width(), m_drawable->height());
        glDrawable->setProjectionMatrix(proj.projectionMatrix());
    }

    // Each onCustomDraw notification should have the transform set to "screen".
    // Since handlers are allowed to modify the transform, we need to make sure to reset it
    // each time a handler is called.
    auto preNotifyAction = [this]()
    {
        // TODO: set both projection and transform/view matrix in prenotify action
        //m_drawable->setTransform(Transform::screenTransform(m_drawable->width(), m_drawable->height()));
        m_drawable->setTransform(Transform());
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

    afterRender();

    events.onUpdated.notify(*this);

    m_updateRequired |= m_updateAttributes.get<bool>(UpdateAttributeKeys::UpdateRequired); // Someone in the operator chain needs more updates (e.g. Visualization evaluations)

    resetUpdateFlags();

    m_isUpdating = false;

    bool updateRequired = m_updateRequired || m_animation != nullptr;
    if (!updateRequired)
    {
        events.onIdle.notify(*this);
    }


    return updateRequired;
}

void Map::renderLayer(const LayerPtr& layer, const FeatureQuery& featureQuery)
{
    layer->prepare(crs(), featureQuery);
    layer->update(shared_from_this());
}

void Map::renderLayers()
{
    m_presentationObjects.clear(); // Clear presentation objects, layers will add new

    FeatureQuery featureQuery = std::move(produceUpdateQuery());

    for (const auto& l : m_layers)
    {
        // TODO add "ViewInfo" as parameter to Layer::update()?
        //l->update(shared_from_this(), getCrs(), featureQuery);
        renderLayer(l, featureQuery);
    }

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

FeatureQuery BlueMarble::Map::produceUpdateQuery()
{
    FeatureQuery featureQuery;

    int w = m_drawable->width();
    int h = m_drawable->height();
    auto screenArea = Rectangle(0,0,w,h);
    screenArea.scale(0.75); // TODO: this scaling is for debugging querying, remove

    auto updateArea = screenToMap(screenArea);
    featureQuery.area(updateArea);
    // Map to camera
    auto centerMap = screenToMap(screenCenter());
    auto centerCam = m_camera->worldToView(centerMap);
    double zCam = centerCam.z();
    // double unitsPerPixel = m_camera->projection()->unitsPerPixelAtDistanceNumerical(std::abs(zCam));
    double unitsPerPixel = m_camera->unitsPerPixelAtDistance(std::abs(zCam));
    double queryScale = 1.0 / unitsPerPixel * m_drawable->pixelSize() / m_crs->globalMeterScale();
    featureQuery.scale(queryScale);

    featureQuery.quickUpdate(quickUpdateEnabled());
    featureQuery.updateAttributes(&updateAttributes());

    return featureQuery;
}

void Map::center(const Point &center)
{
    m_updateRequired = true;
    m_centerChanged = true;
    m_center = center;
    
    setCamera();
}

void Map::scale(double scale)
{
    m_updateRequired = true;
    m_scaleChanged = true;
    m_scale = scale*m_crs->globalMeterScale() / m_drawable->pixelSize();

    setCamera();
}

double Map::scale() const
{
    auto centerMap = screenToMap(screenCenter());
    auto centerCam = m_camera->worldToView(centerMap);
    double zCam = centerCam.z();
    // double unitsPerPixel = m_camera->projection()->unitsPerPixelAtDistanceNumerical(std::abs(zCam));
    double unitsPerPixel = m_camera->unitsPerPixelAtDistance(std::abs(zCam));
    double aprroximateScale = 1.0 / unitsPerPixel * m_drawable->pixelSize() / m_crs->globalMeterScale();

    return aprroximateScale;
    // return m_scale * m_drawable->pixelSize() / m_crs->globalMeterScale();
}

double Map::invertedScale() const
{
    return 1.0 / scale();
}

void Map::invertedScale(double invScale)
{
    scale(1.0 / invScale);
}

double Map::rotation() const
{
    return m_rotation;
}

void Map::rotation(double rotation)
{
    m_updateRequired = true;
    m_rotationChanged = true;
    m_rotation = rotation;

    setCamera();
}

double Map::tilt() const
{
    return m_tilt;
}

void Map::tilt(double tilt)
{
    m_updateRequired = true;
    m_tilt = tilt;

    setCamera();
}

double Map::width() const
{
    return m_drawable->width() / m_scale;
}

void Map::width(double newWidth)
{
    m_scale = m_scale * width() / newWidth;

    setCamera();
}

double Map::height() const
{
    return m_drawable->height() / m_scale;
}

Rectangle Map::area() const
{
    return Rectangle
    (
        center().x() - (width()/*-1*/)*0.5,     // TODO: minus one when pixel coordinates? If so, geCrop needs to be adjusted in render()
        center().y() - (height()/*-1*/)*0.5,    // TODO: minus one when pixel coordinates? If so, geCrop needs to be adjusted in render()
        center().x() + (width()/*-1*/)*0.5,     // TODO: minus one when pixel coordinates? If so, geCrop needs to be adjusted in render()
        center().y() + (height()/*-1*/)*0.5     // TODO: minus one when pixel coordinates? If so, geCrop needs to be adjusted in render()
    );
}

void Map::crs(const CrsPtr& newCrs)
{
    auto lngLatCrs = Crs::wgs84LngLat();

    auto oldCrs = crs();
    auto centerLngLat = crs()->projectTo(lngLatCrs, center());
    double oldScale = scale();

    m_crs = newCrs;

    center(lngLatCrs->projectTo(newCrs, centerLngLat));
    scale(oldScale);

    if (m_cameraController)
    {
        m_cameraController->onActivated(m_camera, m_surfaceModel);
    }

    flushCache(); // We need to flush layer caches since the crs has changed
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
        m_camera = m_cameraController->onActivated(m_camera, m_surfaceModel);
    }
}

Point Map::pixelToScreen(const Point& pixel) const
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
        BMM_DEBUG() << "Map::screenToMap() No map intersection!\n";
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
                                         heightMeters/m_crs->globalMeterScale(),
                                         surfacePoint, 
                                         surfaceNormal))
    {
        BMM_DEBUG() << "Map::screenToMapAtHeight() No map intersection!\n";
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
    ndcX = float(x * 2.0 / float(m_drawable->width()) - 1.0);
    ndcY = float(1.0 - y * 2.0 / float(m_drawable->height()));
}

void Map::ndcToScreen(double ndcx, double ndcy, double &x, double &y) const
{
    // TODO: "ndc" should be owned by the drawable
    x = (ndcx + 1.0f) * 0.5f * m_drawable->width();
    y = (1.0f- ndcy) * 0.5f * m_drawable->height();
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

void Map::startAnimation(AnimationPtr animation)
{
    std::cout << "Started animation\n";
    if (m_animation)
        stopAnimation();
    m_animation = animation;
    m_animationStartTimeStamp = getTimeStampMs();
    quickUpdateEnabled(true); // For now, consider animations as quick updates
}

void Map::stopAnimation()
{
    if (m_animation)
    {
        std::cout << "Stoppped animation\n";
    }

    m_animation = nullptr;
    quickUpdateEnabled(false);
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

// void Map::getFeatures(const Attributes& attributes, std::vector<FeaturePtr>& features)
// {
//     for (auto l : m_layers)
//     {
//         l->onGetFeaturesRequest(attributes, features);
//     }
// }

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
    auto area = Rectangle({(double)x,(double)y}, pointerRadius, pointerRadius);
    area = screenToMap(area);

    return hitTest(area);
}

const std::vector<PresentationObject>& Map::hitTest(const Rectangle& bounds)
{
    FeatureQuery featureQuery = produceUpdateQuery();

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

void Map::drawable(const DrawablePtr &drawable)
{
    m_drawable = drawable;
    resize(drawable->width(), drawable->height()); 
}

void Map::resize(int width, int height)
{
    m_drawable->resize(width, height);
    m_camera->setViewPort(width, height);
}

void Map::flushCache()
{
    for (const auto& l : m_layers)
    {
        l->flushCache();
    }
}

void Map::renderingEnabled(bool enabled)
{
    m_renderingEnabled = enabled;
    for (const auto& l : m_layers)
    {
        l->renderingEnabled(enabled);
    }
}

void Map::setCamera()
{
    // Orthographic 2.5D
    auto c = m_center;
    double rot = m_rotation;
    double scaleFactor = m_scale;
    double fov = 45.0;
    double dHeight = m_drawable->height();
    double tilt = m_tilt;

    
    
    // Adjusted in update()
    float near = 0.1f;
    float far = 1000000.0f;

    if (!m_camera)
        m_camera = Camera::perspectiveCamera(m_drawable->width(), m_drawable->height(), near, far, float(fov));
    glm::mat4 cam = glm::mat4(1.0f);
    cam = glm::translate(cam, glm::vec3(
        c.x(),
        c.y(),
        0.0f
    ));
    cam = glm::rotate(cam, (float)glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
    cam = glm::rotate(cam, float(glm::radians(m_tilt)), glm::vec3(1.0f, 0.0f, 0.0f));
    cam = glm::translate(cam, glm::vec3(
        0.0f,
        0.0f,
        float(dHeight/(2.0*scaleFactor*std::tan(glm::radians(fov) * 0.5)))
    ));
    

    // Orthographic
    // m_camera = Camera::orthoGraphicCamera(m_drawable->width(), m_drawable->height(), -100000000.0f, 10000000.0f, 1.0f);

    // glm::mat4 cam = glm::mat4(1.0f);
    // cam = glm::translate(cam, glm::vec3(
    //     m_center.x(),
    //     m_center.y(),
    //     1.0
    // ));
    // cam = glm::rotate(cam, float(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    // cam = glm::scale(cam, glm::vec3(float(1.0/m_scale), float(1.0/m_scale), 1.0f));
    
    //m_camera->setTransform(cam);
}

void Map::updateUpdateAttributes(int64_t timeStampMs)
{
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
    // TODO: near and far plane might need to be adjusted
    // during cmaera manipulation. Maybe the camera controller should
    // int w = m_drawable->width();
    // int h = m_drawable->height();
    // std::vector<Point> visibleRegionWorld;
    // visibleRegionWorld.push_back(screenToMap(0,0));
    // visibleRegionWorld.push_back(screenToMap(0,w-1));
    // visibleRegionWorld.push_back(screenToMap(h-1,w-1));
    // visibleRegionWorld.push_back(screenToMap(h-1,0));
    // visibleRegionWorld.push_back(screenToMap(screenCenter()));
    float near = std::numeric_limits<float>::max();
    float far = 0.0f;
    auto visibleRegionWorld = m_crs->bounds().corners();

    for (auto& point : visibleRegionWorld)
    {
        float d = -m_camera->worldToView(point).z();
        if (d <= 0.0f)
            continue; // behind camera, ignore

        near = std::min(near, d);
        far = std::max(far, d);
    }
    near *= 0.001; 
    far*=2.0;

    float precision = far/near;
    constexpr float maxRatio = 10000.0;
    
    near = far/maxRatio;

    m_camera->setFrustum(near, far);

    if (auto glDrawable = std::dynamic_pointer_cast<OpenGLDrawable>(m_drawable))
    {
        m_drawable->clearBuffer();

        glDrawable->setProjectionMatrix(m_camera->projectionMatrix());
        glDrawable->setViewMatrix(m_camera->viewMatrix());
    }
    else
    {
        m_drawable->clearBuffer();
        auto sc = screenCenter();
        sc = Point(sc.x(), sc.y());

        auto center = Point(
            m_center.x(),// - (float)width()  / (2.0f * (float)m_scale),
            m_center.y()// - (float)height() / (2.0f * (float)m_scale)
        );
        m_drawable->setTransform(Transform(center, m_scale, m_scale, m_rotation));
    }
}

void Map::afterRender()
{
    m_drawable->swapBuffers();
}

void Map::resetUpdateFlags()
{
    m_centerChanged = false;
    m_scaleChanged = false;
    m_rotationChanged = false;
}

void Map::startInitialAnimation()
{
    auto animation = Animation::Create(*this,
                                       center(),
                                       center(),
                                       scale()*10.0,
                                       scale(),
                                       1000,
                                       false);
    startAnimation(animation);
}

void Map::drawDebugInfo(int elapsedMs)
{
    ScreenPos mousePos;
    if (m_mapControl)
        m_mapControl->getMousePos(mousePos);
    auto mouseMapPos = screenToMap(mousePos.x, mousePos.y);
    auto mouseLngLat = mapToLngLat(mouseMapPos);
    auto centerLngLat = mapToLngLat(center());
    auto screenPos = mapToScreen(mouseMapPos).round();
    auto screenError = Point(mousePos.x-(int)screenPos.x(), mousePos.y-(int)screenPos.y());
    std::string info = "------ Debug -------\n";
    info += "Center: " + std::to_string(m_center.x()) + ", " + std::to_string(m_center.y());
    info += "\nCenter LngLat: " + std::to_string(centerLngLat.x()) + ", " + std::to_string(centerLngLat.y());
    info += "\nScale: " + std::to_string(scale());
    info += "\nScale inv: " + std::to_string(invertedScale());
    info += "\nScale (crs)): " + std::to_string(m_scale);

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

// TODO: remove this
void Map::setDrawableFromCamera(const CameraPtr& camera)
{
    if (auto glDrawable = std::dynamic_pointer_cast<OpenGLDrawable>(m_drawable))
    {
        glDrawable->setProjectionMatrix(camera->projectionMatrix());
        glDrawable->setViewMatrix(camera->viewMatrix());
    }
    else
    {
        throw std::runtime_error("NOOOOT GUUUD!");
    }
}

const Point &Map::center() const
{
    return m_center;
}