#include "BlueMarbleMaps/Core/Serialization/JsonValue.h"
#include <cassert>

namespace BlueMarble
{

JsonValue JsonValue::fromString(const std::string &str)
{
    int idx = 0;
    try
    {
        return parseJson(str, idx, 0);
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
        baseIndentation = "    ";
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

void throwParseError(const std::string &text, int &idx, const std::string& error)
{
    auto e = error + ":\n";
    e += text.substr(idx-20, 21) + "<---\n";
    throw std::runtime_error(e);
}

void expect(const char &c, const std::string &text, int &idx)
{
    if (c != text[idx])
    {
        
        std::string err = "Expected '" + std::string(1, c) + "' but got '" + text[idx];
        throwParseError(text, idx, err);
    }
}

bool isEndOfValueChar(const char &c)
{
    // Used for int/double parsing
    return c == ',' || c == '}' || c == ']';
}

void parseWhiteSpace(const std::string &text, int &idx)
{
    while (text[idx] == ' ' || text[idx] == '\n' || text[idx] == '\r' || text[idx] == '\t')
    {
        idx++;
    }
}

std::string parseKey(const std::string &text, int &idx)
{
    expect('\"', text, idx);
    idx++;

    std::string key;
    while (text[idx] != '\"')
    {
        key += text[idx];
        idx++;
    }
    idx++;

    assert(!key.empty());

    return key;
}

std::pair<std::string, JsonValue> retrieveKeyValuePair(const std::string& text, int& idx, int level)
{
    auto key = std::move(parseKey(text, idx));
    parseWhiteSpace(text, idx);
    expect(':', text, idx);
    idx++;
    parseWhiteSpace(text, idx);

    return { std::move(key), std::move(parseJson(text, idx, level)) };
}

JsonValue parseJson(const std::string &text, int &idx, int level)
{
    parseWhiteSpace(text, idx);

    if (text[idx] == ',')
    {
        std::cout << "parseJson() WARNING: Trailing comma... skipping over...\n"; ++idx;
        parseWhiteSpace(text, idx);
    }

    if (text[idx] == '{')
    {
        // Object
        expect('{', text, idx);
        idx++;
        JsonValue::Object jsonObject;

        do
        {
            parseWhiteSpace(text, idx);
            jsonObject.emplace(std::move(retrieveKeyValuePair(text, idx, level)));
            parseWhiteSpace(text, idx);
            // FIXME: this is needed if comma is forgotten, but parseValue should not step over ','
            if (text[idx] != '}')
            {
                expect(',', text, idx);
                ++idx;
                parseWhiteSpace(text, idx);
            }
        } 
        while (text[idx] != '}');

        idx++; // after '}'
        return jsonObject;
    }
    else if (text[idx] == '[')
    {
        // Array
        auto list = JsonValue::Array();
        int nList = 1;
        idx++;
        do
        {
            if (text[idx] == ']')
            {
                // List done
                nList--;
                idx++;
                assert(nList == 0);
            }
            else
            {
                // parse element value ',' or '['
                parseWhiteSpace(text, idx);
                list.emplace_back(std::move(parseJson(text, idx, level)));
                parseWhiteSpace(text, idx);
                if (text[idx] != ']')
                {
                    expect(',', text, idx);
                    ++idx;
                    parseWhiteSpace(text, idx);
                }
            }

        } while (nList > 0);

        return list;
    }
    else if (text[idx] == '\"')
    {
        // TODO: escape sequences
        // String
        idx++;
        std::string value;
        while (text[idx] != '\"')
        {
            // TODO: not working
            // value += text[idx];
            // idx++;
            char c = text[idx];

            if (c == '\\') 
            { // escape sequence
                ++idx;
                if (idx >= text.size()) break; // error: unexpected end

                char esc = text[idx];
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
                        throwParseError(text, idx, "Invalid escape sequence");
                }
            }
            else if (c == '"') 
            {
                ++idx; // closing quote
                break;
            }
            else 
            {
                value += c;
            }
            ++idx;
        }
        
        if (value.empty())
        {
            throwParseError(text, idx, "Expected string but parsed empty value");
        }
        idx++; // after last '"'

        return value;
    }
    else if (text[idx] == 'n')
    {
        // Null
        ++idx; expect('u', text, idx);
        ++idx; expect('l', text, idx);
        ++idx; expect('l', text, idx);
        ++idx;
        
        return JsonValue();
    }
    else if (text[idx] == 't' || text[idx] == 'f')
    {
        // Boolean
        bool value;
        if (text[idx] == 't')
        {
            ++idx; expect('r', text, idx);
            ++idx; expect('u', text, idx);
            ++idx; expect('e', text, idx);
            
            value = true;
        }
        else
        {
            ++idx; expect('a', text, idx);
            ++idx; expect('l', text, idx);
            ++idx; expect('s', text, idx);
            ++idx; expect('e', text, idx);

            value = false;
        }
        ++idx;

        return value;
    }
    else
    {
        // Double/int
        
        int start = idx;
        while (!isEndOfValueChar(text[idx]))
        {
            idx++;
        }
        if (start == idx)
        {
            throwParseError(text, idx, "Expected double/int but parsed empty value");
        }
        
        std::string_view value = text.substr(start, idx-start);
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
                throwParseError(text, idx, "Could not convert '" + std::string(value) + "' to an integer");
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
                throwParseError(text, idx, "Could not convert '" + std::string(value) + "' to a double");
            }  
        }
    }
    
    
    throw std::runtime_error("parseJson() FAILED");
}

};
