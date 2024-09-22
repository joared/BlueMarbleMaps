#ifndef BLUEMARBLE_ATTRIBUTEVARIABLE
#define BLUEMARBLE_ATTRIBUTEVARIABLE

#include "Animation.h"
#include "Attributes.h"

namespace BlueMarble
{

    template <typename T>
    class AttributeVariable
    {
        public:
            virtual ~AttributeVariable() = default;
            virtual bool tryGetValue(const Attributes& attributes, const T& val) = 0;
            bool operator() (const Attributes& attributes, const T& val) const
            {
                return tryGetValue(attributes, val);
            }
    };
    typedef AttributeVariable<int> IntegerAttributeVariable;
    typedef AttributeVariable<double> DoubleAttributeVariable;
    typedef AttributeVariable<std::string> StringAttributeVariable;
    typedef AttributeVariable<bool> BooleanAttributeVariable;
    // typedef AttributeVariable<Color> ColorAttributeVariable;


    template <typename T>
    class DirectAttributeVariable : public AttributeVariable
    {
        public:
            DirectAttributeVariable(const T& value)
                : m_value(value)
            {}
            
            bool tryGetValue(const Attributes& /*attributes*/, const T& val) override final
            {
                val = m_value;
                return true;
            }

        private:
            T m_value;
    };
    typedef DirectAttributeVariable<int> DirectIntegerAttributeVariable;
    typedef DirectAttributeVariable<double> DirectDoubleAttributeVariable;
    typedef DirectAttributeVariable<std::string> DirectStringAttributeVariable;
    typedef DirectAttributeVariable<bool> DirectBooleanAttributeVariable;


    template <typename T>
    class IndirectAttributeVariable : public AttributeVariable
    {
        public:
            IndirectAttributeVariable(const std::string& key, const T& defaultValue)
                : m_key(key)
                , m_defaultValue(defaultValue)
            {}

            bool tryGetValue(const Attributes& attributes, const T& val) override final
            {
                if (attributes.contains(m_key))
                {
                    val = attributes.get(m_key);
                    return true;
                }

                val = std::get<T>(m_defaultValue);
                return false;
            }

        private:
            std::string m_key;
            T           m_defaultValue;
    };
    typedef IndirectAttributeVariable<int> IndirectIntegerAttributeVariable;
    typedef IndirectAttributeVariable<double> IndirectDoubleAttributeVariable;
    typedef IndirectAttributeVariable<std::string> IndirectStringAttributeVariable;
    typedef IndirectAttributeVariable<bool> IndirectBooleanAttributeVariable;

    // Typename T needs to support operator+, operator- and operator*(double) to specialize this template
    template <typename T>
    class AnimatedAttributeVariable
        : public AttributeVariable
        , private AbstractAnimation // Animation interface is private, we update ourselfs
    {
        public:
            // template <typename T>
            AnimatedAttributeVariable(const AttributeVariable& from,
                                      const AttributeVariable& to,
                                      double duration, 
                                      EasingFunctionType easingFunc=EasingFunctionType::Linear)
                //: AttributeVariable(key, defaultValue)
                : AbstractAnimation(duration, easingFunc)
                , m_key(key)
                , m_progress(0)
                , m_from(from)
                , m_to(to)
                , m_value()
            {}

            // template <typename T>
            bool tryGetValue(const Attributes& attributes, const T& val) override final
            {
                double timeMs = attributes.get<int>(UpdateAttributeKeys::UpdateTimeMs);
                if (!attributes.contains(FeatureAttributeKeys::StartAnimationTimeMs))
                {
                    // First time the feature passes, add a timestamp
                    // TODO: maybe the dataset should do this?
                    attributes.set(FeatureAttributeKeys::StartAnimationTimeMs, timeMs);
                }

                double startTime = attributes.get<double>(FeatureAttributeKeys::StartAnimationTimeMs);
                update(timeMs - startTime);

                double p = progress(); // Progress includes easing

                const T& from;
                const T& to;
                if(!m_from.tryGetValue(attributes, from))
                {
                    // TODO: error handling
                    std::cout << "AnimatedAttributeVariable::tryGetValue(): Failed to retrieve from value\n".
                }
                if (!m_to.tryGetValue(attributes, to))
                {
                    // TODO: error handling
                    std::cout << "AnimatedAttributeVariable::tryGetValue(): Failed to retrieve to value\n".
                }

                val = from + (to-from)*p;
            }

        protected:
            void onStarted() override final 
            {

            };

            void onUpdated(double progress) override final 
            {

            };


            void onFinished() override final 
            {
                
            };
        private:
            AttributeVariable m_from;
            AttributeVariable m_to;
            T                 m_value;
    };

    typedef AnimatedAttributeVariable<double> AnimatedDoubleAttributeVariable;
    


};

#endif /* ATTRIBUTEVALUE */
