#ifndef BLUEMARBLE_BLUEMARBLELAYOUT
#define BLUEMARBLE_BLUEMARBLELAYOUT

#include "BlueMarbleMaps/Core/Attributes.h"
#include "BlueMarbleMaps/Core/EngineObject.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/System/JsonFile.h"

#include <memory>
#include <string>
#include <map>

namespace BlueMarble
{
    // Forward declarations
    class LayoutParameters; typedef std::shared_ptr<LayoutParameters> LayoutParametersPtr;
    class BlueMarbleLayout; typedef std::shared_ptr<LayoutParameters> BlueMarbleLayoutPtr;

    class LayoutParameters
    {
        // public:
        //     LayoutParameters();
        //     void add(const std::string&, const std::string& value);
        //     void add(const std::string&, int64_t value);
        //     void add(const std::string&, double value);

        // private:
        //     Attributes m_parameters;
    };
    
    class MapViewParser : public JSONParseHandler
    {
        virtual void onInteger(int value) override final {}
        virtual void onDouble(double value) override final {}
        virtual void onString(const std::string& value) override final {}
        virtual void onNull() override final {}

        virtual void onStartList(const JsonList& value) override final {}
        virtual void onEndList(const JsonList& value) override final {}

        virtual void onStartObject(const JsonData& value) override final {}
        virtual void onEndObject(const JsonData& value) override final {}
        
        virtual void onKey(const std::string& key) override final {}
    };


    class BlueMarbleLayout : private JSONParseHandler
    {
        public:
            BlueMarbleLayout(const std::string& filePath);
            MapPtr getView() { return m_mapView; }; // Temporary
        private:
            EngineObjectPtr parseObject(JsonValue* jsonValue);

            void onInteger(int value) override final 
            {
                std::cout << "onInteger: " << value << "\n";
            }

            void onDouble(double value) override final 
            {
                std::cout << "onDouble: " << value << "\n";
            }

            void onString(const std::string& value) override final
            {
                std::cout << "onString: " << value << "\n";
            }

            void onNull() override final
            {
                std::cout << "onNull" << "\n";
            }

            void onStartList(const JsonList& value) override final 
            {
                std::cout << "onStartList: " << value.size() << "\n";
            }

            void onEndList(const JsonList& value) override final
            {
                std::cout << "onEndList" << "\n";
            }

            void onStartObject(const JsonData& value) override final
            {
                std::cout << "onStartObject: " << value.size() << "\n";
            }
            
            void onEndObject(const JsonData& value) override final
            {
                std::cout << "onEndObject: " << "\n";
            }
            
            void onKey(const std::string& key) override final
            {
                std::cout << "onKey: " << key << "\n";
            }

            MapPtr m_mapView;
            std::map<std::string, EngineObjectPtr> m_layoutObjects;
    };

}

#endif /* BLUEMARBLE_BLUEMARBLELAYOUT */
