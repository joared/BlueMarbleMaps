#pragma once
#include <string>

#define KEY_STROKES \
    X(INVALID, 0x00, "invalid character") \
    X(ESCAPE, 0x01, "Escape") \
    X(ONE, 0x02, "One") \
    X(TWO, 0x03, "Two") \
    X(THREE, 0x04, "Three") \
    X(FOUR, 0x05, "Four") \
    X(FIVE, 0x06, "Five") \
    X(SIX, 0x07, "Six") \
    X(SEVEN, 0x08, "Seven") \
    X(EIGHT, 0x09, "Eight") \
    X(NINE, 0x0A, "Nine") \
    X(ZERO, 0x0B, "Zero") \
    X(PLUS, 0x0C, "Plus") \
    X(ACUTE_ACCENT, 0x0D, "Acute Accent") \
    X(BACKSPACE, 0x0E, "Backspace") \
    X(TAB, 0x0F, "Tab") \
    X(Q, 0x10, "Q") \
    X(W, 0x11, "W") \
    X(E, 0x12, "E") \
    X(R, 0x13, "R") \
    X(T, 0x14, "T") \
    X(Y, 0x15, "Y") \
    X(U, 0x16, "U") \
    X(I, 0x17, "I") \
    X(O, 0x18, "O") \
    X(P, 0x19, "P") \
    X(SWED_AO, 0x1A, "Swedish AO") \
    X(GRAVE_ACCENT, 0x1B, "Grave Accent") \
    X(ENTER, 0x1C, "Enter") \
    X(LEFT_CONTROL, 0x1D, "Left Control") \
    X(A, 0x1E, "A") \
    X(S, 0x1F, "S") \
    X(D, 0x20, "D") \
    X(F, 0x21, "F") \
    X(G, 0x22, "G") \
    X(H, 0x23, "H") \
    X(J, 0x24, "J") \
    X(K, 0x25, "K") \
    X(L, 0x26, "L") \
    X(SWED_OE, 0x27, "Swedish OE") \
    X(SWED_AE, 0x28, "Swedish AE") \
    X(PARAGRAPHE, 0x29, "Paragraphe") \
    X(LEFT_SHIFT, 0x2A, "Left Shift") \
    X(APOSTROPHE, 0x2B, "Apostrophe") \
    X(Z, 0x2C, "Z") \
    X(X, 0x2D, "X") \
    X(C, 0x2E, "C") \
    X(V, 0x2F, "V") \
    X(B, 0x30, "B") \
    X(N, 0x31, "N") \
    X(M, 0x32, "M") \
    X(COMMA, 0x33, ",") \
    X(DOT, 0x34, ".") \
    X(DASH, 0x35, "-") \
    X(RIGHT_SHIFT, 0x36, "Right Shift") \
    X(NUM_STAR, 0x37, "Numpad Star") \
    X(LEFT_ALT, 0x38, "Left Alt") \
    X(SPACE, 0x39, "Space") \
    X(CAPS_LOCK, 0x3A, "Caps Lock") \
    X(F1, 0x3B, "F1") \
    X(F2, 0x3C, "F2") \
    X(F3, 0x3D, "F3") \
    X(F4, 0x3E, "F4") \
    X(F5, 0x3F, "F5") \
    X(F6, 0x40, "F6") \
    X(F7, 0x41, "F7") \
    X(F8, 0x42, "F8") \
    X(F9, 0x43, "F9") \
    X(F10, 0x44, "F10") \
    X(PAUSE, 0x45, "Pause") \
    X(SCROLL_LOCK, 0x46, "Scroll Lock") \
    X(NUM_SEVEN, 0x47, "Numpad Seven") \
    X(NUM_EIGHT, 0x48, "Numpad Eight") \
    X(NUM_NINE, 0x49, "Numpad Nine") \
    X(NUM_SUBTRACT, 0x4A, "Numpad Subtract") \
    X(NUM_FOUR, 0x4B, "Numpad Four") \
    X(NUM_FIVE, 0x4C, "Numpad Five") \
    X(NUM_SIX, 0x4D, "Numpad Six") \
    X(NUM_PLUS, 0x4E, "Numpad Plus") \
    X(NUM_1, 0x4F, "Numpad One") \
    X(NUM_2, 0x50, "Numpad Two") \
    X(NUM_3, 0x51, "Numpad Three") \
    X(NUM_0, 0x52, "Numpad Zero") \
    X(NUM_DELETE, 0x53, "Numpad Delete") \
    X(SYSTEM_REQUEST, 0x54, "System Request") \
    X(LESS_THAN, 0x56, "Less Than") \
    X(F11, 0x57, "F11") \
    X(F12, 0x58, "F12") \
    X(NUM_ENTER, 0x011C, "Numpad Enter") \
    X(RIGHT_CONTROL, 0x011D, "Right Control") \
    X(NUM_SLASH, 0x0135, "Numpad Slash") \
    X(RIGHT_ALT, 0x0138, "Right Alt") \
    X(NUM_LOCK, 0x0145, "Num Lock") \
    X(HOME, 0x0147, "Home") \
    X(UP_ARROW, 0x0148, "Up Arrow") \
    X(PAGE_UP, 0x0149, "Page Up") \
    X(LEFT_ARROW, 0x014B, "Left Arrow") \
    X(RIGHT_ARROW, 0x014D, "Right Arrow") \
    X(END, 0x014F, "End") \
    X(DOWN_ARROW, 0x0150, "Down Arrow") \
    X(PAGE_DOWN, 0x0151, "Page Down") \
    X(INSERT, 0x0152, "Insert") \
    X(DEL_KEY, 0x0153, "Delete") \
    X(LEFT_WINDOWS, 0x015B, "Left Windows") \
    X(RIGHT_WINDOW, 0x015C, "Right Windows") \
    X(PROGRAMME, 0x015D, "Programme")

struct Key
{
#ifdef _WIN32
#define X(name, id, string) name = id,
	enum KeyStroke
	{
		KEY_STROKES
	};
#undef X
#elif __linux__
#define X(name, id, string) name = id-8,
    enum KeyStroke
    {
        KEY_STROKES
    };
#undef X
#endif

	Key() = default;
	constexpr Key(KeyStroke key):keyValue(key) {};
    Key(int key)
    {
        switch (key)
        {
#define X(name, id, string) \
            case id: \
                keyValue = KeyStroke::name; \
            break;
            KEY_STROKES
#undef X
        default:
            keyValue = KeyStroke::INVALID;
        }
    }

	//allow user to get value by calling 

	constexpr operator KeyStroke() const { return keyValue; };

	//Delete the bool operator in order to programmer from writing if(key)

	explicit operator bool() const = delete;

	constexpr bool operator == (KeyStroke key) const { return key == keyValue; };
	constexpr bool operator != (KeyStroke key) const { return key != keyValue; };

    const char* toString()
    {
        switch (keyValue)
        {
#define X(name, id, string) \
            case id: \
                return string;
                KEY_STROKES
#undef X
        }
    }

    bool isNumberKey()
    {
        return (keyValue >= KeyStroke::ONE && keyValue <= KeyStroke::ZERO);
    }

    int numberKeyToInt()
    {
        assert(isNumberKey());
        if (keyValue == KeyStroke::ZERO)
        {
            return 0;
        }
        return keyValue - KeyStroke::ONE + 1;
    }

	private:
		KeyStroke keyValue;
};

