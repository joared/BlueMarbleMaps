#ifndef CIMGEVENTMANAGER
#define CIMGEVENTMANAGER

#include "EventHandler.h"
#include "PointerEvent.h"
#include "KeyEvent.h"
#include "Drawable.h"

#include <CImg.h>

#include <map>

namespace BlueMarble
{

class CImgEventManager 
    : public EventManager
{
    public:
        CImgEventManager(Drawable& drawable);
        bool captureEvents() override final;
        void wait();
        void wait(int durationMs);
    private:
        void reset();

        void getMousePos(ScreenPos& pos) const; // Window specific
        ModificationKey getModificationKeyMask() const; // Window specific
        // Mouse events
        MouseButton getMouseButton(); // Window specific
        
        bool mouseButtonChanged(MouseButton lastDown, MouseButton current);
        bool mousePosChanged(const ScreenPos& lastPos, const ScreenPos& currPos);
        void handlePosChanged(MouseButton lastDown, const ScreenPos& currPos);
        void handleMouseButtonChanged(MouseButton curr, const ScreenPos& currPos);
        bool detectDrag(const ScreenPos& startPos, const ScreenPos& currPos, int thresh);
        void captureMouseEvents();
        int getWheelDelta();
        void resetWheelDelta();
        void resize();
        
        // Key events
        void captureKeyEvents();// Window specific
        void handleKey(bool keyDownState, KeyButton key);

        cimg_library::CImgDisplay& m_disp;
        Drawable& m_drawable;

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