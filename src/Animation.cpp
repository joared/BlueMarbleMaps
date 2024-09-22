#include <cmath>
#include "Animation.h"
#include "Map.h"

using namespace BlueMarble;


AbstractAnimation::AbstractAnimation(double duration, EasingFunctionType easingFunc)
    : m_duration(duration)
    , m_elapsed(0)
{
}

bool AbstractAnimation::isFinished() const
{
    return m_elapsed >= m_duration;
}

bool AbstractAnimation::update(double elapsed)
{
    if (isFinished())
    {
        std::cout << "AbstractAnimation::update(): update called on finished animation not allowed!\n";
        throw std::exception();
    }

    m_elapsed = elapsed;
    if (m_elapsed == 0)
        onStarted();

    double progress = m_elapsed/m_duration; // TODO: apply easing function
    onUpdated(progress);

    if (isFinished())
    {
        m_elapsed = m_duration;
        onFinished();
    }
}

bool AbstractAnimation::updateDelta(double deltaTime)
{
    update(m_elapsed + deltaTime);
}

void BlueMarble::AbstractAnimation::restart()
{
    m_elapsed = 0;
}

AnimationPtr Animation::Create(Map &map, const Point &from, const Point &to, double duration, bool isInertial, const InertiaOptions &options)
{
    return AnimationPtr(new Animation(map, from, to, map.scale(), map.scale(), duration, isInertial, options));
}

AnimationPtr BlueMarble::Animation::Create(Map &map, const Point &from, const Point &to, double fromScale, double toScale, double duration, bool isInertial, const InertiaOptions &options)
{
    return AnimationPtr(new Animation(map, from, to, fromScale, toScale, duration, isInertial, options));
}

Animation::Animation(Map &map, 
                     const Point &from, 
                     const Point &to, 
                     double fromScale, 
                     double toScale,
                     double duration, 
                     bool isInertial, 
                     const InertiaOptions &options)
    : m_map(map)
    , m_from(from)
    , m_to(to)
    , m_fromScale(fromScale)
    , m_toScale(toScale)
    , m_duration(duration)
    , m_isInertial(isInertial)
    , m_options(options)
{
}

bool Animation::update(double elapsed)
{
    if (elapsed >= m_duration)
    {
        m_map.center(m_to);
        m_map.scale(m_toScale);
        return true;
    }

    // TODO: special handling for inertial animation
    double ratio = elapsed / m_duration;
    double easeOutPower = m_isInertial ? 1.0/m_options.linearity : 4.0;
    double progress = easeOut(ratio, easeOutPower);

    m_map.center(m_from + (m_to-m_from)*progress);

    double invFrom = 1.0 / m_fromScale;
    double invTo = 1.0 / m_toScale;
    double invNew = invFrom + (invTo-invFrom)*progress;
    m_map.scale(1.0 / invNew);

    return false;
}

double Animation::easeOut(double ratio, double easeOutPower)
{
    return 1 - std::pow(1 - ratio, easeOutPower);
}

AnimationController::AnimationController()
    : m_animations()
    , m_persistentAnimations()
{
}

bool AnimationController::update(int timeStamp)
{
    bool needMoreUpdates = false;
    for (auto a : m_animations)
    {
        needMoreUpdates = needMoreUpdates || a->update(timeStamp);
        if (a->isFinished())
        {
            
        }
    }
}

void AnimationController::addAnimation(AbstractAnimationPtr animation)
{
}

void AnimationController::addPersistentAnimation(AbstractAnimationPtr animation)
{
}

bool AnimationController::updateAnimations()
{
    return false;
}
