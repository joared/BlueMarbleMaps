#include "BlueMarbleMaps/Core/Camera.h"

double CameraProjection::unitsPerPixelAtDistanceNumerical(double zDistCamera) const
{
    // TODO
    // int w = m_projection->width();
    // auto p1 = rayDirectionCamera(0,0);
    // auto p2 = rayDirectionCamera(w,0);
    // p1 = p1 * (zDistCamera / p1.z());
    // p2 = p2 * (zDistCamera / p2.z());
    // double widthCam = (p1-p2).length3D();

    // return w / widthCam;
    return 1.0; 
}

void Camera::pixelToNDC(double x, double y, double &ndcX, double &ndcY) const
{
    // ndcX = float(x * 2.0 / float(m_drawable->width() -1) - 1.0); // FIXME: -1 might be wrong
    // ndcY = float(1.0 - y * 2.0 / float(m_drawable->height()-1)); // FIXME: -1 might be wrong
}


