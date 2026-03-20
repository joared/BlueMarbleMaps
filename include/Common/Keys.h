#pragma once
#include <string>

#ifdef _WIN32
#define ACTUAL_ID(id) id
#elif __linux__
#define ACTUAL_ID(id) id + 8 // Linux key codes are offset by 8 compared to Windows
#else
#define ACTUAL_ID(id) id
#endif

#define KEY_STROKES \
    X(INVALID, ACTUAL_ID(0x00), "invalid character") \
    X(ESCAPE, ACTUAL_ID(0x01), "Escape") \
    X(ONE, ACTUAL_ID(0x02), "One") \
    X(TWO, ACTUAL_ID(0x03), "Two") \
    X(THREE, ACTUAL_ID(0x04), "Three") \
    X(FOUR, ACTUAL_ID(0x05), "Four") \
    X(FIVE, ACTUAL_ID(0x06), "Five") \
    X(SIX, ACTUAL_ID(0x07), "Six") \
    X(SEVEN, ACTUAL_ID(0x08), "Seven") \
    X(EIGHT, ACTUAL_ID(0x09), "Eight") \
    X(NINE, ACTUAL_ID(0x0A), "Nine") \
    X(ZERO, ACTUAL_ID(0x0B), "Zero") \
    X(PLUS, ACTUAL_ID(0x0C), "Plus") \
    X(ACUTE_ACCENT, ACTUAL_ID(0x0D), "Acute Accent") \
    X(BACKSPACE, ACTUAL_ID(0x0E), "Backspace") \
    X(TAB, ACTUAL_ID(0x0F), "Tab") \
    X(Q, ACTUAL_ID(0x10), "Q") \
    X(W, ACTUAL_ID(0x11), "W") \
    X(E, ACTUAL_ID(0x12), "E") \
    X(R, ACTUAL_ID(0x13), "R") \
    X(T, ACTUAL_ID(0x14), "T") \
    X(Y, ACTUAL_ID(0x15), "Y") \
    X(U, ACTUAL_ID(0x16), "U") \
    X(I, ACTUAL_ID(0x17), "I") \
    X(O, ACTUAL_ID(0x18), "O") \
    X(P, ACTUAL_ID(0x19), "P") \
    X(SWED_AO, ACTUAL_ID(0x1A), "Swedish AO") \
    X(GRAVE_ACCENT, ACTUAL_ID(0x1B), "Grave Accent") \
    X(ENTER, ACTUAL_ID(0x1C), "Enter") \
    X(LEFT_CONTROL, ACTUAL_ID(0x1D), "Left Control") \
    X(A, ACTUAL_ID(0x1E), "A") \
    X(S, ACTUAL_ID(0x1F), "S") \
    X(D, ACTUAL_ID(0x20), "D") \
    X(F, ACTUAL_ID(0x21), "F") \
    X(G, ACTUAL_ID(0x22), "G") \
    X(H, ACTUAL_ID(0x23), "H") \
    X(J, ACTUAL_ID(0x24), "J") \
    X(K, ACTUAL_ID(0x25), "K") \
    X(L, ACTUAL_ID(0x26), "L") \
    X(SWED_OE, ACTUAL_ID(0x27), "Swedish OE") \
    X(SWED_AE, ACTUAL_ID(0x28), "Swedish AE") \
    X(PARAGRAPHE, ACTUAL_ID(0x29), "Paragraphe") \
    X(LEFT_SHIFT, ACTUAL_ID(0x2A), "Left Shift") \
    X(APOSTROPHE, ACTUAL_ID(0x2B), "Apostrophe") \
    X(Z, ACTUAL_ID(0x2C), "Z") \
    X(X, ACTUAL_ID(0x2D), "X") \
    X(C, ACTUAL_ID(0x2E), "C") \
    X(V, ACTUAL_ID(0x2F), "V") \
    X(B, ACTUAL_ID(0x30), "B") \
    X(N, ACTUAL_ID(0x31), "N") \
    X(M, ACTUAL_ID(0x32), "M") \
    X(COMMA, ACTUAL_ID(0x33), ",") \
    X(DOT, ACTUAL_ID(0x34), ".") \
    X(DASH, ACTUAL_ID(0x35), "-") \
    X(RIGHT_SHIFT, ACTUAL_ID(0x36), "Right Shift") \
    X(NUM_STAR, ACTUAL_ID(0x37), "Numpad Star") \
    X(LEFT_ALT, ACTUAL_ID(0x38), "Left Alt") \
    X(SPACE, ACTUAL_ID(0x39), "Space") \
    X(CAPS_LOCK, ACTUAL_ID(0x3A), "Caps Lock") \
    X(F1, ACTUAL_ID(0x3B), "F1") \
    X(F2, ACTUAL_ID(0x3C), "F2") \
    X(F3, ACTUAL_ID(0x3D), "F3") \
    X(F4, ACTUAL_ID(0x3E), "F4") \
    X(F5, ACTUAL_ID(0x3F), "F5") \
    X(F6, ACTUAL_ID(0x40), "F6") \
    X(F7, ACTUAL_ID(0x41), "F7") \
    X(F8, ACTUAL_ID(0x42), "F8") \
    X(F9, ACTUAL_ID(0x43), "F9") \
    X(F10, ACTUAL_ID(0x44), "F10") \
    X(PAUSE, ACTUAL_ID(0x45), "Pause") \
    X(SCROLL_LOCK, ACTUAL_ID(0x46), "Scroll Lock") \
    X(NUM_SEVEN, ACTUAL_ID(0x47), "Numpad Seven") \
    X(NUM_EIGHT, ACTUAL_ID(0x48), "Numpad Eight") \
    X(NUM_NINE, ACTUAL_ID(0x49), "Numpad Nine") \
    X(NUM_SUBTRACT, ACTUAL_ID(0x4A), "Numpad Subtract") \
    X(NUM_FOUR, ACTUAL_ID(0x4B), "Numpad Four") \
    X(NUM_FIVE, ACTUAL_ID(0x4C), "Numpad Five") \
    X(NUM_SIX, ACTUAL_ID(0x4D), "Numpad Six") \
    X(NUM_PLUS, ACTUAL_ID(0x4E), "Numpad Plus") \
    X(NUM_1, ACTUAL_ID(0x4F), "Numpad One") \
    X(NUM_2, ACTUAL_ID(0x50), "Numpad Two") \
    X(NUM_3, ACTUAL_ID(0x51), "Numpad Three") \
    X(NUM_0, ACTUAL_ID(0x52), "Numpad Zero") \
    X(NUM_DELETE, ACTUAL_ID(0x53), "Numpad Delete") \
    X(SYSTEM_REQUEST, ACTUAL_ID(0x54), "System Request") \
    X(LESS_THAN, ACTUAL_ID(0x56), "Less Than") \
    X(F11, ACTUAL_ID(0x57), "F11") \
    X(F12, ACTUAL_ID(0x58), "F12") \
    X(NUM_ENTER, ACTUAL_ID(0x011C), "Numpad Enter") \
    X(RIGHT_CONTROL, ACTUAL_ID(0x011D), "Right Control") \
    X(NUM_SLASH, ACTUAL_ID(0x0135), "Numpad Slash") \
    X(RIGHT_ALT, ACTUAL_ID(0x0138), "Right Alt") \
    X(NUM_LOCK, ACTUAL_ID(0x0145), "Num Lock") \
    X(HOME, ACTUAL_ID(0x0147), "Home") \
    X(UP_ARROW, ACTUAL_ID(0x0148), "Up Arrow") \
    X(PAGE_UP, ACTUAL_ID(0x0149), "Page Up") \
    X(LEFT_ARROW, ACTUAL_ID(0x014B), "Left Arrow") \
    X(RIGHT_ARROW, ACTUAL_ID(0x014D), "Right Arrow") \
    X(END, ACTUAL_ID(0x014F), "End") \
    X(DOWN_ARROW, ACTUAL_ID(0x0150), "Down Arrow") \
    X(PAGE_DOWN, ACTUAL_ID(0x0151), "Page Down") \
    X(INSERT, ACTUAL_ID(0x0152), "Insert") \
    X(DEL_KEY, ACTUAL_ID(0x0153), "Delete") \
    X(LEFT_WINDOWS, ACTUAL_ID(0x015B), "Left Windows") \
    X(RIGHT_WINDOW, ACTUAL_ID(0x015C), "Right Windows") \
    X(PROGRAMME, ACTUAL_ID(0x015D), "Programme")

struct Key
{
#define X(name, id, string) name = id,
	enum KeyStroke
	{
		KEY_STROKES
	};
#undef X

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

