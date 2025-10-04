#ifndef BLUEMARBLE_ATTRIBUTEVARIABLE
#define BLUEMARBLE_ATTRIBUTEVARIABLE

#include "Animation.h"
#include "Attributes.h"
#include "DataSets/DataSet.h"

namespace BlueMarble
{

    template <typename T>
    class AttributeVariable
    {
        public:
            virtual ~AttributeVariable() = default;
            virtual bool tryGetValue(const FeaturePtr& f, Attributes& attributes, T& val) const { return false; };

            T operator() (const FeaturePtr& f, Attributes& attributes) const
            {
                T val;
                if(!tryGetValue(f, attributes, val))
                {
                    std::cout << "AttributeVariable::operator() Failed to get value, undefined return value.\n";
                }

                return val;
            }
    };
    typedef AttributeVariable<int> IntegerAttributeVariable;
    typedef AttributeVariable<double> DoubleAttributeVariable;
    typedef AttributeVariable<std::string> StringAttributeVariable;
    typedef AttributeVariable<bool> BooleanAttributeVariable; 
    typedef AttributeVariable<Color> ColorAttributeVariable;

    template <typename T>
    using EvaluationFunction = std::function<T(const FeaturePtr&, Attributes&)>;

    template <typename T>
    class ArtibtraryAttributeVariable : public AttributeVariable<T>
    {
        public:
            ArtibtraryAttributeVariable(const ArtibtraryAttributeVariable<T>& other)
                : m_function(other.m_function)
            {}

            template <typename Callable>
            ArtibtraryAttributeVariable(Callable function) // Needs to be a copy
                : m_function(std::forward<Callable>(function)) 
            {}

            ArtibtraryAttributeVariable(const EvaluationFunction<T>& function)
                : m_function(function)
            {}

            template <typename Callable>
            void operator=(Callable function) // Needs to be a copy
            {
                m_function = std::forward<Callable>(function);
            }

            void operator=(const EvaluationFunction<T>& function)
            {
                m_function = function;
            }

            bool tryGetValue(const FeaturePtr& f, Attributes& attributes, T& val) const override final
            {
                val = T(m_function(f, attributes));
                return true;
            }

        private:
            EvaluationFunction<T> m_function;
    };

    typedef ArtibtraryAttributeVariable<bool>         Condition;
    typedef ArtibtraryAttributeVariable<Color>        ColorEvaluation;
    typedef ArtibtraryAttributeVariable<int>          IntEvaluation;
    typedef ArtibtraryAttributeVariable<double>       DoubleEvaluation;
    typedef ArtibtraryAttributeVariable<std::string>  StringEvaluation;

    template <typename T>
    class DirectAttributeVariable : public AttributeVariable<T>
    {
        public:
            DirectAttributeVariable(const T& value)
                : m_value(value)
            {}
            
            bool tryGetValue(const FeaturePtr& /*f*/, Attributes& /*attributes*/, T& val) const override final
            {
                val = T(m_value);
                return true;
            }

        private:
            T m_value;
    };
    typedef DirectAttributeVariable<int> DirectIntegerAttributeVariable;
    typedef DirectAttributeVariable<double> DirectDoubleAttributeVariable;
    typedef DirectAttributeVariable<std::string> DirectStringAttributeVariable;
    typedef DirectAttributeVariable<bool> DirectBooleanAttributeVariable;
    typedef DirectAttributeVariable<Color> DirectColorAttributeVariable;


    template <typename T, typename U=T>
    class IndirectAttributeVariable : public AttributeVariable<T>
    {
        public:
            IndirectAttributeVariable(const std::string& key)
                : m_key(key)
                , m_defaultValue(T())
                , m_hasDefaultValue(false)
            {}

            IndirectAttributeVariable(const std::string& key, const T& defaultValue)
                : m_key(key)
                , m_defaultValue(defaultValue)
                , m_hasDefaultValue(true)
            {}

            bool tryGetValue(const FeaturePtr& f, Attributes& attributes, T& val) const override final
            {
                if (f->attributes().contains(m_key))
                {
                    val = f->attributes().get<T>(m_key);
                    return true;
                }
                else if (attributes.contains(m_key))
                {
                    val = attributes.get<T>(m_key);
                    return true;
                }

                val = m_defaultValue;

                return m_hasDefaultValue;
            }

        private:
            std::string m_key;
            T           m_defaultValue;
            bool        m_hasDefaultValue;
    };
    typedef IndirectAttributeVariable<int> IndirectIntegerAttributeVariable;
    typedef IndirectAttributeVariable<double> IndirectDoubleAttributeVariable;
    typedef IndirectAttributeVariable<std::string> IndirectStringAttributeVariable;
    typedef IndirectAttributeVariable<bool> IndirectBooleanAttributeVariable;

    // Typename T needs to support operator+, operator- and operator*(double) to specialize this template
    template <typename T>
    class AnimatedAttributeVariable
        : public AttributeVariable<T>
        , private AbstractAnimation // Animation interface is private, we update ourselfs
    {
        public:
            // template <typename T>
            AnimatedAttributeVariable(AttributeVariable<T>& from,
                                      AttributeVariable<T>& to,
                                      double duration, 
                                      EasingFunctionType easingFunc=EasingFunctionType::Linear)
                //: AttributeVariable(key, defaultValue)
                : AbstractAnimation(duration, easingFunc)
                , m_from(from)
                , m_to(to)
                , m_value()
            {}

            // template <typename T>
            bool tryGetValue(const FeaturePtr& f, Attributes& attributes, T& val) const override final
            {
                // TODO: since we are using int, timestamps may be negative
                int timeMs = attributes.get<int>(UpdateAttributeKeys::UpdateTimeMs);
                // if (!f->attributes().contains(FeatureAttributeKeys::StartAnimationTimeMs))
                // {
                //     // No animation start time info, return "to" value
                //     return m_to.tryGetValue(f, attributes, val);
                // }
                // double startTimeMs = f->attributes().get<int>(FeatureAttributeKeys::StartAnimationTimeMs);

                auto dataSet = DataSet::getDataSetById(f->id().dataSetId());
                int startTimeMs = dataSet->getVisualizationTimeStampForFeature(f->id());

                
                double elapsed = timeMs - startTimeMs;

                if (elapsed < 0)
                {
                    return m_to.tryGetValue(f, attributes, val);
                }

                double p = elapsed/duration(); // TODO: easing

                bool retVal = true;
                T to;
                if (!m_to.tryGetValue(f, attributes, to))
                {
                    retVal = false;
                    std::cout << "AnimatedAttributeVariable::tryGetValue(): Failed to retrieve to value\n";
                }

                if (p >= 1.0)
                {
                    val = to;
                    return retVal;
                }

                T from;
                if(!m_from.tryGetValue(f, attributes, from))
                {
                    // TODO: error handling
                    retVal = false;
                    std::cout << "AnimatedAttributeVariable::tryGetValue(): Failed to retrieve from value\n";
                }
                

                val = from + (to-from)*p;

                // Set update required such that the animation continues
                attributes.set(UpdateAttributeKeys::UpdateRequired, true);

                return retVal;
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
            AttributeVariable<T>& m_from;
            AttributeVariable<T>& m_to;
            T                     m_value;
    };

    typedef AnimatedAttributeVariable<double> AnimatedDoubleAttributeVariable;
};

#endif /* ATTRIBUTEVALUE */
