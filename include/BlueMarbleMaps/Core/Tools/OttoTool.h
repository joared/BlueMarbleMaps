#ifndef BLUEMARBLE_OTTOTOOL
#define BLUEMARBLE_OTTOTOOL

#include "BlueMarbleMaps/Core/Tools/Tool.h"
#include "BlueMarbleMaps/Core/Camera/OttoCameraController.h"

namespace BlueMarble
{
    struct ActionBinding
    {
        // std::string actionName;
        EventType type;
        InteractionEvent::Phase phase;
        std::function<void(const InteractionEvent&)> callback;

    };

    class InputMapper : public EventHandler
    {
        public:
            InputMapper() {}

            void bindAction(EventType type, InteractionEvent::Phase phase, std::function<void(const InteractionEvent&)> callback)
            {
                // Wrap type-specific callback into generic Event callback
                auto wrapper = [cb = std::move(callback)](const InteractionEvent& e)
                {
                    cb(static_cast<const InteractionEvent&>(e));
                };

                m_bindings[typeid(EventType).hash_code()].push_back(ActionBinding{type, phase, wrapper});
            }
        private:
            std::unordered_map<size_t, std::vector<ActionBinding>> m_bindings;
    };

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
                auto delta = m_map->screenToMap(Point{event.lastPos.x, event.lastPos.y}) - m_map->screenToMap(Point{event.pos.x, event.pos.y});
                m_cameraController.panBy(delta.x(), delta.y());
                m_map->update();
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

#endif /* BLUEMARBLE_OTTOTOOL */
