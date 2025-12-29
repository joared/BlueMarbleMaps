#ifndef BLUEMARBLE_CPPUTILS
#define BLUEMARBLE_CPPUTILS

namespace BlueMarble
{
    enum MouseButton : int
	{
		MouseButtonNone = 0,
		MouseButtonLeft = BIT(0),
		MouseButtonRight = BIT(1),
		MouseButtonMiddle = BIT(2),
	};

	inline MouseButton operator|(MouseButton a, MouseButton b)
	{
		return static_cast<MouseButton>(static_cast<int>(a) | static_cast<int>(b));
	}
}

#endif /* BLUEMARBLE_CPPUTILS */
