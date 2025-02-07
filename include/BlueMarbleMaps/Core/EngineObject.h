#ifndef ENGINEOBJECT
#define ENGINEOBJECT

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

/*#define BLUEMARBLE_OBJECT_PROPERTY(propertyName, propertyType) \
    private: \
        propertyType m_##propertyName; \
    public: \
        const propertyType& propertyName() { return m_##propertyName; } \  
        void propertyName(propertyType value) { m_##propertyName = value; } \
        */
namespace BlueMarble
{
    typedef int64_t BMID;
    
    class EngineObject; typedef std::shared_ptr<EngineObject> EngineObjectPtr;

    class EngineObject
    {
        public:
            // TODO: make methods pure virtual instead
            EngineObject();
            EngineObject(const std::string& name);
            const std::string& name();
            void name(const std::string& name);
            void addChild(EngineObject* child);
            EngineObject* findChild(const std::string& name, bool recursive=true);
            template<typename T>
            void findChildren(std::vector<T*>& foundObjects, bool recursive=true)
            {
                for (auto c : m_children)
                {
                    if (auto obj = dynamic_cast<T*>(c))
                        foundObjects.push_back(obj);
                }

                if (recursive)
                {
                    for (auto c : m_children)   
                        c->findChildren(foundObjects, true);
                }
            }
            BMID getID() const { return m_id; }
        protected:
            virtual void onChildAdded(EngineObject* child) {}; // TODO: should be pure virtual
        private:
            static BMID generateUUID();
            std::string m_name;
            std::vector<EngineObject*> m_children;
            BMID m_id;
            static int64_t newestID;
    };
}

#endif /* ENGINEOBJECT */
