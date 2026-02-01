#ifndef JSONVALUE
#define JSONVALUE

#include <iostream>
#include <variant>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <initializer_list>
#include <algorithm>

namespace BlueMarble
{
    
#define DEBUG_JsonValue false

class JsonParseHandler;

class ICharReader
{
public:
    virtual ~ICharReader() = default;
    virtual char peek() const = 0;
    virtual char get() = 0;
    virtual bool eof() const = 0;
};

class StringReader : public ICharReader
{
public:
    explicit StringReader(const std::string& str)
        : m_str(str)
        , m_idx(0)
        {}
    
    virtual char peek() const override final
    {
        return m_str[m_idx];
    }

    virtual char get() override final
    {
        return m_str[m_idx++];
    }

    virtual bool eof() const override final
    {
        return m_idx >= m_str.size();
    }   
private:
    const std::string& m_str;
    size_t             m_idx;
};

class StreamReader : public ICharReader
{
public:
    explicit StreamReader(std::istream& stream)
        : m_stream(stream)
        {}
    
    virtual char peek() const override final
    {
        return m_stream.peek();
    }

    virtual char get() override final
    {
        if (eof()) throw std::runtime_error("StreamReader eof");
        return m_stream.get();
    }

    virtual bool eof() const override final
    {
        return m_stream.eof();
    }   
private:
    std::istream& m_stream;
};

class JsonValue
{
public:
    using Object = std::map<std::string, JsonValue>;
    using Array  = std::vector<JsonValue>;
    using Null   = std::monostate;
    using Value  = std::variant<Null, bool, int64_t, double, std::string, Array, Object>;

    JsonValue() : m_val(Null{}) {}
    // TODO: make only movable?
    // JsonValue(JsonValue&& v) : m_val(std::move(v.m_val)) {}
    // void operator=(JsonValue&& v) { m_val = std::move(v.m_val); }
    // Move semantics (explicit)
    JsonValue(JsonValue&& v) noexcept = default;
    JsonValue& operator=(JsonValue&& v) noexcept = default;
    
    // Delete copies to prevent accidental copying
    JsonValue(const JsonValue&) = default;
    JsonValue& operator=(const JsonValue&) = delete;

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
                auto& arr = element.asArray();   // arr[0] = key, arr[1] = value
                obj.emplace(arr[0].asString(), std::move(arr[1]));
            }
            m_val = std::move(obj);
        }
        else
        {
            // the initializer list describes an array -> create an array
            m_val = Array(init);
        }
    }

    // void operator=(JsonValue&& v) { m_val = std::move(v.m_val); }

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
    std::string& asString() { return std::get<std::string>(m_val); }
    const Array& asArray() const { return std::get<Array>(m_val); }
    Array& asArray() { return std::get<Array>(m_val); }
    const Object& asObject() const { return std::get<Object>(m_val); }
    Object& asObject() { return std::get<Object>(m_val); }

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

    static JsonValue fromStream(std::istream& ss);
    static JsonValue fromString(const std::string& str);
    static bool fromStream(std::istream& ss, JsonParseHandler* handler);
    static bool fromString(const std::string& str, JsonParseHandler* handler);

    std::string toString(bool format=false) const;

private:

    std::string toString(const std::string& currentIndentation, const std::string& baseIndentation) const;

private:
    Value m_val;
};

class JsonParseHandler
{
    public:
        virtual ~JsonParseHandler() = default;

        // structure
        virtual bool onStartObject(std::size_t /*estimated_size*/) { return true; }
        virtual bool onEndObject() { return true; }

        virtual bool onStartArray(std::size_t /*estimated_size*/) { return true; }
        virtual bool onEndArray() { return true; }

        // object-specific
        virtual bool onKey(std::string&& key) { return true; }

        // values
        virtual bool onNull() { return true; }
        virtual bool onBool(bool v) { return true; }
        virtual bool onInteger(int64_t v) { return true; }
        virtual bool onDouble(double v) { return true; }
        virtual bool onString(std::string&& v) { return true; }

        virtual bool onError(std::string&& v) { return false; }
};


bool parseJson(ICharReader* reader, JsonParseHandler* handler);

}

#endif /* JSONVALUE */
