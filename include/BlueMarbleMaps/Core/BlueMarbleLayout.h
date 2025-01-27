#ifndef BLUEMARBLE_BLUEMARBLELAYOUT
#define BLUEMARBLE_BLUEMARBLELAYOUT

#include "Attributes.h"

#include <memory>
#include <string>

namespace BlueMarble
{
    // Forward declarations
    class LayoutParameters; typedef std::shared_ptr<LayoutParameters> LayoutParametersPtr;
    class BlueMarbleLayout; typedef std::shared_ptr<LayoutParameters> BlueMarbleLayoutPtr;

    class LayoutParameters
    {
        public:
            LayoutParameters();
            void add(const std::string&, const std::string& value);
            void add(const std::string&, int64_t value);
            void add(const std::string&, double value);

        private:
            Attributes m_parameters;
    };
    

    class BlueMarbleLayout
    {
        public:
            BlueMarbleLayout(const std::string& filePath);
            
        private:

    };

}

#endif /* BLUEMARBLE_BLUEMARBLELAYOUT */
