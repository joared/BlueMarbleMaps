#ifndef BLUEMARBLE_EVENT
#define BLUEMARBLE_EVENT

#include "BlueMarbleMaps/Core/Core.h"
#include <functional>
#include <iostream>
#include <string>

namespace BlueMarble 
{
	#define EVENT_CALLBACK(eventType) virtual bool On##eventType(const eventType##Event& /*event*/) { return false; }
	#define ENUM_ELEMENT(element) element,
	#define FORWARD_DECLARE_EVENT(event) class event##Event;

	#define DECLARE_EVENTS(...) 		\
	enum class EventType				\
	{									\
		__VA_ARGS__(ENUM_ELEMENT)		\
		Invalid							\
	};									\
										\
	__VA_ARGS__(FORWARD_DECLARE_EVENT)	\
	FORWARD_DECLARE_EVENT(Invalid)		\
										\
	class EventCallbacks				\
	{									\
		public:							\
			__VA_ARGS__(EVENT_CALLBACK)	\
			EVENT_CALLBACK(Invalid)		\
	};									\

	#define EVENT_LIST(X)				\
		X(Timer)						\
		X(Resize)						\
		X(KeyDown)						\
		X(KeyUp)						\
		X(MouseEnter)					\
		X(MouseLeave)					\
		X(MouseDown)					\
		X(MouseUp)						\
		X(MouseMove)					\
		X(MouseWheel) 					\
		X(Click)						\
		X(DoubleClick)					\
		X(Hover)						\
		X(DragBegin)					\
		X(Drag)							\
		X(DragEnd)						\

	DECLARE_EVENTS(EVENT_LIST)

	#define DEFINE_EVENT(eventType) virtual EventType getType() const override { return EventType::eventType; }\
									virtual std::string toString() const override { return #eventType; }\
									virtual bool dispatch(EventCallbacks* handler) const override { return handler->On##eventType(*this); }\

	class Event
	{
	public:
		bool stopPropagation = false;
		int timeStampMs = 0;
		virtual EventType getType() const = 0;
		virtual std::string toString() const = 0;
		virtual bool dispatch(EventCallbacks* eventHandler) const = 0;
	};
}

#endif /* BLUEMARBLE_EVENT */
