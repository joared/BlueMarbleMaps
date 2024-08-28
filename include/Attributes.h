#ifndef BLUEMARBLE_ATTRIBUTES
#define BLUEMARBLE_ATTRIBUTES

#include <string>
#include <map>
#include <variant>

namespace BlueMarble
{

    // TODO: make attributes an std::map and use class for AttributeValue with get/set?
    // "contatins" method is convenient, maybe Attributes both should its own class still
    class Attributes
    {
        public:
            using AttributeValue = std::variant<int, double, std::string>;

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

}

#endif /* BLUEMARBLE_ATTRIBUTES */
