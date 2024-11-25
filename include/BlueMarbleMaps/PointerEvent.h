#ifndef BLUEMARBLE_POINTEREVENT
#define BLUEMARBLE_POINTEREVENT

#include "Event.h"
#include "Buttons.h"

namespace BlueMarble
{
	class ResizeEvent : public Event
	{
		public:
			DEFINE_EVENT(Resize);
			int width;
			int height;
	};

	class PointerEvent : public Event
	{
		public:
			ScreenPos pos;
			ModificationKey modificationKey = ModificationKeyNone;
	};

	class MouseEvent : public PointerEvent
	{
		public:
			MouseButton mouseButton;
	};

	class MouseDownEvent : public MouseEvent
	{
		public:
			DEFINE_EVENT(MouseDown)
	};

	class MouseMoveEvent : public MouseEvent
	{
		public:
			DEFINE_EVENT(MouseMove)
	};

	class MouseUpEvent : public MouseEvent
	{
		public:
			DEFINE_EVENT(MouseUp)
	};

	class MouseWheelEvent : public MouseEvent
	{
		public:
			DEFINE_EVENT(MouseWheel);
			int delta = 0;
	};

	class ClickEvent : public MouseEvent
	{
		public:
			DEFINE_EVENT(Click);
	};

    class DoubleClickEvent : public MouseEvent
	{
		public:
			DEFINE_EVENT(DoubleClick);
	};

	class DragEvent : public MouseEvent
	{
		public:
			DEFINE_EVENT(Drag)
			ScreenPos startPos;
			ScreenPos lastPos;
	};

	class DragBeginEvent : public DragEvent
	{
		public:
			DEFINE_EVENT(DragBegin)
	};

	class DragEndEvent : public DragEvent
	{
		public:
			DEFINE_EVENT(DragEnd)
	};
}

#endif /* BLUEMARBLE_POINTTEREVENT */
