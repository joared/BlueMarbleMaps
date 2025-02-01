#ifndef BLUEMARBLE_LABELORGANIZER
#define BLUEMARBLE_LABELORGANIZER

#include "BlueMarbleMaps/Core/Feature.h"

namespace BlueMarble
{

    struct LabelOrganizingSettings
    {
        public:
            double maxDist = 100;
    };

    class LabelOrganizer
    {
        public:
            LabelOrganizer();
            void organize(std::vector<FeaturePtr>& pointFeatures, std::vector<FeaturePtr>& sourceFeatures);
    };

}

#endif /* BLUEMARBLE_LABELORGANIZER */
