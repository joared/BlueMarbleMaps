#ifndef JSONDETAILS
#define JSONDETAILS

#include <iostream>
#include <string>
#include <sstream>

namespace BlueMarble
{
namespace JsonDetails
{
    

template<typename ReaderType, typename HandlerType>
inline bool expect(const char &c, ReaderType* reader, HandlerType* handler)
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

template<typename ReaderType>
inline void parseWhiteSpace(ReaderType* reader)
{
    char v = reader->peek();
    while (v == ' ' || v == '\n' || v == '\r' || v == '\t')
    {
        reader->get();
        v = reader->peek();
    }
}

template<typename ReaderType, typename HandlerType>
inline bool parseKey(ReaderType* reader, HandlerType* handler)
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

template<typename ReaderType, typename HandlerType>
inline bool parseJson(ReaderType* reader, HandlerType* handler, int level)
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

template<typename ReaderType, typename HandlerType> 
bool parseJson(ReaderType* reader, HandlerType* handler)
{
    bool success = parseJson(reader, handler, 0);
    parseWhiteSpace(reader);
    if (success && !reader->eof())
    {
        success = handler->onError("Json contains values after parsing completed");
    }

    return success;
}

} // namespace JsonDetails
} // namespace BlueMarble

#endif /* JSONDETAILS */
