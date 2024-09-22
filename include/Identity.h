#ifndef BLUEMARBLE_IDENTITY
#define BLUEMARBLE_IDENTITY

namespace BlueMarble
{
    typedef uint64_t FeatureId;
    typedef uint64_t DataSetId;

    class Id
    {
        public:
            inline Id(DataSetId dId, FeatureId fId)
                : m_dataSetId(dId)
                , m_featureId(fId)
            {}

            inline DataSetId dataSetId() const { return m_dataSetId; }
            inline FeatureId featureId() const { return m_featureId; }

            inline bool operator==(const Id& other) const { return m_dataSetId == other.m_dataSetId && m_featureId == other.m_featureId; }
            inline bool operator!=(const Id& other) const { return !(*this == other); }

            inline bool operator<(const Id& other) const { return (m_dataSetId < other.m_dataSetId) ? true : m_featureId<other.m_featureId;  }
        
        private:
            DataSetId m_dataSetId;
            DataSetId m_featureId;
    };
}

#endif /* BLUEMARBLE_ */
