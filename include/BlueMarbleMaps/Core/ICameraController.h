#ifndef BLUEMARBLE_ICAMERACONTROLLER
#define BLUEMARBLE_ICAMERACONTROLLER

#include "BlueMarbleMaps/Core/Camera.h"
#include <memory>

namespace BlueMarble
{
    
class ICameraController
{
    public:
        enum class ControllerStatus : int
        {
            Idle        = 0,
            Updated     = BIT(0),
            NeedsUpdate = BIT(1)
        };

        virtual ~ICameraController() = default;

        virtual CameraPtr onActivated(const CameraPtr& currentCamera, const Rectangle& worldBounds) = 0;
        virtual void onDectivated() = 0;
        virtual ControllerStatus updateCamera(const CameraPtr& camera, int64_t deltaMs)= 0;
};

inline ICameraController::ControllerStatus operator|(ICameraController::ControllerStatus a, ICameraController::ControllerStatus b)
{
    return static_cast<ICameraController::ControllerStatus>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr bool hasFlag(ICameraController::ControllerStatus value, ICameraController::ControllerStatus flag)
{
    using T = std::underlying_type_t<ICameraController::ControllerStatus>;
    return (static_cast<T>(value) & static_cast<T>(flag)) != 0;
}


}

#endif /* BLUEMARBLE_ICAMERACONTROLLER */
