#ifndef BLUEMARBLE_CSVDATASET
#define BLUEMARBLE_CSVDATASET

#include "FileDataSet.h"

namespace BlueMarble
{
    class CsvFileDataSet : public AbstractFileDataSet
    {
        public:
            CsvFileDataSet(const std::string& filePath);
        protected:
            FeatureCollectionPtr read(const std::string& filePath) override final;
    };
}

#endif /* BLUEMARBLE_CSVDATASET */
