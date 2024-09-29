#ifndef BLUEMARBLE_FILE
#define BLUEMARBLE_FILE

#include <iostream>
#include <vector>
#include <map>
#include <variant>

namespace BlueMarble
{
    
    class File
    {
        public:
            File(const std::string& filePath);
            const std::vector<std::string>& lines() const;
            std::string asString() const;
            static std::vector<std::string> readLines(const std::string& filePath);
            static void writeLines(const std::string& filePath, const std::vector<std::string>& lines);
        protected:
            std::string m_filePath;
            std::vector<std::string> m_lines;
    };

    class CSVFile : public File
    {
        public:
            CSVFile(const std::string& filePath, const std::string& delimiter = ",");

            const std::vector<std::vector<std::string>>& rows();

        private:
            void extractData();
            std::vector<std::vector<std::string>>   m_rows;
            std::string                             m_delimiter;

    };


    class JsonValue;
    typedef std::map<std::string, JsonValue*> JsonData;
    typedef std::vector<JsonValue*> JsonList;

    class JsonValue
    {
        public:
            //JsonValue(const JsonValue& other) : m_val(other.m_val) {} // TODO: delete this and use pointers instead.
            JsonValue(const std::variant<int, double, std::string, JsonList, JsonData>& val) : m_val(val) {}
            ~JsonValue()
            {
                if(isType<JsonList>())
                {
                    for (auto v : get<JsonList>())
                        delete v;
                }
                else if(isType<JsonData>())
                {
                    for (auto v : get<JsonData>())
                        delete v.second;
                }
            }


            JsonValue operator=(const JsonValue& jsonValue) = delete;

            template <typename T>
            const T& get() const 
            { 
                return std::get<T>(m_val); 
            }

            template <typename T>
            const T& tryGet() const 
            { 
                if (!isType<T>())
                {
                    std::cout << "Error trying to get(), actual type: " << typeAsString() << "\n";
                    return T();
                }
                return std::get<T>(m_val); 
            }

            template <typename T>
            bool isType() const
            { 
                try
                {
                    get<T>();
                }
                catch(const std::exception& e)
                {
                    return false;
                }

                return true;
            }
            inline std::string typeAsString() const
            {
                switch(m_val.index())
                {
                    case 0:
                        return "int";
                    case 1:
                        return "double";
                    case 2:
                        return "string";
                    case 3:
                        return "JsonList";
                    case 4:
                        return "JasonData";
                    default:
                        std::cerr << "JasonValue::typeAsString() Unknown type\n";
                        throw std::exception();
                }
            }
        private:
            std::variant<int, double, std::string, JsonList, JsonData> m_val;
    };


    // TODO: currenlty there are memory leaks/not implemented correctly. Use pointers and make sure JsonValues are freed
    class JSONFile : public File
    {
        public:
            JSONFile(const std::string& filePath);
            ~JSONFile();
            JsonValue* data();
            static std::string prettyString();
            static void prettyString(JsonValue* jsonData, std::string& s, int indentation);
            std::string prettyKeys(JsonValue* jsonData);
            void prettyKeys(JsonValue* jsonData, std::string& s, int indentation);
        private:
            JsonValue* m_jsonData;

            JsonValue* parseJson(const std::string& text, int& idx, int level);
            std::pair<std::string, JsonValue*> retrieveKeyValuePair(const std::string& text, int& idx, int level);
            std::string parseKey(const std::string& text, int& idx);
            JsonValue* parseValue(const std::string& text, int& idx, int level);
            void parseWhiteSpace(const std::string& text, int& idx);

            void expect(const char& c, const std::string& text, int& idx);
            bool isEndOfValueChar(const char& c);
    };

}


#endif /* BLUEMARBLE_FILE */
