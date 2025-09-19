#ifndef SHAPEFILEDATASET
#define SHAPEFILEDATASET

#include "FileDataSet.h"

namespace BlueMarble
{
    class ShapeFileDataSet : public AbstractFileDataSet
    {
        public:
            ShapeFileDataSet(const std::string& filePath);
        protected:
            void read(const std::string& filePath) override final;
    };
}

#endif /* SHAPEFILEDATASET */
