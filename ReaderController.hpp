#ifndef CHARACTER_RECOGNITION_H
#define CHARACTER_RECOGNITION_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <X11/extensions/XTest.h>

#include <iostream>
#include <map>
#include <unordered_map>
#include <cstring>
#include <optional>
#include <array>
#include <set>
#include <queue>

#include <unistd.h>

#include "../DelayedFunction.hpp"

struct ReaderController {
public:
	using Unit = int;
	using Pixel = int;
	using CoordT = float;
	using CoordsT = std::array<CoordT, 3>;
//private:
	static constexpr int coordsXTopLeft = 26, coordsYTopLeft = 92;

	static constexpr int textColour = 0xdddddd;

	static constexpr int maskSize = 6;
	static constexpr int mask[maskSize][2] = {
		{0,3},
		{1,3},
		{2,3},
		{3,3},
		{4,3},
		{4,4},
	};

	static std::map<int, int> maskedToInt;

	static constexpr int minusSign = 62;
	static constexpr int keyDown = 1, keyUp = 0;

	std::queue<std::string> queuedText;

	std::queue<void*> delayedFunctions;

	XImage *image = 0;

	CoordsT coords;

	Display *display = 0;


	Window rootWindow, minecraftWindow, rootReturn; // For windows we don't care about

	Atom windowsProperty, nameProperty;
	const char *const namePropertyString = "WM_NAME";
	//const char *const namePropertyString = "WM_CLASS";

	const char *const targetName = "Minecraft 1.19";
	//const char *const targetName = "IDLE Shell 3.9.13";
	//const char *const targetName = "Event Tester";
	const int targetLength = std::strlen(targetName);

	char *minecraftWindowName = 0;
	Window *windows = 0;

	unsigned int windowW, windowH, windowBorderH, windowBorderWidth, windowDepth;
	int windowX, windowY;

	std::set<char> heldCharacters, toReleaseCharacters;

	//XKeyEvent keyboardEvent;
	XEvent inputEvent;
	XKeyEvent& keyboardEvent = inputEvent.xkey;
	XButtonEvent& mouseEvent = inputEvent.xbutton;

	static std::map<char, KeySym> charsToKeySyms;
	static constexpr const char *const mouseButtons = "lmdrud"; // left middle right up down

	static int GetButton(char);

	void GetWindow();

	void SetupKeyPress();

public:

	ReaderController();

	~ReaderController();


	void GetXImage();

	int GetNumberMasked(int, int);

	bool CheckDot(int, int);

	std::optional<std::array<CoordT, 3>> InterpretCoord();

	unsigned char *GetWindowProperty(Window, Atom, Atom, unsigned long * = NULL);

	void Once();

	void SendKeyEvent(char);
	void KeyDown(const char*);
	void KeyUp(const char*);
	void MouseDown(char);
	void MouseUp(char);
	void MaskDown(char);
	void MaskUp(char);
	std::string QueryHeldCharacters() const;

	bool MinecraftIsFocused() const;
	void FocusMinecraft();

	void NextCycle();

	void TempDisableAllKeys();
	void ReenableAllKeys();

	CoordT GetX() const;
	CoordT GetY() const;
	CoordT GetZ() const;

	void QueueText(const char *);

	template <typename... Ts>
	void QueueCall(DelayedFunction<>::NumT, DelayedFunction<>::FunctionT const &, Ts...);
	void DecrementOrCallDelayedFunction(void *ptr);
};

#endif
