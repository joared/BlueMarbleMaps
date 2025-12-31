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

            inline bool operator<(const Id& other) const 
            { 
                if (m_dataSetId == other.m_dataSetId)
                {
                    return m_featureId<other.m_featureId;
                }
                
                return m_dataSetId < other.m_dataSetId;
            }

            inline std::string toString() const { return "id: " + std::to_string(m_dataSetId) + ", " + std::to_string(m_featureId); }
        private:
            DataSetId m_dataSetId;
            FeatureId m_featureId;
    };

    struct IdHash
    {
        std::size_t operator()(const BlueMarble::Id& id) const noexcept
        {
            return std::hash<uint64_t>{}(id.dataSetId()) ^
                (std::hash<uint64_t>{}(id.featureId()) << 1);
        }
    };
}

#endif /* BLUEMARBLE_ */
