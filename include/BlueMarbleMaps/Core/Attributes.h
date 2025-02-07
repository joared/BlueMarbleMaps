#ifndef BLUEMARBLE_ATTRIBUTES
#define BLUEMARBLE_ATTRIBUTES

#include <string>
#include <map>
#include <variant>
#include <memory>
#include <iostream>

namespace BlueMarble
{
    namespace UpdateAttributeKeys
    {
        const std::string UpdateTimeMs = std::string("__timeMs");
        const std::string UpdateViewScale = std::string("__updateViewScale");
        const std::string QuickUpdate = std::string("__quickUpdate");
        const std::string SelectionUpdate = std::string("__selection");
        const std::string HoverUpdate = std::string("__hover");
        const std::string UpdateRequired = std::string("updateRequired__");
    };

    namespace FeatureAttributeKeys
    {
        const std::string StartAnimationTimeMs = std::string("__animationTimeMs");
    };

    // TODO: make attributes an std::map and use class for AttributeValue with get/set?
    // "contatins" method is convenient, maybe Attributes both should its own class still
    // TODO: big issue when introducing bool, where "hello" in "set" can be interpreted as bool in the template argument deduction
    using AttributeValue = std::variant<int, double, std::string, bool>;
    enum class AttributeValueType
    {
        Integer,
        Double,
        String,
        Boolean
    };

    inline std::string attributeToString(const AttributeValue& attr)
    {
        switch (attr.index())
        {
        case 0:
            return std::to_string(std::get<int>(attr));
        case 1:
            return std::to_string(std::get<double>(attr));
        case 2:
            return std::get<std::string>(attr);
        case 3:
            return std::get<bool>(attr) ? "true" : "false";
        default:
            std::cout << "attributeToString() Unhandled index: " << attr.index() << "\n";
            return "Invalid attribute index: " + std::to_string(attr.index());
        }
    }

    
    class Attributes
    {
        public:
            inline Attributes()
                : m_attributes()
            {}
            
            template <typename T>
            inline void set(const std::string& key, const T& value) { m_attributes[key] = value; }
            
            template <typename T>
            inline const T& get(const std::string& key) const {
                auto it = m_attributes.find(key);
                if (it != m_attributes.end()) {
                    return std::get<T>(it->second);
                }
                throw std::runtime_error("Key '" + key + "' not found");
            }

            template <typename T>
            inline const T& tryGet(const std::string& key) const {
                try
                {
                    return get<T>(key);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                    auto it = m_attributes.find(key);
                    auto value = it->second;
                    std::cout << "Tried to get index: " << value.index() << "\n";
                    throw e;
                }
                
            }

            inline const AttributeValue& val(const std::string& key) { return m_attributes[key]; }

            inline bool contains(const std::string& key) const { return m_attributes.find(key) != m_attributes.end(); }
            
            inline size_t size() const { return m_attributes.size(); }
            inline auto begin() const { return m_attributes.begin(); }
            inline auto end() const { return m_attributes.end(); }
            // inline auto cbegin() const { return m_attributes.cbegin(); }
            // inline auto cend() const { return m_attributes.cend(); }

        private:
            std::map<std::string, AttributeValue>   m_attributes;
    };
    typedef std::shared_ptr<Attributes> AttributesPtr;

}

#endif /* BLUEMARBLE_ATTRIBUTES */
