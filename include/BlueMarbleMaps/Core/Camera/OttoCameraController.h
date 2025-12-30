#ifndef BLUEMARBLE_OTTOCAMERACONTROLLER
#define BLUEMARBLE_OTTOCAMERACONTROLLER

#include "BlueMarbleMaps/Core/Camera/ICameraController.h"

namespace BlueMarble
{
    class OttoCameraController : public ICameraController
    {
        public:
            OttoCameraController()
            {}

            CameraPtr onActivated(const CameraPtr& currentCamera, const Rectangle& worldBounds) override final
            {
                m_camera = currentCamera;

                return currentCamera;
            }

            void onDeactivated() override final
            {
                m_camera = nullptr;
            }

            virtual ControllerStatus updateCamera(const CameraPtr& camera, int64_t deltaMs)= 0;
        private:
            CameraPtr m_camera;
    };
}

#endif /* BLUEMARBLE_OTTOCAMERACONTROLLER */
