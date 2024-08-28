#ifndef BLUEMARBLE_IDENTITY
#define BLUEMARBLE_IDENTITY

namespace BlueMarble
{
    typedef uintptr_t FeatureId;
    typedef uintptr_t DataSetId;

    class Id
    {
        public:
            inline Id(DataSetId dId, FeatureId fId)
                : m_dataSetId(dId)
                , m_featureId(fId)
            {}

            inline DataSetId dataSetId() { return m_dataSetId; }
            inline FeatureId featureId() { return m_featureId; }

            inline bool operator==(const Id& other) const { return m_dataSetId == other.m_dataSetId && m_featureId == other.m_featureId; }
            inline bool operator!=(const Id& other) const { return !(*this == other); }
        
        private:
            DataSetId m_dataSetId;
            DataSetId m_featureId;
    };
}

#endif /* BLUEMARBLE_ */
