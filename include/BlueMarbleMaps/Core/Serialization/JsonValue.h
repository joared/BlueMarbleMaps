#ifndef BLUEMARBLE_JSON
#define BLUEMARBLE_JSON

#include <iostream>
#include <variant>
#include <map>
#include <vector>
#include <string>
#include <initializer_list>
#include <algorithm>

namespace BlueMarble
{
    
#define DEBUG_JsonValue false

class JsonValue
{
public:
    using Object = std::map<std::string, JsonValue>;
    using Array  = std::vector<JsonValue>;
    using Null   = std::monostate;
    using Value  = std::variant<Null, bool, int64_t, double, std::string, Array, Object>;

    JsonValue() : m_val(Null{}) {}

    //JsonValue(bool v) : m_val(v) {} // Can apparently do JsonValue({}) which is bool
    //JsonValue(std::initializer_list<bool>) = delete;
    // JsonValue(bool v)
    //     requires(!std::is_same_v<std::decay_t<decltype(v)>, std::initializer_list<JsonValue>>) 
    //     : m_val(v) {}
    JsonValue(std::nullptr_t) : m_val(Null{}) {}
    template <typename T, typename = std::enable_if_t<std::is_same_v<T, bool>>>
    JsonValue(T v) : m_val(v) {}
    template <typename T,
          typename = std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T,bool>>, typename = void>
    JsonValue(T v) : m_val(std::in_place_type<int64_t>, static_cast<int64_t>(v)) {}
    // JsonValue(int64_t v) : m_val(v) {}
    // JsonValue(int v) : m_val(std::in_place_type<int64_t>, v) {}

    JsonValue(double v) : m_val(v) {}
    
    JsonValue(const char* v) : m_val(std::string(v)) {}
    JsonValue(const std::string& v) : m_val(v) {}
    JsonValue(std::string&& v) : m_val(std::move(v)) {}
    JsonValue(const Array& v) : m_val(v) {}
    JsonValue(Array&& v) : m_val(std::move(v)) {}
    JsonValue(const Object& v) : m_val(v) {}
    JsonValue(Object&& v) : m_val(std::move(v)) {}

    JsonValue(std::initializer_list<JsonValue> init)
    {
        if (DEBUG_JsonValue) std::cout << "Array constructor (direct)\n";
         // check if each element is an array with two elements whose first
        // element is a string
        bool isAnObject = std::all_of(init.begin(), init.end(),
                                        [](const JsonValue& jsonValue)
        {
            // The cast is to ensure op[size_type] is called, bearing in mind size_type may not be int;
            // (many string types can be constructed from 0 via its null-pointer guise, so we get a
            // broken call to op[key_type], the wrong semantics, and a 4804 warning on Windows)
            return jsonValue.isType<Array>() && jsonValue.asArray().size() == 2 && jsonValue.asArray()[0].isType<std::string>();
        });

        if (isAnObject)
        {
            Object obj;
            for (const auto& element : init)
            {
                const auto& arr = element.asArray();   // arr[0] = key, arr[1] = value
                obj.emplace(arr[0].asString(), arr[1]);
            }
            m_val = std::move(obj);
        }
        else
        {
            // the initializer list describes an array -> create an array
            m_val = Array(init);
        }
    }

    bool hasValue() const { return !std::holds_alternative<std::monostate>(m_val); }
    bool isBool() const { return std::holds_alternative<bool>(m_val); }
    bool isInteger() const { return std::holds_alternative<int64_t>(m_val); }
    bool isDouble() const { return std::holds_alternative<double>(m_val); }
    bool isString() const { return std::holds_alternative<std::string>(m_val); }
    bool isArray() const { return std::holds_alternative<Array>(m_val); }
    bool isObject() const { return std::holds_alternative<Object>(m_val); }

    bool asBool() const { return std::get<bool>(m_val); }
    int64_t asInteger() const { return std::get<int64_t>(m_val); }
    double asDouble() const { return std::get<double>(m_val); }
    const std::string& asString() const { return std::get<std::string>(m_val); }
    const Array& asArray() const { return std::get<Array>(m_val); }
    const Object& asObject() const { return std::get<Object>(m_val); }

    template<typename T>
    bool tryGet(T& val) const 
    { 
        if (isType<T>())
        {
            val = get<T>(m_val);
            return true;
        }
        return false;
    }

    // type queries
    template<typename T>
    bool isType() const { return std::holds_alternative<T>(m_val); }

    template<typename T>
    const T& get() const { return std::get<T>(m_val); }

    template<typename T>
    T& get() { return std::get<T>(m_val); }

    // template <typename Serializer>
    // std::string serialize(Serializer s)
    // {
    //     // using T = std::decay_t<decltype(v)>

    // }

    static JsonValue fromString(const std::string& str);

    std::string toString(bool format=false) const;

private:

    std::string toString(const std::string& currentIndentation, const std::string& baseIndentation) const;

private:
    Value m_val;
};

class JSONParseHandler
{
    public:
        virtual void onInteger(int value) {}
        virtual void onDouble(double value) {}
        virtual void onString(const std::string& value) {}
        virtual void onNull() {}

        virtual void onStartArray(const JsonValue::Array& value) {}
        virtual void onEndArray(const JsonValue::Array& value) {}

        virtual void onStartObject(const JsonValue::Object& value) {}
        virtual void onEndObject(const JsonValue::Object& value) {}
        
        virtual void onKey(const std::string& key) {}
};

bool isEndOfValueChar(const char& c);
void expect(const char& c, const std::string& text, int& idx);
void parseWhiteSpace(const std::string& text, int& idx);
std::string parseKey(const std::string& text, int& idx);
std::pair<std::string, JsonValue> retrieveKeyValuePair(const std::string& text, int& idx, int level);
JsonValue parseJson(const std::string& text, int& idx, int level);

}

#endif /* BLUEMARBLE_JSON */
