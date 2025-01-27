#include "Core/LabelOrganizer.h"

using namespace BlueMarble;

LabelOrganizer::LabelOrganizer()
{
}

void LabelOrganizer::organize(std::vector<FeaturePtr>& pointFeatures, std::vector<FeaturePtr>& sourceFeatures)
{
    std::vector<FeaturePtr> outPointFeatures;
    std::vector<FeaturePtr> outSourceFeatures;

    double MIN_DISTANCE = 75;
    //for (int i = 0; i<(int)pointFeatures.size(); i++)
    for (int i = pointFeatures.size()-1; i>=0; i--) // Go in reverse, seems like that works better?
    {
        auto& p1 = pointFeatures[i]->geometryAsPoint()->point();

        bool render = true;
        // for (int j=i+1; j<(int)pointFeatures.size(); j++)
        for (int j = i-1; j>=0; j--) // Go in reverse, seems like that works better?
        {
            auto& p2 = pointFeatures[j]->geometryAsPoint()->point();
            if ((p1-p2).length() < MIN_DISTANCE)
            {
                render = false;
                break;
            }
        }

        if (render)
        {
            // TODO: temporary. Add features as presentation objects somewhere else?
            outPointFeatures.push_back(pointFeatures[i]);
            outSourceFeatures.push_back(sourceFeatures[i]);
        }
    }

    if (pointFeatures.size() > 0 && outPointFeatures.size() == 0)
    {
        std::cout << "LabelOrganizer::organize() Resulted in 0 features to show, organization probably failed\n";
        std::cout << "Initial size: " << pointFeatures.size() << "\n";
    }

    pointFeatures = outPointFeatures;
    sourceFeatures = outSourceFeatures;
}
