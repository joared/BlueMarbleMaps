#ifndef BLUEMARBLE_OTTOTOOL
#define BLUEMARBLE_OTTOTOOL

#include "BlueMarbleMaps/Core/Tools/Tool.h"

namespace BlueMarble
{
    class OttoTool : public Tool
    {
        public:
            OttoTool() {}
            bool isActive() override final { return false; };
        protected:
            MapPtr m_map;
    };
};

#endif /* BLUEMARBLE_OTTOTOOL */
