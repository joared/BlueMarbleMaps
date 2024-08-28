#ifndef BLUEMARBLE_BUTTONS
#define BLUEMARBLE_BUTTONS

#include <string>

namespace BlueMarble
{
    enum MouseButton
	{
		MouseButtonNone = 0,
		MouseButtonLeft = BIT(0),
		MouseButtonRight = BIT(1),
		MouseButtonMiddle = BIT(2),
	};

	enum ModificationKey : int
	{
		ModificationKeyNone = 0,
		ModificationKeyShift = BIT(0),
		ModificationKeyCtrl = BIT(1),
		ModificationKeyAlt = BIT(2)
	};

	inline ModificationKey operator|(ModificationKey a, ModificationKey b)
	{
		return static_cast<ModificationKey>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline std::string mouseButtonToString(MouseButton button)
	{
		std::string str = "ErrorButton";
		switch (button)
		{
		case BlueMarble::MouseButtonNone:
			str = "None";
			break;
		case BlueMarble::MouseButtonLeft:
			str = "Left";
			break;
		case BlueMarble::MouseButtonRight:
			str = "Right";
			break;
		case BlueMarble::MouseButtonMiddle:
			str = "Middle";
			break;
		
		default:
			break;
		}

		return str;
	}

	enum class KeyButton
    {
        ArrowLeft,
        ArrowRight,
        ArrowUp,
        ArrowDown,
		Enter,
		Space,
		BackSpace,
		One,Two,Three,Four,Five,Six,Seven,Eight,Nine
    };

	// Helper function to determine if a key is a number
	inline bool isNumberKey(KeyButton key)
	{
		int i = (int)key;
		return i >= (int)KeyButton::One && i <= (int)KeyButton::Nine;
	}

	inline int numberKeyToInt(KeyButton key)
	{
		assert(isNumberKey(key));
		return int(key) - (int)KeyButton::One + 1;
	}

    struct ScreenPos
    {
        int x, y = 0;
    };

};

#endif /* BLUEMARBLE_BUTTONS */
