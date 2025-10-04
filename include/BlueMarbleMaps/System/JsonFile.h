// #ifndef BLUEMARBLE_JSONFILE
// #define BLUEMARBLE_JSONFILE

// #include "BlueMarbleMaps/System/File.h"

// #include <vector>
// #include <map>
// #include <variant>

// namespace BlueMarble
// {

//     // class JJsonValue; // forward declare

//     // using JsonObject = std::map<std::string, JJsonValue>;   // objects
//     // using JsonArray = std::vector<JJsonValue>;             // arrays

//     // class JJsonValue 
//     // {
//     // public:
//     //     using VariantType = std::variant<bool, int, double, std::string, JsonArray, JsonObject>;

//     //     JJsonValue() = default;
//     //     JJsonValue(int v) : m_val(v) {}
//     //     JJsonValue(double v) : m_val(v) {}
//     //     JJsonValue(const std::string& v) : m_val(v) {}
//     //     JJsonValue(const char* v) : m_val(std::string(v)) {}
//     //     JJsonValue(const JsonArray& v) : m_val(v) {}
//     //     JJsonValue(const JsonObject& v) : m_val(v) {}

//     //     // JJsonValue(std::initializer_list<JJsonValue> init)
//     //     //         : m_val(JsonArray(init)) {}
//     //     JJsonValue(std::initializer_list<std::pair<const std::string, JJsonValue>> init)
//     //             : m_val(JsonObject(init)) {}

//     //     static JsonArray array(std::initializer_list<JJsonValue> init = {})
//     //     {
//     //         return JsonArray(init);
//     //     }

//     //     // template<typename T,
//     //     //      typename = std::enable_if_t<(std::is_same_v<T>>
//     //     // JsonValue(T v) : m_val(std::move(v)) {}

//     //     template <typename T>
//     //     bool isType() const
//     //     { 
//     //         return std::holds_alternative<T>(m_val);
//     //     }

//     //     template <typename T>
//     //     const T& get() const 
//     //     { 
//     //         return std::get<T>(m_val); 
//     //     }

//     //     template <typename T>
//     //     T& get() 
//     //     { 
//     //         return std::get<T>(m_val); 
//     //     }

//     //     const VariantType& value() const { return m_val; }
//     //     VariantType& value() { return m_val; }

//     // private:
//     //     VariantType m_val;
//     // };


//     class JsonValue;
//     typedef std::map<std::string, JsonValue*> JsonData;
//     typedef std::vector<JsonValue*> JsonList;

//     class JsonValue
//     {
//         public:
//             //JsonValue(const JsonValue& other) : m_val(other.m_val) {} // TODO: delete this and use pointers instead.
//             JsonValue(const std::variant<int, double, std::string, JsonList, JsonData>& val) : m_val(val) {}
//             ~JsonValue()
//             {
//                 if(isType<JsonList>())
//                 {
//                     for (auto v : get<JsonList>())
//                         delete v;
//                 }
//                 else if(isType<JsonData>())
//                 {
//                     for (auto v : get<JsonData>())
//                         delete v.second;
//                 }
//             }


//             JsonValue operator=(const JsonValue& jsonValue) = delete;

//             template <typename T>
//             const T& get() const 
//             { 
//                 return std::get<T>(m_val); 
//             }

//             template <typename T>
//             T& get() 
//             { 
//                 return std::get<T>(m_val); 
//             }

//             template <typename T>
//             const T& tryGet() const 
//             { 
//                 if (!isType<T>())
//                 {
//                     std::cout << "Error trying to get(), actual type: " << typeAsString() << "\n";
//                     return T();
//                 }
//                 return std::get<T>(m_val); 
//             }

//             template <typename T>
//             bool isType() const
//             { 
//                 try
//                 {
//                     get<T>();
//                 }
//                 catch(const std::exception& e)
//                 {
//                     return false;
//                 }

//                 return true;
//             }
//             inline std::string typeAsString() const
//             {
//                 switch(m_val.index())
//                 {
//                     case 0:
//                         return "int";
//                     case 1:
//                         return "double";
//                     case 2:
//                         return "string";
//                     case 3:
//                         return "JsonList";
//                     case 4:
//                         return "JsonData";
//                     default:
//                         std::cerr << "JasonValue::typeAsString() Unknown type\n";
//                         throw std::exception();
//                 }
//             }
//         private:
//             std::variant<int, double, std::string, JsonList, JsonData> m_val;
//     };


//     class JSONParseHandler
//     {
//         public:
//             virtual void onInteger(int value) {}
//             virtual void onDouble(double value) {}
//             virtual void onString(const std::string& value) {}
//             virtual void onNull() {}

//             virtual void onStartList(const JsonList& value) {}
//             virtual void onEndList(const JsonList& value) {}

//             virtual void onStartObject(const JsonData& value) {}
//             virtual void onEndObject(const JsonData& value) {}
            
//             virtual void onKey(const std::string& key) {}
//     };

//     class JSONFile : public File
//     {
//         public:
//             JSONFile(const std::string& filePath);
//             ~JSONFile();
            
//             JsonValue* data();
//             static JsonValue* fromString(const std::string& str);
//             static void parseData(JsonValue* data, JSONParseHandler* parseHandler);
//             static std::string serializeData(JsonValue* data);
//             static std::string prettyString(JsonValue* value, bool useIndentation=false);
//             static void prettyString(JsonValue* jsonData, std::string& s, int indentation=0, bool useIndentation=false);
//             std::string prettyKeys(JsonValue* jsonData);
//             void prettyKeys(JsonValue* jsonData, std::string& s, int indentation);
//         private:
//             static JsonValue* parseJson(const std::string& text, int& idx, int level);
//             static std::pair<std::string, JsonValue*> retrieveKeyValuePair(const std::string& text, int& idx, int level);
//             static std::string parseKey(const std::string& text, int& idx);
//             static JsonValue* parseValue(const std::string& text, int& idx, int level);
//             static void parseWhiteSpace(const std::string& text, int& idx);

//             static void expect(const char& c, const std::string& text, int& idx);
//             static bool isEndOfValueChar(const char& c);

//             JsonValue*          m_jsonData;
//             JSONParseHandler*   m_parseHandler;
//     };

// } // namespace BlueMarble


// #endif /* BLUEMARBLE_JSONFILE */
