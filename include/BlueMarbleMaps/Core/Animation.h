#ifndef ANIMATION
#define ANIMATION

#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Event/Signal.h"
#include <memory>

namespace BlueMarble
{

    enum class EasingFunctionType
    {
        Linear,
        EaseOut
    };

    class AbstractAnimation;
    typedef std::shared_ptr<AbstractAnimation> AbstractAnimationPtr;
    class AbstractAnimation
    {
        public:
            AbstractAnimation(double duration, EasingFunctionType easingFunc=EasingFunctionType::Linear);
            bool isFinished() const;
            bool updateTimeStamp(int timeStampMs);
            bool update(double elapsed);
            bool updateDelta(double deltaTime);
            void restart();

            double duration() const { return m_duration; };
            double elapsed() const { return m_elapsed; };
            double progress() const { return m_elapsed/m_duration; };

            struct AnimationEvents
            {
                Signal<AbstractAnimationPtr> onAnimationStarted;
                Signal<AbstractAnimationPtr> onAnimationFinished;
            };

        protected:
            virtual void onStarted() = 0;
            virtual void onUpdated(double progress) = 0;
            virtual void onFinished() = 0;
        private:
            double             m_duration;
            double             m_elapsed;
            int                m_startTimeStamp;
    };
    

    class Map; // Forward declaration

    struct InertiaOptions
    {
        double alpha = 0.001;
        double linearity = 0.2;
        double maxSpeed = 1000000;
    };

    class Animation; // Forward declaration
    typedef std::shared_ptr<Animation> AnimationPtr;

    class Animation
    {
        public:
            static AnimationPtr Create(Map& map, const Point& from, const Point& to, double duration, bool isInertial, const InertiaOptions& options = InertiaOptions());
            static AnimationPtr Create(Map& map, const Point& from, const Point& to, double fromScale, double toScale, double duration, bool isInertial, const InertiaOptions& options = InertiaOptions());
            Animation(Map& map, const Point& from, const Point& to, double duration, double fromScale, double toScale, bool isInertial, const InertiaOptions& options = InertiaOptions());

            const Point& fromCenter() { return m_from; }
            const Point& toCenter() { return m_to; }
            double fromScale() { return m_fromScale; }
            double toScale() { return m_toScale; }

            bool update(double elapsed);

            struct AnimationEvents
            {
                Signal<Animation*> onAnimationStarted;
                Signal<Animation*> onAnimationFinished;
            } events;

        private:
            double easeOut(double ratio, double easeOutPower);

            Map&  m_map;
            Point m_from;
            Point m_to;
            double m_fromScale;
            double m_toScale;
            double m_duration;
            bool m_isInertial;
            const InertiaOptions& m_options;
    };



    class AnimationController
    {
        public:
            AnimationController();
            bool update(int timeStamp);
            void addAnimation(AbstractAnimationPtr animation);
            void addPersistentAnimation(AbstractAnimationPtr animation);
            //AbstractAnimationPtr getAnimation(const Id& id);
        private:
            bool updateAnimations();
            //std::map<const T& key, AbstractAnimation> m_animations;
            std::vector<AbstractAnimationPtr> m_animations;
            std::vector<AbstractAnimationPtr> m_persistentAnimations;
            //std::map<Id, AbstractAnimationPtr> m_identitfiedAnimations;
    };


    /*
    AnimatedDoubleAttributeVariable myAttributeAnimation;           // keep around

    auto progressAnimation = animationController->getAnimation(id); // Get top animation for feature id
    auto linear progress = progressAnimation.getValue();            // Get progress for top animation (should have infinite duration?)
    myAnimation.updateProgress(progress);                           // Update own local animation using progress (own easing and parameters)
    double myDouble = myAnimation.tryGetValue(attributes);
    */

}
#endif /* ANIMATION */
