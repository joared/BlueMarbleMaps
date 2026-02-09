#include "BlueMarbleMaps/Core/Serialization/Json/JsonValue.h"
#include "BlueMarbleMaps/Core/Serialization/Json/JsonDetails.h"
#include <cassert>

namespace BlueMarble
{

class JsonDomBuilder final : public JsonParseHandler
{
public:
    // ----- Scalars -----

    bool onNull() override
    {
        addValue(JsonValue(nullptr));
        return true;
    }

    bool onBool(bool v) override
    {
        addValue(JsonValue(v));
        return true;
    }

    bool onInteger(int64_t v) override
    {
        addValue(JsonValue(v));
        return true;
    }

    bool onDouble(double v) override
    {
        addValue(JsonValue(v));
        return true;
    }

    bool onString(std::string&& v) override
    {
        addValue(JsonValue(std::move(v)));
        return true;
    }

    // ----- Object / array structure -----
    bool onKey(std::string&& key) override
    {
        m_keyStack.emplace_back(std::move(key));
        return true;
    }

    bool onStartObject(std::size_t /*estimated_size*/) override
    {
        m_stack.emplace_back(JsonValue::Object{});
        return true;
    }

    bool onEndObject() override
    {
        JsonValue obj = std::move(m_stack.back());
        m_stack.pop_back();
        addValue(std::move(obj));
        return true;
    }

    bool onStartArray(std::size_t /*estimated_size*/) override
    {
        m_stack.emplace_back(JsonValue::Array{});
        return true;
    }

    bool onEndArray() override
    {
        JsonValue arr = std::move(m_stack.back());
        m_stack.pop_back();
        addValue(std::move(arr));
        return true;
    }

    bool onError(std::string&& error) override
    {
        std::cout << error << "\n";
        return false;
    }

    JsonValue&& takeResult() 
    { 
        return std::move(m_result);
    }

private:
    void addValue(JsonValue&& v)
    {
        if (m_stack.empty())
        {
            m_result = std::move(v);
            return;
        }

        JsonValue& top = m_stack.back();

        if (top.isArray())
        {
            top.asArray().emplace_back(std::move(v));
        }
        else
        {
            assert(!m_keyStack.empty());

            std::string key = std::move(m_keyStack.back());
            m_keyStack.pop_back();

            top.asObject().emplace(
                std::move(key),
                std::move(v)
            );
        }
    }

private:
    JsonValue m_result;
    std::vector<JsonValue> m_stack;
    std::vector<std::string> m_keyStack;
};

JsonValue JsonValue::fromStream(std::istream &ss)
{
    JsonDomBuilder domBuilder;
    StreamReader reader(ss);

    if (load(&reader, &domBuilder))
    {
        return std::move(domBuilder.takeResult());
    }
    
    return std::move(JsonValue());
}

JsonValue JsonValue::fromString(const std::string &str)
{
    JsonDomBuilder domBuilder;
    StringReader strReader(str);
    if (load(&strReader, &domBuilder))
    {
        return std::move(domBuilder.takeResult());
    }

    return std::move(JsonValue());
}


std::string JsonValue::toString(bool format) const
{
    std::string baseIndentation="";
    if (format)
    {
        baseIndentation = "  ";
    }

    return toString("", baseIndentation);
}

// TODO: does thi even work?
std::string escapeString(const std::string& s) 
{
    std::string result;
    for (char c : s) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b";  break;
            case '\f': result += "\\f";  break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) 
                {
                    // escape control characters as \u00XX
                    char buf[7];
                    sprintf(buf, "\\u%04x", c);
                    result += buf;
                } 
                else 
                {
                    result += c;
                }
        }
    }
    return result;
}

std::string JsonValue::toString(const std::string& currentIndentation, const std::string &baseIndentation) const
{
    bool doFormat = !baseIndentation.empty();
    std::string space = doFormat ? " " : "";
    std::string newLine = doFormat ? "\n" : "";
    std::string elementIndentation = doFormat ? currentIndentation + baseIndentation : "";

    if (isType<Object>())
    {
        std::string s = "{";
        s += newLine;
        int i = 0;
        for (const auto& [key, val] : get<Object>())
        {
            s += elementIndentation + "\"" + key + "\":";
            s += space;
            s += val.toString(elementIndentation, baseIndentation);
            if (i < get<Object>().size()-1)
            {
                s += ",";
                s += newLine;
            }
            ++i;
        }
        s += newLine + currentIndentation;
        s += "}";

        return s;
    }
    else if (isType<Array>())
    {
        std::string s = "[";
        s += newLine;
        int i = 0;
        for (const auto& val : get<Array>())
        {
            s += elementIndentation + val.toString(elementIndentation, baseIndentation);
            if (i < get<Array>().size()-1)
            {
                s += ",";
                s += newLine;
            }
            ++i;
        }
        s += newLine + currentIndentation;
        s += "]";

        return s;
    }
    else if (!hasValue())
    {
        return "null";
    }
    else if (isBool())
    {
        return asBool() ? "true" : "false";
    }
    else if (isInteger())
    {
        return std::to_string(asInteger());
    }
    else if (isDouble())
    {
        return std::to_string(asDouble());
    }
    else if (isString())
    {
        // TODO: fix escape
        return "\"" + escapeString(get<std::string>()) + "\"";
    }
    else
    {
        std::cout << "Unhandled JsonValue value\n";
    }

    return "";
}

};
