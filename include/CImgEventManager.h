#ifndef CIMGEVENTMANAGER
#define CIMGEVENTMANAGER

#include "EventHandler.h"
#include "PointerEvent.h"
#include "KeyEvent.h"

#include <CImg.h>

#include <map>

namespace BlueMarble
{

class CImgEventManager 
    : public EventManager
{
    public:
        CImgEventManager(cimg_library::CImgDisplay& disp);
        void handleOsMessage(const OSEvent& /*msg*/) override final {};
        bool captureEvents() override final;
        void wait();
    private:
        void reset();

        void getMousePos(ScreenPos& pos) const;
        ModificationKey getModificationKeyMask() const;
        // Mouse events
        MouseButton getMouseButton(unsigned int button);
        bool mouseButtonChanged(MouseButton lastDown, MouseButton current);
        bool mousePosChanged(const ScreenPos& lastPos, const ScreenPos& currPos);
        void handlePosChanged(MouseButton lastDown, const ScreenPos& currPos);
        void handleMouseButtonChanged(MouseButton curr, const ScreenPos& currPos);
        bool detectDrag(const ScreenPos& startPos, const ScreenPos& currPos, int thresh);
        void captureMouseEvents();

        // Key events
        void captureKeyEvents();
        void handleKey(bool currDownState, KeyButton key);

        cimg_library::CImgDisplay& m_disp;

        EventConfiguration m_configration;

        // For mouse events
        int m_timeStampMs;
        int m_downTimeStampMs;
        ScreenPos m_downPos;
        ScreenPos m_lastPos;
        MouseButton m_lastDown;
        MouseDownEvent m_previousMouseDownEvent;
        bool m_isDragging;

        // For key events
        std::map<KeyButton, bool> m_keyButtonDownMap;
};

}

#endif /* CIMGEVENTMANAGER */