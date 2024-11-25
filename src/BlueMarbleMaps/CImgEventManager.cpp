#include "CImgEventManager.h"

using namespace BlueMarble;

CImgEventManager::CImgEventManager(Drawable &drawable)
    : EventManager()
    , m_disp(*static_cast<cimg_library::CImgDisplay*>(drawable.getDisplay()))
    , m_timeStampMs(0)
    , m_downTimeStampMs(0)
    , m_drawable(drawable)
{
    reset();
}

bool CImgEventManager::captureEvents()
{   
    // std::cout << "Locking display\n";
    // cimg_library::cimg_lock_display();
    m_eventDispatched = false;
    m_timeStampMs = getTimeStampMs();
    captureMouseEvents();
    captureKeyEvents();
    resize();
    // std::cout << "Unlocking display\n";
    // cimg_library::cimg_unlock_display();
    // std::cout << "Display unlocked\n";
    return m_eventDispatched;

}

void CImgEventManager::wait()
{
    if(!captureEvents())
    {
        m_disp.wait();
        captureEvents();
    }
    
    // Why does this not work well for mouse wheel events?
    // if (!display.is_event())
    // {
    //     display.wait();
    // }
    // eventManager.captureEvents();
    // map.update();
}

void CImgEventManager::wait(int durationMs)
{
    m_disp.wait(durationMs);
    captureEvents();
}

void CImgEventManager::reset()
{
    m_lastDown = MouseButtonNone;
    m_isDragging = false;
    m_downPos = ScreenPos();
    m_lastPos = ScreenPos();

    m_keyButtonDownMap[KeyButton::ArrowDown] = false;
    m_keyButtonDownMap[KeyButton::ArrowUp] = false;
    m_keyButtonDownMap[KeyButton::ArrowLeft] = false;
    m_keyButtonDownMap[KeyButton::ArrowRight] = false;
    m_keyButtonDownMap[KeyButton::Enter] = false;
    m_keyButtonDownMap[KeyButton::Space] = false;
    m_keyButtonDownMap[KeyButton::BackSpace] = false;
    m_keyButtonDownMap[KeyButton::One] = false;
    m_keyButtonDownMap[KeyButton::Two] = false;
    m_keyButtonDownMap[KeyButton::Three] = false;
    m_keyButtonDownMap[KeyButton::Four] = false;
    m_keyButtonDownMap[KeyButton::Five] = false;
    m_keyButtonDownMap[KeyButton::Six] = false;
    m_keyButtonDownMap[KeyButton::Seven] = false;
    m_keyButtonDownMap[KeyButton::Eight] = false;
    m_keyButtonDownMap[KeyButton::Nine] = false;
    m_keyButtonDownMap[KeyButton::Add] = false;
    m_keyButtonDownMap[KeyButton::Subtract] = false;
}

void BlueMarble::CImgEventManager::getMousePos(ScreenPos &pos) const
{
    pos.x = m_disp.mouse_x();
    pos.y = m_disp.mouse_y();
}

ModificationKey BlueMarble::CImgEventManager::getModificationKeyMask() const
{
    ModificationKey keyMask = ModificationKeyNone;
    if (m_disp.is_keySHIFTLEFT() || m_disp.is_keySHIFTRIGHT())
    {
        keyMask = keyMask | ModificationKeyShift;
    }

    if (m_disp.is_keyCTRLLEFT() || m_disp.is_keyCTRLRIGHT())
    {
        keyMask = keyMask | ModificationKeyCtrl;
    }

    if (m_disp.is_keyALT())
    {
        keyMask = keyMask | ModificationKeyAlt;
    }

    return keyMask;
}

MouseButton CImgEventManager::getMouseButton()
{
    unsigned int button = m_disp.button();
    MouseButton mouseButton = MouseButtonNone;
    if (button & 0x1)
    {
        // Left mouse button
        mouseButton = MouseButtonLeft;
    }
    else if (button & 0x2)
    {
        // Right mouse button
        mouseButton = MouseButtonRight;
    }
    else if (button & 0x4)
    {
        // Middle mouse button
        mouseButton = MouseButtonMiddle;
    }

    return mouseButton;
}

bool CImgEventManager::mouseButtonChanged(MouseButton lastDown, MouseButton current)
{
    return lastDown != current;
}

bool CImgEventManager::mousePosChanged(const ScreenPos &lastPos, const ScreenPos &currPos)
{
    return !(currPos.x == -1 || currPos.y == -1) && (lastPos.x != currPos.x || lastPos.y != currPos.y);
}

void CImgEventManager::handlePosChanged(MouseButton lastDown, const ScreenPos &currPos)
{
    ModificationKey modKey = getModificationKeyMask();
    if (!m_isDragging)
    {
        if (lastDown != MouseButtonNone && detectDrag(m_downPos, currPos, m_configration.dragThresh))
        {
            // Drag begin, use only down pos
            DragBeginEvent beginEvent;
            beginEvent.startPos = m_downPos;
            beginEvent.lastPos = m_downPos;
            beginEvent.pos = m_downPos;
            beginEvent.modificationKey = modKey;
            beginEvent.mouseButton = lastDown;
            dispatchEvent(beginEvent, m_previousMouseDownEvent.timeStampMs); // Use the timestamp of the mouse down event

            // Drag event directly after, using current pos
            DragEvent event;
            event.startPos = m_downPos;
            event.lastPos = m_downPos;
            event.pos = currPos;
            event.modificationKey = modKey;
            event.mouseButton = m_lastDown;
            dispatchEvent(event, m_timeStampMs);

            m_isDragging = true;
        }
        else 
        {
            // Mouse move
            MouseMoveEvent event;
            event.pos = currPos;
            event.modificationKey = modKey;
            event.mouseButton = lastDown;
            dispatchEvent(event, m_timeStampMs);
        }
    }
    else
    {
        // Drag
        DragEvent event;
        event.startPos = m_downPos;
        event.lastPos = m_lastPos;
        event.pos = currPos;
        event.modificationKey = modKey;
        event.mouseButton = lastDown;     // We use the last down button
        dispatchEvent(event, m_timeStampMs);
    }
}

void CImgEventManager::handleMouseButtonChanged(MouseButton currButton, const ScreenPos &currPos)
{
    ModificationKey modKey = getModificationKeyMask();
    if (m_lastDown == MouseButtonNone)
    {
        m_downPos = currPos; // Store down position for drag detection
        m_lastDown = currButton; // Store last down button for mouse up

        if (m_previousMouseDownEvent.mouseButton == currButton                                                  // Same button (or any butt none?)
            && m_timeStampMs - m_previousMouseDownEvent.timeStampMs < m_configration.doubleClickTimeoutMs    // No timeout
            && !detectDrag(m_previousMouseDownEvent.pos, currPos, m_configration.doubleClickThresh))            // Within threshold
        {
            // Double click
            // TODO: Double click drag
            DoubleClickEvent clickEvent;
            clickEvent.pos = currPos;
            clickEvent.modificationKey = modKey;
            clickEvent.mouseButton = currButton;
            dispatchEvent(clickEvent, m_timeStampMs);

            m_previousMouseDownEvent = MouseDownEvent(); // Reset such that no click event occurs in next mouse down
        }
        else
        {
            // Mouse down
            MouseDownEvent event;
            event.pos = currPos;
            event.modificationKey = modKey;
            event.mouseButton = currButton;
            dispatchEvent(event, m_timeStampMs);

            m_previousMouseDownEvent = event; //TODO: Store the mouse down event for double click
        }
    }
    else if (currButton == MouseButtonNone)
    {
        if (!m_isDragging)
        {
            // Mouse up TOOD: should we dispatch mouse up?
            MouseUpEvent event;
            event.pos = currPos;
            event.modificationKey = modKey;
            event.mouseButton = m_lastDown; // We use the last down button
            dispatchEvent(event, m_timeStampMs);

            // Also dispatch click if we have a previous mouse down event (not double click)
            if (m_previousMouseDownEvent.mouseButton != MouseButtonNone)
            {
                ClickEvent clickEvent;
                clickEvent.pos = currPos;
                clickEvent.modificationKey = modKey;
                clickEvent.mouseButton = m_lastDown; // We use the last down button
                dispatchEvent(clickEvent, m_timeStampMs);
            }
        }
        else
        {
            // Drag end
            DragEndEvent event;
            event.startPos = m_downPos;
            event.lastPos = m_lastPos;
            event.pos = currPos;
            event.modificationKey = modKey;
            event.mouseButton = m_lastDown; // We use the last down button
            dispatchEvent(event, m_timeStampMs);

            m_isDragging = false; // Reset
        }
        m_lastDown = MouseButtonNone; // Reset
    }
    else 
    {
        // A mouse button was pressed while another was already down.
        // Ignore for now.
    }
}

bool CImgEventManager::detectDrag(const ScreenPos &startPos, const ScreenPos &currPos, int thresh)
{
    return abs(startPos.x - currPos.x) > thresh || abs(startPos.y - currPos.y) > thresh;
}

void CImgEventManager::captureMouseEvents()
{
    MouseButton mouseButton = getMouseButton();
    bool mButtonChanged = mouseButtonChanged(m_lastDown, mouseButton);
    
    ScreenPos currScreenPos;
    getMousePos(currScreenPos);
    bool posChanged = mousePosChanged(m_lastPos, currScreenPos);

    // We need to handle button changes first, since CImg
    // stores mouse positions of button press/release events.
    // This can cause erronous mouse position updates if position
    // changes are handled first.
    // bool first = m_lastDown != MouseButtonNone;
    if (mButtonChanged)
    {
        handleMouseButtonChanged(mouseButton, currScreenPos);
    }

    // Handle move event first
    if (posChanged)
    {
        handlePosChanged(m_lastDown, currScreenPos);
        m_lastPos = currScreenPos;
    }

    // if (mButtonChanged && !first)
    // {
    //     handleMouseButtonChanged(mouseButton, currScreenPos);
    // }


    // Mouse wheel
    if (int wheelDelta = getWheelDelta())
    {
        MouseWheelEvent event;
        event.mouseButton = mouseButton;
        event.pos = currScreenPos;
        event.delta = wheelDelta;
        dispatchEvent(event, m_timeStampMs);

        resetWheelDelta(); // Reset the wheel counter.
    }
}

int CImgEventManager::getWheelDelta()
{
    return m_disp.wheel();
}

void CImgEventManager::resetWheelDelta()
{
    m_disp.set_wheel();
}

void CImgEventManager::captureKeyEvents()
{
    handleKey(m_disp.is_keyARROWDOWN(), KeyButton::ArrowDown);
    handleKey(m_disp.is_keyARROWUP(), KeyButton::ArrowUp);
    handleKey(m_disp.is_keyARROWLEFT(), KeyButton::ArrowLeft);
    handleKey(m_disp.is_keyARROWRIGHT(), KeyButton::ArrowRight);
    handleKey(m_disp.is_keyENTER(), KeyButton::Enter);
    handleKey(m_disp.is_keySPACE(), KeyButton::Space);
    handleKey(m_disp.is_keyBACKSPACE(), KeyButton::BackSpace);
    handleKey(m_disp.is_key1(), KeyButton::One);
    handleKey(m_disp.is_key2(), KeyButton::Two);
    handleKey(m_disp.is_key3(), KeyButton::Three);
    handleKey(m_disp.is_key4(), KeyButton::Four);
    handleKey(m_disp.is_key5(), KeyButton::Five);
    handleKey(m_disp.is_key6(), KeyButton::Six);
    handleKey(m_disp.is_key7(), KeyButton::Seven);
    handleKey(m_disp.is_key8(), KeyButton::Eight);
    handleKey(m_disp.is_key9(), KeyButton::Nine);
    handleKey(m_disp.is_keyPADADD(), KeyButton::Add);
    handleKey(m_disp.is_keyPADSUB(), KeyButton::Subtract);
}

void CImgEventManager::handleKey(bool keyIsDown, KeyButton key)
{
    if (keyIsDown == m_keyButtonDownMap[key])
        return;

    m_keyButtonDownMap[key] = keyIsDown;
    ScreenPos pos;
    getMousePos(pos);
    ModificationKey modKey = getModificationKeyMask();
    if (keyIsDown)
    {
        // Key down
        KeyDownEvent event;
        event.pos = pos;
        event.modificationKey = modKey;
        event.keyButton = key;
        dispatchEvent(event, m_timeStampMs);
    }
    else
    {
        // Key up
        KeyUpEvent event;
        event.pos = pos;
        event.modificationKey = modKey;
        event.keyButton = key;
        dispatchEvent(event, m_timeStampMs);
    }
}

void CImgEventManager::resize()
    {
        if (m_disp.is_resized() || m_disp.is_keyF11())
        {
            m_disp.resize(m_disp.window_width(), m_disp.window_height());
            std::cout << "Resize: " << m_disp.window_width() << ", " << m_disp.window_height() << "\n";
            //map.resize(display.window_width(), display.window_height());
            m_drawable.resize(m_disp.window_width(), m_disp.window_height());

            ResizeEvent event;
            event.width = m_disp.window_width();
            event.height = m_disp.window_height();
            dispatchEvent(event, m_timeStampMs);
        }
    }
