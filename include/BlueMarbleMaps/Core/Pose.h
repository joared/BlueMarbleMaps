#ifndef BMM_POSE
#define BMM_POSE

#include "glm.hpp"

class Orientation
{
public:
    static fromEuler(double roll, double pitch, double yaw) { return glm::}

    Orientation() : m_quat(0,0,0,1) {}

private:
    glm::quat m_quat;
};

class Pose
{

};

#endif /* BMM_POSE */
