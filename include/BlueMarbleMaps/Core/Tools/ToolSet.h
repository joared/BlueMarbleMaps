#ifndef BLUEMARBLE_TOOLSET
#define BLUEMARBLE_TOOLSET

#include "BlueMarbleMaps/Core/Tools/Tool.h"

#include <memory>
#include <vector>

namespace BlueMarble
{

// Abstract class for a tool
class ToolSet;
typedef std::shared_ptr<ToolSet> ToolSetPtr;
class ToolSet
    : public Tool
    , public EventDispatcher
{
    public:
        ToolSet();

        void onConnected(const MapControlPtr& control, const MapPtr& map) override final;
        void onDisconnected() override final;

        void addSubTool(const ToolPtr& tool);
        void removeSubTool(const ToolPtr& tool);
        bool isActive() override final;
    protected:
        virtual bool onEvent(const Event& event) override;
    private:
        std::vector<ToolPtr> m_subTools;
        ToolPtr              m_activeSubTool;
};
}


#endif /* BLUEMARBLE_TOOLSET */
