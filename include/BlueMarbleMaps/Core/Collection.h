#ifndef BLUEMARBLE_COLLECTION
#define BLUEMARBLE_COLLECTION

#include <vector>

namespace BlueMarble
{

    // void add(FeaturePtr feature)

    // void remove(const Id& id)

    // const FeaturePtr& getFeature(const Id& id)

    // bool empty() const { return m_features.empty(); }
    // inline size_t size() const { return m_features.size(); }
    // inline auto begin() const { return m_features.begin(); }
    // inline auto end() const { return m_features.end(); }

    template<typename T, typename Alloc = std::allocator<T>>
    class Collection
    {
    public:
        inline void add(const T& obj) { m_collection.push_back(obj); }

        template<typename... Args>
        inline void emplace(Args&&... args) {
            m_collection.emplace_back(std::forward<Args>(args)...);
        }
        bool empty() const { return m_collection.empty(); }
        inline size_t size() const { return m_collection.size(); }
        inline void clear() { m_collection.clear(); }
        
        inline bool contains(const T& obj) const
        {
            for (const auto& o : m_collection)
            {
                if (o == obj)
                {
                    return true;
                }
            }

            return false;
        }

        inline void remove(size_t index) { auto it = m_collection.begin()+index; m_collection.erase(it); }
        inline const T& get(size_t index) const { return m_collection.at(index); }
        inline void reserve(size_t size) { m_collection.reserve(size); }
        inline auto begin() { return m_collection.begin(); }
        inline auto end() { return m_collection.end(); }
        inline auto begin() const { return m_collection.begin(); }
        inline auto end() const { return m_collection.end(); }

    private:
        std::vector<T, Alloc> m_collection;
    };
}

#endif /* BLUEMARBLE_COLLECTION */
