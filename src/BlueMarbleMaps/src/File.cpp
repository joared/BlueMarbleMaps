#include "File.h"
#include "Utils.h"

#include <fstream>


using namespace BlueMarble;

File::File(const std::string& filePath)
    : m_filePath(filePath)
    , m_lines(readLines(filePath))
{
    std::cout << "File()\n";
}

const std::vector<std::string> &BlueMarble::File::lines() const
{
    return m_lines;
}

std::string BlueMarble::File::asString() const
{
    std::string s;
    for (auto& l : lines())
    {
        s += l;
    }

    return s;
}

std::vector<std::string> File::readLines(const std::string& filePath)
{
    std::ifstream file(filePath);
    // Check if the file is successfully opened 
    if (!file.is_open()) 
    { 
        std::cerr << "File::readLines() Error opening the file!\n";
        return std::vector<std::string>();
    }

    std::vector<std::string> lines;
    std::string line;
    while (getline(file, line))
    {
        lines.push_back(line);
    }

    file.close();

    return lines;
}

void File::writeLines(const std::string& filePath, const std::vector<std::string>& lines)
{
    std::ofstream file(filePath);
    // Check if the file is successfully opened 
    if (!file.is_open()) 
    { 
        std::cerr << "File::writeLines() Error opening the file!\n";
    }

    for (auto l : lines)
    {
        file << l << "\n";
    }
    
    file.close();
}

CSVFile::CSVFile(const std::string& filePath, const std::string& delimiter)
    : File(filePath)
    , m_rows()
    , m_delimiter(delimiter)
{
    extractData();
}

const std::vector<std::vector<std::string>> &BlueMarble::CSVFile::rows()
{
    return m_rows;
}

void CSVFile::extractData()
{
    for (auto& line : lines())
    {
        auto row = Utils::splitString(line, m_delimiter);
        m_rows.push_back(row);
    }
}

JSONFile::JSONFile(const std::string& filePath)
    : File(filePath)
    , m_jsonData(nullptr)
{
    std::cout << "JSONFile(): " << filePath << "\n";
    int idx = 0;
    m_jsonData = parseJson(asString(), idx, 0);

    // auto jsonData = m_jsonData.get<JsonData>();

    // JsonValue* one = jsonData["one"];
    // auto two = jsonData["two"]->get<JsonData>();
    // JsonValue* three = two["three"];
    // auto four = two["four"]->get<JsonData>();
    // JsonValue* five = four["five"];
    // auto six = jsonData["six"]->get<JsonData>();
    // JsonValue* seven = six["seven"];

    // std::cout << "one: " << one->get<std::string>() << "\n";
    // std::cout << "three: " << three->get<std::string>() << "\n";
    // std::cout << "five: " << five->get<std::string>() << "\n";
    // std::cout << "seven: " << seven->get<std::string>() << "\n";
    
    // std::string s = "";
    // prettyString(m_jsonData, s, 0);
    // std::cout << "Json:\n";
    // std::cout << s;
    
}

JSONFile::~JSONFile()
{
    std::cout << "JSONFile::~JSONFile()\n";
    delete m_jsonData; // FIXME: this takes an insane amount of time
    std::cout << "JSONFile::~JSONFile() exit\n";
}

JsonValue *BlueMarble::JSONFile::data()
{
    return m_jsonData;
}

std::string BlueMarble::JSONFile::prettyString()
{
    //prettyString(&value);

    return "";
}


void BlueMarble::JSONFile::prettyString(JsonValue* value, std::string& s, int indentation)
{
    // std::cout << "prettyString";
    // std::string temp;
    // std::cin >> temp;    

    std::string indentationStr = "";
    for (int i(0); i<indentation; i++) { indentationStr += "  "; }

    if (value->isType<int>())
    {
        s += std::to_string(value->get<int>());
    }
    else if (value->isType<double>())
    {
        s += std::to_string(value->get<double>());
    }
    else if (value->isType<std::string>())
    {
        s += value->get<std::string>();
    }
    else if (value->isType<JsonList>())
    {
        s += "[";
        for (auto v : value->get<JsonList>())
        {
            prettyString(v, s, indentation);
            s += ",";
        }
        s += "]";
    }
    else if (value->isType<JsonData>())
    {
        s += "{\n";

        JsonData jsonData;
        try
        {
            jsonData = value->get<JsonData>();
        }
        catch (...)
        {
            std::cout << "Failed: " << (uint64_t)value << "\n";
        }

        for (auto& it : jsonData)
        {
            s += indentationStr;
            s += it.first + ":";
            prettyString(it.second, s, indentation+1);
            s += ",\n";
        }
        s += indentationStr + "}\n";
    }
    else
    {
        std::cout << "Unsupported type\n";
    }
}

std::string JSONFile::prettyKeys(JsonValue *jsonData)
{
    std::string s;
    prettyKeys(jsonData, s, 0);

    return s;
}

void JSONFile::prettyKeys(JsonValue *jsonData, std::string& s, int indentation)
{
    std::string indentationStr = "";
    for (int i(0); i<indentation; i++) { indentationStr += "  "; }

    
    try
    {
        if (jsonData->isType<JsonData>())
        {
            s += indentationStr + "{\n";
            for (auto& v : jsonData->get<JsonData>())
            {
                s += "  " + v.first + ": (<type>)\n";
                prettyKeys(v.second, s, indentation+1);
            }
            s += indentationStr + "}\n";
        }
    }
    catch (...)
    {
        std::cout << "Failed: " << (uint64_t)jsonData << "\n";

    }
}

JsonValue* JSONFile::parseJson(const std::string &text, int &idx, int level)
{
    //std::cout << "parseJson() enter: " << text.substr(0, idx+1) << "\n";
    parseWhiteSpace(text, idx);
    expect('{', text, idx);
    idx++;
    JsonData jsonData;

    do
    {
        parseWhiteSpace(text, idx);
        try
        {
            const auto [key, value] = retrieveKeyValuePair(text, idx, level);
            jsonData[key] = value;
            parseWhiteSpace(text, idx);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            while (text[idx] != '}')
            {
                idx++;
            }
            idx++;
            return new JsonValue(jsonData);
        }
        
    } 
    while (text[idx] != '}');
    
    //std::cout << "parseJson() exit: " << text.substr(0, idx+1) << "\n";

    idx++; // after '}'

    if (idx >= (int)text.size())
    {
        std::cout << "End of file, level: " << level << "\n";
    }

    
    return new JsonValue(jsonData);
}

std::pair<std::string, JsonValue*> JSONFile::retrieveKeyValuePair(const std::string &text, int &idx, int level)
{
    auto key = parseKey(text, idx);
    parseWhiteSpace(text, idx);
    expect(':', text, idx);
    idx++;
    parseWhiteSpace(text, idx);
    auto jsonValue = parseValue(text, idx, level);

    return std::make_pair(key, jsonValue);
}

std::string JSONFile::parseKey(const std::string &text, int &idx)
{
    expect('\"', text, idx);
    idx++;

    std::string key = "";
    while (text[idx] != '\"')
    {
        key += text[idx];
        idx++;
    }
    idx++;

    assert(!key.empty());

    return key;
}

JsonValue* JSONFile::parseValue(const std::string &text, int &idx, int level)
{
    JsonValue* jsonValue(nullptr);

    if (text[idx] == '{')
    {
        // JsonData
        jsonValue = parseJson(text, idx, level+1);
    }
    else if (text[idx] == '[')
    {
        // List
        auto list = JsonList();
        int nList = 1;
        idx++;
        do
        {
            if (text[idx] == ',')
            {
                std::cout << "Shouldn't happen...\n";
                throw std::exception();
            }
            else if (text[idx] == ']')
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
                auto val = parseValue(text, idx, level);
                list.push_back(val);
                parseWhiteSpace(text, idx);
            }

        } while (nList > 0);
        
        //std::cout << "List (" << list.size() << "): " << text.substr(start, end-start+1) << "\n";

        jsonValue = new JsonValue(list);
    }
    else if (text[idx] == '\"')
    {
        // String
        idx++;
        std::string value = "";
        while (text[idx] != '\"')
        {
            value += text[idx];
            idx++;
        }
        
        if (value.empty())
        {
            expect('e', text, idx);
        }
        assert(!value.empty());
        idx++; // after last '"'
        
        //std::cout << "Got actual string: " << value << "\n";

        jsonValue = new JsonValue(value);
    }
    else
    {
        // Double/int
        std::string value = "";
        while (!isEndOfValueChar(text[idx]))
        {
            value += text[idx];
            idx++;
        }
        if (value.empty())
        {
            expect('e', text, idx);
        }
        assert(!value.empty());

        if (value.find('.') == std::string::npos)
        {
            try
            {
                // Try to convert to int
                int integer = std::stoi(value);
                jsonValue = new JsonValue(integer);
            }
            catch(const std::exception& e)
            {
                // std::cout << "Could not convert '" << value << "' to an integer. Interpreting as string instead.\n";
                jsonValue = new JsonValue(value);
            }
        }
        else
        {
            try
            {
                // Try to convert to double
                double d = std::stod(value);
                jsonValue = new JsonValue(d);
            }
            catch(const std::exception& e)
            {
                std::cout << "Could not convert '" << value << "' to a double. Interpreting as string instead.\n";
                jsonValue = new JsonValue(value);
            }  
        }
    }
    
    parseWhiteSpace(text, idx);

    if (text[idx] == ',')
        idx++;

    return jsonValue;
}

void BlueMarble::JSONFile::parseWhiteSpace(const std::string &text, int &idx)
{
    std::string whiteSpace = "";
    while (text[idx] == ' ' || text[idx] == '\n' || text[idx] == '\r' || text[idx] == '\t')
    {
        whiteSpace += text[idx];
        idx++;
    }
    // std::cout << "white space: " << whiteSpace << "\n";
}

void BlueMarble::JSONFile::expect(const char &c, const std::string &text, int &idx)
{
    if (c != text[idx])
    {
        //std::cout << text.substr(idx-20, idx+1) << "<---\n";
        std::cout << "Expected '" << c << "' but got '" << text[idx] << "'\n";

        throw std::exception();
    }
}

bool BlueMarble::JSONFile::isEndOfValueChar(const char &c)
{
    return c == ',' || c == '}' || c == ']'; // TODO: it is probably valid to have ']' within a string?
}
