#ifndef BLUEMARBLE_MAPCONTROL
#define BLUEMARBLE_MAPCONTROL

#include "Map.h"

namespace BlueMarble
{
    class MapControl
    {
        public:
            MapControl();
            virtual ~MapControl() = default;
            
        private:
            MapPtr m_mapView;

    };
}

#endif /* BLUEMARBLE_MAPCONTROL */
