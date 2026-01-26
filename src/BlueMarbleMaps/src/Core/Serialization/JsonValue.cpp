#include "BlueMarbleMaps/Core/Serialization/JsonValue.h"
#include <cassert>

namespace BlueMarble
{

JsonValue JsonValue::fromStream(std::istream &ss, JsonParseHandler *handler)
{
    StreamReader reader(ss);
    try
    {
        return parseJson(&reader, 0, handler);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';

        return JsonValue();
    }
}

JsonValue JsonValue::fromString(const std::string &str)
{
    StringReader reader(str);
    try
    {
        return parseJson(&reader, 0);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';

        return JsonValue();
    }
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

void throwParseError(ICharReader* reader, const std::string& error)
{
    constexpr int errLength = 40;
    auto e = error + ":\n";
    //e += text.substr(idx-errLength, errLength+1) + "<---\n";
    throw std::runtime_error(e);
}

void expect(const char &c, ICharReader* reader)
{
    if (c != reader->peek())
    {
        
        std::string err = "Expected '" + std::string(1, c) + "' but got '" + reader->peek();
        throwParseError(reader, err);
    }
}

bool isEndOfValueChar(const char &c)
{
    // Used for int/double parsing
    return c == ',' || c == '}' || c == ']';
}

void parseWhiteSpace(ICharReader* reader)
{
    char v = reader->peek();
    while (v == ' ' || v == '\n' || v == '\r' || v == '\t')
    {
        reader->get();
        v = reader->peek();
    }
}

std::string parseKey(ICharReader* reader)
{
    expect('\"', reader);
    reader->get();

    std::string key;
    while (reader->peek() != '\"')
    {
        key += reader->peek();
        reader->get();
    }
    reader->get();

    assert(!key.empty());

    return key;
}

std::pair<std::string, JsonValue> retrieveKeyValuePair(ICharReader* reader, int level)
{
    auto key = std::move(parseKey(reader));
    parseWhiteSpace(reader);
    expect(':', reader);
    reader->get();
    parseWhiteSpace(reader);

    return { std::move(key), std::move(parseJson(reader, level)) };
}

JsonValue parseJson(ICharReader* reader, int level, JsonParseHandler* handler)
{
    parseWhiteSpace(reader);

    if (reader->peek() == ',')
    {
        std::cout << "parseJson() WARNING: Trailing comma... skipping over...\n"; reader->get();
        parseWhiteSpace(reader);
    }

    if (reader->peek() == '{')
    {
        
        // Object
        expect('{', reader);
        reader->get();
        JsonValue::Object jsonObject;
        do
        {
            parseWhiteSpace(reader);
            jsonObject.emplace(std::move(retrieveKeyValuePair(reader, level)));
            parseWhiteSpace(reader);
            // FIXME: this is needed if comma is forgotten, but parseValue should not step over ','
            if (reader->peek() != '}')
            {
                expect(',', reader);
                reader->get();
                parseWhiteSpace(reader);
            }
        } 
        while (reader->peek() != '}');

        reader->get(); // after '}'

        return jsonObject;
    }
    else if (reader->peek() == '[')
    {
        // Array
        auto list = JsonValue::Array();
        int nList = 1;
        reader->get();
        do
        {
            if (reader->peek() == ']')
            {
                // List done
                nList--;
                reader->get();
                assert(nList == 0);
            }
            else
            {
                // parse element value ',' or '['
                parseWhiteSpace(reader);
                if (reader->peek() != ']')
                    list.emplace_back(std::move(parseJson(reader, level)));
                parseWhiteSpace(reader);
                if (reader->peek() != ']')
                {
                    expect(',', reader);
                    reader->get();
                    parseWhiteSpace(reader);
                }
            }

        } while (nList > 0);

        return list;
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
                        throwParseError(reader, "Invalid escape sequence");
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
            throwParseError(reader, "Expected string but parsed empty value");
        }
        reader->get(); // after last '"'

        return value;
    }
    else if (reader->peek() == 'n')
    {
        // Null
        reader->get(); expect('u', reader);
        reader->get(); expect('l', reader);
        reader->get(); expect('l', reader);
        reader->get();
        
        return JsonValue();
    }
    else if (reader->peek() == 't' || reader->peek() == 'f')
    {
        // Boolean
        bool value;
        if (reader->peek() == 't')
        {
            reader->get(); expect('r', reader);
            reader->get(); expect('u', reader);
            reader->get(); expect('e', reader);
            
            value = true;
        }
        else
        {
            reader->get(); expect('a', reader);
            reader->get(); expect('l', reader);
            reader->get(); expect('s', reader);
            reader->get(); expect('e', reader);

            value = false;
        }
        reader->get();

        return value;
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
            throwParseError(reader, "Expected double/int but parsed empty value");
        }
        
        if (value.find('.') == std::string::npos)
        {
            try
            {
                // Try to convert to int
                return std::stoi(value.data());
                // int v;
                // std::from_chars(sv.data(), sv.data() + sv.size(), v);
                // char* end;
                // long v = std::strtol(text.data(), &end, 10);
            }
            catch(const std::exception& e)
            {
                throwParseError(reader, "Could not convert '" + std::string(value) + "' to an integer");
            }
        }
        else
        {
            try
            {
                // Try to convert to double
                return std::stod(value.data());
            }
            catch(const std::exception& e)
            {
                throwParseError(reader, "Could not convert '" + std::string(value) + "' to a double");
            }  
        }
    }
    
    
    throw std::runtime_error("parseJson() FAILED");
}

};
