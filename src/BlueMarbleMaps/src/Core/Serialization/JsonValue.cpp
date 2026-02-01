#include "BlueMarbleMaps/Core/Serialization/Json/JsonValue.h"
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
    if (fromStream(ss, &domBuilder))
    {
        return std::move(domBuilder.takeResult());
    }
    
    return std::move(JsonValue());
}

bool JsonValue::fromStream(std::istream &ss, JsonParseHandler *handler)
{
    StreamReader reader(ss);
    try
    {
        return parseJson(&reader, handler);
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
        return false;
    }

    return true;
}

JsonValue JsonValue::fromString(const std::string &str)
{
    JsonDomBuilder domBuilder;
    if (fromString(str, &domBuilder))
    {
        return domBuilder.takeResult();
    }

    return JsonValue();
}

bool JsonValue::fromString(const std::string &str, JsonParseHandler *handler)
{
    StringReader reader(str);
    try
    {
        return parseJson(&reader, handler);
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
        return false;
    }

    return true;
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

inline bool expect(const char &c, ICharReader* reader, JsonParseHandler* handler)
{
    if (c != reader->peek())
    {
        return handler->onError("Expected '" + std::string(1, c) + "' but got '" + reader->peek() + "'");
    }
    return true;
}

inline bool isEndOfValueChar(const char &c)
{
    // Used for int/double parsing
    return c == ',' || c == '}' || c == ']';
}

inline void parseWhiteSpace(ICharReader* reader)
{
    char v = reader->peek();
    while (v == ' ' || v == '\n' || v == '\r' || v == '\t')
    {
        reader->get();
        v = reader->peek();
    }
}

inline bool parseKey(ICharReader* reader, JsonParseHandler* handler)
{
    if (!expect('\"', reader, handler)) return false;
    reader->get();

    std::string key;
    while (reader->peek() != '\"')
    {
        key += reader->peek();
        reader->get();
    }
    reader->get();

    //assert(!key.empty());
    if (key.empty() && !handler->onError("Empty key"))
    {
        return false;
    }

    return handler->onKey(std::move(key));
}

inline bool parseJson(ICharReader* reader, JsonParseHandler* handler, int level=0)
{
    parseWhiteSpace(reader);

    if (level == 0)
    {
        if (reader->eof())
        {
            handler->onError("Empty JSON"); 
            return false;
        }
        char firstChar = reader->peek();
        if (firstChar != '{' && firstChar != '[')
        {
            if (!handler->onError("A JSON payload should be an object or array, its not!")) return false;
        }
    }
    ++level;

    if (reader->peek() == ',')
    {
        if (!handler->onError("Trailing comma")) return false;
        reader->get();
        parseWhiteSpace(reader);
    }

    if (reader->peek() == '{')
    {
        // Object
        reader->get();
        if (!handler->onStartObject(-1)) return false;
        bool expectCommaOrClose = false;
        bool closed = reader->peek() == '}';
        if (closed) reader->get();
        while (!closed)
        {
            parseWhiteSpace(reader);
            if (expectCommaOrClose)
            {
                char commaOrClose = reader->peek();
                if (commaOrClose == ',')
                {
                    reader->get();
                }
                else if (commaOrClose == '}')
                {
                    // Array closed
                    reader->get();
                    closed = true;
                }
                else
                {
                    std::string err = "Expected a comma or close when parsing object, but got '";
                    err += std::string(1, reader->peek());
                    err += "'";
                    if (!handler->onError(std::move(err))) return false;
                }
            }
            else // expect key-value pair
            {
                if (!parseKey(reader, handler)) return false;
                parseWhiteSpace(reader);
                if (!expect(':', reader, handler)) return false;
                reader->get();
                parseWhiteSpace(reader);
                if (!parseJson(reader, handler, level)) return false;
            }
            expectCommaOrClose = !expectCommaOrClose;
            // parseWhiteSpace(reader);
            // if (!parseKey(reader, handler)) return false;
            // parseWhiteSpace(reader);
            // if (!expect(':', reader, handler)) return false;
            // reader->get();
            // parseWhiteSpace(reader);
            // if (!parseJson(reader, level, handler)) return false;
            // parseWhiteSpace(reader);
            // // FIXME: this is needed if comma is forgotten, but parseValue should not step over ','
            // if (reader->peek() != '}')
            // {
            //     if (!expect(',', reader, handler)) return false;
            //     reader->get();
            //     parseWhiteSpace(reader);
            // }
        }

        return handler->onEndObject();
    }
    else if (reader->peek() == '[')
    {
        // Array
        if (!handler->onStartArray(-1)) return false;
        
        reader->get();
        parseWhiteSpace(reader);
        bool expectCommaOrClose = false; // false means value
        bool closed = reader->peek() == ']';
        if (closed) reader->get();
        while (!closed)
        {
            parseWhiteSpace(reader);
            if (expectCommaOrClose)
            {
                char commaOrClose = reader->get();
                if (commaOrClose == ',')
                {
                    
                }
                else if (commaOrClose == ']')
                {
                    closed = true;
                }
                else
                {
                    std::string err = "Expected a comma or close when parsing array, but got '";
                    err += std::string(1, reader->peek());
                    err += "'";
                    if (!handler->onError(std::move(err))) return false;
                }
            }
            else // expect value
            {
                if (!parseJson(reader, handler, level)) return false;
            }
            expectCommaOrClose = !expectCommaOrClose;
            parseWhiteSpace(reader);

        }

        return handler->onEndArray();
    }
    else if (reader->peek() == '\"')
    {
        // TODO: escape sequences
        // String
        reader->get();
        std::string value;
        while (reader->peek() != '\"')
        {
            // TODO: not working
            // value += reader->peek();
            // reader->get();
            char c = reader->peek();

            if (c == '\\') 
            { // escape sequence
                reader->get();
                if (reader->eof()) break; // error: unexpected end

                char esc = reader->peek();
                switch (esc) {
                    case '"': value += '"'; break;
                    case '\\':value += '\\'; break;
                    case '/': value += '/'; break;
                    case 'b': value += '\b'; break;
                    case 'f': value += '\f'; break;
                    case 'n': value += '\n'; break;
                    case 'r': value += '\r'; break;
                    case 't': value += '\t'; break;
                    case 'u': value += "HEX not implemented:"; break;
                    // optionally: handle \uXXXX unicode escapes
                    default:
                    {
                        if (!handler->onError("Invalid escape sequence")) return false; 
                    }
                }
            }
            else if (c == '"') 
            {
                reader->get(); // closing quote
                break;
            }
            else 
            {
                value += c;
            }
            reader->get();
        }
        
        if (value.empty())
        {
            if (!handler->onError("Parsed empty string")) return false;
        }
        reader->get(); // after last '"'

        return handler->onString(std::move(value));
    }
    else if (reader->peek() == 'n')
    {
        // Null
        reader->get(); if(!expect('u', reader, handler)) return false;
        reader->get(); if(!expect('l', reader, handler)) return false;
        reader->get(); if(!expect('l', reader, handler)) return false;
        reader->get();

        return handler->onNull();
    }
    else if (reader->peek() == 't' || reader->peek() == 'f')
    {
        // Boolean
        bool value;
        if (reader->peek() == 't')
        {
            reader->get(); if(!expect('r', reader, handler)) return false;
            reader->get(); if(!expect('u', reader, handler)) return false;
            reader->get(); if(!expect('e', reader, handler)) return false;
            
            value = true;
        }
        else // we know its 'f'
        {
            reader->get(); if(!expect('a', reader, handler)) return false;
            reader->get(); if(!expect('l', reader, handler)) return false;
            reader->get(); if(!expect('s', reader, handler)) return false;
            reader->get(); if(!expect('e', reader, handler)) return false;

            value = false;
        }
        reader->get();

        return handler->onBool(value);
    }
    else
    {
        // Double/int
        
        std::string value;
        while (!isEndOfValueChar(reader->peek()))
        {
            value += reader->get();
        }
        if (value.empty())
        {
            if (!handler->onError("Expected double/int but parsed empty value")) return false;
        }
        
        if (value.find('.') == std::string::npos)
        {
            try
            {
                // Try to convert to int
                size_t pos = 0;
                int integer = std::stoi(value, &pos);
                if (pos != value.size()) 
                {
                    // invalid number
                    if (!handler->onError("Invalid integer")) return false;
                }
                else
                {
                    return handler->onInteger(integer);
                }
                
                // int v;
                // std::from_chars(sv.data(), sv.data() + sv.size(), v);
                // char* end;
                // long v = std::strtol(text.data(), &end, 10);
            }
            catch(const std::exception& e)
            {
                if (!handler->onError("Could not convert '" + std::string(value) + "' to an integer")) return false;
            }
        }
        else
        {
            try
            {
                // Try to convert to double
                return handler->onDouble(std::stod(value.data()));
            }
            catch(const std::exception& e)
            {
                if (!handler->onError("Could not convert '" + std::string(value) + "' to a double")) return false;
            }  
        }
    }
    
    if (!handler->onError("Something is not closed"))
    {
        return false;
    }
    // Try to continue step by step
    if (reader->eof()) return false;
    reader->get();
    return true;
}

bool parseJson(ICharReader* reader, JsonParseHandler* handler)
{
    bool success = parseJson(reader, handler, 0);
    parseWhiteSpace(reader);
    if (success && !reader->eof())
    {
        success = handler->onError("Json contains values after parsing completed");
    }

    return success;
}

};
