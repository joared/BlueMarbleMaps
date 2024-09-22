#ifndef BLUEMARBLE_ENGINEOBJECT
#define BLUEMARBLE_ENGINEOBJECT

#include <string>
#include <vector>

namespace BlueMarble
{
    class EngineObject
    {
        public:
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
        protected:
            virtual void onChildAdded(EngineObject* child) {}; // TODO: should be pure virtual
        private:
            std::string m_name;
            std::vector<EngineObject*> m_children;
    };
}

#endif /* BLUEMARBLE_ENGINEOBJECT */
