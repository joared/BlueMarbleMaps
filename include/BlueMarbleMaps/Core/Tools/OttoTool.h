#ifndef OTTOTOOL
#define OTTOTOOL

#include "BlueMarbleMaps/Core/Tools/Tool.h"
#include "BlueMarbleMaps/Core/Camera/OttoCameraController.h"

namespace BlueMarble
{
    class OttoTool : public Tool
    {
        public:
            OttoTool() {}
            bool isActive() override final { return false; };
            virtual void onConnected(const MapControlPtr& control, const MapPtr& map) override final
            {
                m_map = map;
                m_map->setCameraController(&m_cameraController);

            }
            virtual void onDisconnected() override final
            {
                m_map->setCameraController(nullptr);
                m_map = nullptr;
            }


            bool OnDrag(const DragEvent& event)
            {
                if (event.mouseButton == MouseButtonLeft)
                {
                    auto delta = m_map->screenToMap(Point{event.lastPos.x, event.lastPos.y}) - m_map->screenToMap(Point{event.pos.x, event.pos.y});
                    m_cameraController.panBy(delta.x(), delta.y());
                    m_map->update();
                }
                else if (event.mouseButton == MouseButtonMiddle)
                {
                    switch (event.phase)
                    {
                    case InteractionEvent::Phase::Started:
                        m_cameraController.setPivot(m_map->screenToMap(double(event.pos.x), double(event.pos.y)));
                        return true;
                    case InteractionEvent::Phase::Completed:
                        m_cameraController.setPivot(m_cameraController.translation());
                        return true;
                    }
                    constexpr double factor = 0.001;
                    double deltaX = event.lastPos.x - event.pos.x;
                    double deltaY = event.lastPos.y - event.pos.y;
                    m_cameraController.orbitBy(deltaX*factor, deltaY*factor);
                    m_map->update();
                }
                else if (event.mouseButton == MouseButtonRight)
                {
                    switch (event.phase)
                    {
                    case InteractionEvent::Phase::Started:
                        return true;
                    case InteractionEvent::Phase::Completed:
                        return true;
                    }

                    auto rayCurr = m_map->screenToViewRay(event.pos.x, event.pos.y);
                    auto rayLast = m_map->screenToViewRay(event.lastPos.x, event.lastPos.y);
                     
                    // TODO: For orthographic wee need to offset instead
                    // Point delta = rayCurr.origin - rayLast.origin;

                    auto xzCurr = Point(rayCurr.direction.x(), 0.0, rayCurr.direction.z()).norm3D();
                    auto xzLast = Point(rayLast.direction.x(), 0.0, rayLast.direction.z()).norm3D();
                    auto yzCurr = Point(0.0, rayCurr.direction.y(), rayCurr.direction.z()).norm3D();
                    auto yzLast = Point(0.0, rayLast.direction.y(), rayLast.direction.z()).norm3D();
                    
                    double yaw = std::atan2(xzCurr.crossProduct(xzLast).y(), xzCurr.dotProduct(xzLast));
                    double pitch = std::atan2(yzCurr.crossProduct(yzLast).x(), yzCurr.dotProduct(yzLast));
                    
                    m_cameraController.yawBy(RAD_TO_DEG*yaw);
                    m_cameraController.pitchBy(RAD_TO_DEG*pitch);
                    
                    
                    m_map->update();
                }
            }

            bool OnMouseWheel(const MouseWheelEvent& event) override final
            {
                constexpr double wheelDelta = 5;
                double scale = 1.0 + abs(event.delta)/wheelDelta;
                double zoomFactor = event.delta > 0 ? scale : 1.0/scale;
                 m_cameraController.zoomBy(event.delta);
                // double z = -event.delta * 1.0 / m_map->scale();
                //m_cameraController.panBy(0,0,z);
               
                m_map->update();
            }

        protected:
            MapPtr m_map;
            OttoCameraController m_cameraController;
    };
};

#endif /* OTTOTOOL */
