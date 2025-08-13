#ifndef BLUEMARBLE_LOGGING
#define BLUEMARBLE_LOGGING

#include <iostream>

namespace BlueMarble
{
    #define BMM_DEBUG() std::cout
    #define BMM_LOG() std::cout << __FILE__ << ":" << __LINE__ <<  ":" << __func__ << ": "
}

#endif /* BLUEMARBLE_LOGGING */
