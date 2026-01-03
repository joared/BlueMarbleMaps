#ifndef BLUEMARBLE_TOOLSET
#define BLUEMARBLE_TOOLSET

#include "BlueMarbleMaps/Core/Tools/Tool.h"

#include <memory>
#include <vector>

namespace BlueMarble
{

// Abstract class for a tool
class Tool;
typedef std::shared_ptr<Tool> ToolPtr;
class Tool 
    : public EventHandler
    , public EventDispatcher
{
    public:
        Tool();
        virtual ~Tool() = default;

        void onConnectedToMapControl(const MapControlPtr& control, const MapPtr& map) 
        {
            onConnected(control, map);
            for (const auto& tool : m_subTools)
            {
                tool->onConnected(control, map);
            }
        }
        void onDisconnectedFromMapControl()
        {
            onDisconnected();
            for (const auto& tool : m_subTools)
            {
                tool->onDisconnected();
            }
        }

        void addSubTool(const ToolPtr& tool);
        void removeSubTool(const ToolPtr& tool);
        virtual bool isActive() = 0;
    protected:
        virtual void onConnected(const MapControlPtr& control, const MapPtr& map) = 0;
        virtual void onDisconnected() = 0;
        virtual bool onEvent(const Event& event) override;
    private:
        std::vector<ToolPtr> m_subTools;
        ToolPtr              m_activeSubTool;
};
}


#endif /* BLUEMARBLE_TOOLSET */
