#ifndef BLUEMARBLE_ICAMERACONTROLLER
#define BLUEMARBLE_ICAMERACONTROLLER

#include "BlueMarbleMaps/Core/Camera/Camera.h"
#include "BlueMarbleMaps/CoordinateSystem/SurfaceModel.h"
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

        virtual CameraPtr onActivated(const CameraPtr& currentCamera, const SurfaceModelPtr& surfaceModel) = 0;
        virtual void onDeactivated() = 0;
        // Called each update. Used to determine if updateCamera should be called.
        // The controller could determine this itself, however, this was added because the map needs to be able to force an update
        // when projection parameters have changed (such as at resizing).
        virtual bool needsUpdate() const = 0;
        // The controller must always update the camera when this method is called since the projection
        // is partially controller by the Map. The map will call this when needed
        virtual ControllerStatus updateCamera(const CameraPtr& camera, int64_t deltaMs) = 0;
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
