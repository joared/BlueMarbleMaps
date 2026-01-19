#ifndef BLUEMARBLE_IPERSISTABLE
#define BLUEMARBLE_IPERSISTABLE

#include <string>

namespace BlueMarble
{
    // Common (optional) interface for ISpatialIndex and IFeatureDatabase.
    // If implemented, the FeatureStore will call load/save with an instance of PersistanceContext.
    // The PersistanceContext tells "where" the IPersistable should be stored, not "how".
    // If additional storage implementations are requires, the PersistanceContext needs to be updated
    // with additional members, and the featurestore needs to be update to generate unique storage members.
    class IPersistable
    {
        public:
            struct PersistanceContext
            {
                std::string fileName; // File name to be used for File base persistance
            };

            virtual ~IPersistable() = default;

            virtual std::string persistanceId() const = 0;
            virtual bool load(const PersistanceContext& ctx) = 0;
            virtual void save(const PersistanceContext& ctx) const = 0;
    };
};

#endif /*BLUEMARBLE_IPERSISTABLE*/
