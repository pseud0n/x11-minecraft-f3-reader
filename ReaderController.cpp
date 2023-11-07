#include <iostream>
#include "ReaderController.hpp"
#include <chrono>

std::map<char, KeySym> ReaderController::charsToKeySyms;
std::map<int, int> ReaderController::maskedToInt;

unsigned char *ReaderController::GetWindowProperty(Window window, Atom property, Atom requiredType, unsigned long *nItemsReturnPtr) {
	static unsigned long bytesAfterReturn;
	static long longLength = 1024;
	static Atom actualTypeReturn;
	static int actualFormatReturn;
	unsigned char *propReturn;
	XGetWindowProperty(display,
					window,
					property,
					0, longLength,
					false,
					requiredType,
					&actualTypeReturn, &actualFormatReturn,
					nItemsReturnPtr,
					&bytesAfterReturn,
					&propReturn);
	return propReturn;
}
ReaderController::ReaderController() {
	// Setup XImage
	display = XOpenDisplay(nullptr);
	rootWindow = XDefaultRootWindow(display);
	windowsProperty = XInternAtom(display, "_NET_CLIENT_LIST_STACKING", true);
	nameProperty = XInternAtom(display, namePropertyString, true);
	if (charsToKeySyms.empty()) {
		for (int c = 33; c <= 255; ++c) {
			// a:97, 0x61
			charsToKeySyms[c] = XKeysymToKeycode(display, c);
		}
		charsToKeySyms[' '] = 65;
		charsToKeySyms['\n'] = 36;

		maskedToInt[43] = 0;
		maskedToInt[8] = 1;
		maskedToInt[12] = 2;
		maskedToInt[13] = 3;
		maskedToInt[35] = 4;
		maskedToInt[3] = 5;
		maskedToInt[61] = 6;
		maskedToInt[4] = 7;
		maskedToInt[29] = 8;
		maskedToInt[31] = 9;

		maskedToInt[minusSign] = 10;
	}
	GetWindow();
	SetupKeyPress();
	//std::clog << "Constructed\n";
}

ReaderController::~ReaderController() {
	if (image)
		XDestroyImage(image);
	//std::clog << "Free: " << minecraftWindowName << ", " << windows << "\n";
	if (minecraftWindowName)
		XFree(minecraftWindowName);
	if (windows)
		XFree(windows); // TODO
	if (display)
		XCloseDisplay(display);
	//std::clog << "Destructed\n";
}

void ReaderController::GetWindow() {
	bool windowFound = false;
	unsigned long windowCount;
	unsigned long namePropertyLength;
	//std::clog << "Getting window\n";
	for (;;) {
		windows = (Window *)GetWindowProperty(rootWindow, windowsProperty, XA_WINDOW, &windowCount);
		for (int i = 0; i < windowCount; ++i) {
			minecraftWindowName = (char *)GetWindowProperty(windows[i], nameProperty, XA_STRING, &namePropertyLength);
			//std::clog << targetName << ", " << namePropertyLength << "," << minecraftWindowName << "\n";
			if (namePropertyLength == 0) {
				XFree(minecraftWindowName);
				continue;
			}
			if (!std::strncmp(targetName, minecraftWindowName, targetLength)) {
				minecraftWindow = windows[i];
				windowFound = true;
				std::cout << "Found window: " << minecraftWindowName << '\n';
				break;
			}
			XFree(minecraftWindowName);
		}
		//windowFound = true;
		if (windowFound)
			break;
		std::cerr << "Could not find window with " << namePropertyString << " preceded by '" << targetName << "', or no " << namePropertyString << " present in target window\n";
		XFree(windows);
		sleep(3);
	}
	//std::clog << "Got window\n";
}

void ReaderController::SetupKeyPress() {
	keyboardEvent.display = mouseEvent.display = display;
	keyboardEvent.window = mouseEvent.window = minecraftWindow;
	keyboardEvent.root = mouseEvent.root = rootWindow;

	keyboardEvent.subwindow = keyboardEvent.subwindow = None;
	keyboardEvent.time = mouseEvent.time = CurrentTime; // ?

	keyboardEvent.state = mouseEvent.state = 0;
	//keyboardEvent.keycode = 25;
	keyboardEvent.same_screen = mouseEvent.same_screen = True;

	keyboardEvent.x = keyboardEvent.y = keyboardEvent.x_root = keyboardEvent.y_root = 1;

	//std::clog << "Setup key press\n";
}


void ReaderController::GetXImage() {
	int windowXBorder, windowYBorder; // Temporary
	//std::clog <<
	XGetGeometry(display, minecraftWindow, &rootReturn, &windowXBorder, &windowYBorder, &windowW, &windowH, &windowBorderWidth, &windowDepth);
	//XGetGeometry(Display *, Drawable, Window *, int *, int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *)
	//<< '\n';
	//std::clog << windowXBorder << " " << windowYBorder << "\n";

	int windowX, windowY;
	XTranslateCoordinates(display, minecraftWindow, rootReturn, windowXBorder,  windowYBorder, &windowX, &windowY, &rootReturn);
	// Obtain top-left coordinates relative to root window
	windowX -= windowXBorder;
	windowY -= windowYBorder;
	//std::clog << windowX << " " << windowY << " " << windowW << " " << windowH << " " << windowBorderWidth << " " << windowDepth << "\n";
	if (image)
		XDestroyImage(image);
	//std::clog << "Getting X image\n";
	//XMapRaised(display, minecraftWindow);
	image = XGetImage(display, rootWindow, windowX, windowY, windowW, windowH, AllPlanes, XYPixmap);
	//std::clog << "Got X image\n";
}

int ReaderController::GetNumberMasked(Unit xTopLeft, Unit yTopLeft) {
	 // Top left pixel
	int resultNum = 0;
	XColor rgb;
	/*
	rgb.pixel = XGetPixel(image, xTopLeft, yTopLeft);
	XQueryColor(display, XDefaultColormap(display, XDefaultScreen(display)), &rgb);
	for (int y = yTopLeft; y < yTopLeft + 14; y += 2) { // y goes down screen
		for (int x = xTopLeft; x < xTopLeft + 10; x += 2) {
			rgb.pixel = XGetPixel(image, x, y);
			XQueryColor(display, XDefaultColormap(display, XDefaultScreen(display)), &rgb);
			if (rgb.pixel == textColour)
				std::cout << "O";
				//resultNum += 1;
			else
				std::cout << " ";
		}
		std::cout << "\n";
	}
	*/
	//std::cout << "\n";
	for (int p = 0; p < maskSize; ++p) {
		resultNum <<= 1;
		rgb.pixel = XGetPixel(image, 2 * (xTopLeft + mask[p][0]), 2 * (yTopLeft +  mask[p][1]));
		XQueryColor(display, XDefaultColormap(display, XDefaultScreen(display)), &rgb);
		if (rgb.pixel == textColour) {
			resultNum += 1;
			//std::clog << mask[p][0] << "," << mask[p][1] << " ";
		}
	}
	//std::cout << "\n";
	//std::clog << maskedToInt[resultNum] << '\n';
	//return maskedToInt[resultNum];
	return resultNum;
}

bool ReaderController::CheckDot(Unit xTopLeft, Unit yTopLeft) {
	XColor bottomLeft, rightOfBottomLeft;
	bottomLeft.pixel = XGetPixel(image, 2 * xTopLeft, 2 * yTopLeft + 12);
	rightOfBottomLeft.pixel = XGetPixel(image, 2 * xTopLeft + 2, 2 * yTopLeft + 12);
	XQueryColor(display, XDefaultColormap(display, XDefaultScreen(display)), &bottomLeft);
	XQueryColor(display, XDefaultColormap(display, XDefaultScreen(display)), &rightOfBottomLeft);
	return bottomLeft.pixel == textColour && rightOfBottomLeft.pixel != textColour;
	// 2nd condition for 1,2
}

std::optional<ReaderController::CoordsT> ReaderController::InterpretCoord() {
	Unit x = coordsXTopLeft, y = coordsYTopLeft;
	int multiplyBy ; // Or -1
	// As units
	std::array<float, 3> coordinates;
	for (int i = 0; i < 3; ++i) {
		float result = 0;
		int next = 0;
		multiplyBy = 1;
		do {
			next = GetNumberMasked(x, y);
			if (next == 0) {
				sleep(1);
				return std::nullopt;
			}
			//std::clog << "(" << x << "," << y << ")" << next <<  "->" << maskedToInt[next] << " ";
			result *= 10;
			if (next == minusSign)
				multiplyBy = -1;
			else
				result += maskedToInt.at(next);
			x += 6;
		} while (!CheckDot(x, y));
		x += 2;
		int place = 10;
		for (;;) {
			next = GetNumberMasked(x, y);
			if (!next)
				break;
			//std::clog << "(" << x << "," << y << ")" << next <<  "->" << maskedToInt[next] << " ";
			result += (float)maskedToInt.at(next) / place;
			place *= 10;
			x += 6;
		}
		x += 14;
		result *= multiplyBy;
		//std::clog << result << ' ';
		coordinates[i] = result;
	}
	//std::clog << "\n";
	return std::optional(coordinates);
}

void ReaderController::Once() {
	 GetXImage();
}

void ReaderController::SendKeyEvent(char c) {
	XTestFakeKeyEvent(display, charsToKeySyms.at(c), keyDown, CurrentTime);
	XFlush(display);
	XTestFakeKeyEvent(display, charsToKeySyms.at(c), keyUp, CurrentTime);
	XFlush(display);
}


void ReaderController::KeyDown(const char* keys) {
	for (const char *c = keys; *c; ++c) {
		//XSendEvent(keyboardEvent.display, keyboardEvent.window, 0, KeyPressMask, (XEvent *)&keyboardEvent);
		//XFlush(display);
		heldCharacters.insert(*c);
	}
	//puts("Key down");
}

void ReaderController::KeyUp(const char* keys) {
	for (const char *c = keys; *c; ++c) {
		//XTestFakeKeyEvent(display, charsToKeySyms.at(*c), True, CurrentTime);
		//XFlush(display);
		if (heldCharacters.erase(*c))
			toReleaseCharacters.insert(*c);
	}
	//puts("Key up");

}
int ReaderController::GetButton(char btn) {
	for (int i = 0; mouseButtons[i]; ++i) {
		if (mouseButtons[i] == btn)
			return i + 1;
	}
	return 1;
}

void ReaderController::MouseDown(char btn) {
	mouseEvent.button = GetButton(btn);
}

std::string ReaderController::QueryHeldCharacters() const {
	return std::string(heldCharacters.begin(), heldCharacters.end());
	//s.reserve(heldCharacters.size() + 1 + toReleaseCharacters.size());
	//s.append(heldCharacters.begin(), heldCharacters.end());
	//s.append(",");
	//s.append(toReleaseCharacters.begin(), toReleaseCharacters.end());
	//return s;
}

bool ReaderController::MinecraftIsFocused() const {
	static Window focusedWindow;
	static int revertToReturn;
	XGetInputFocus(display, &focusedWindow, &revertToReturn);
	return focusedWindow == minecraftWindow;
}

void ReaderController::FocusMinecraft() {
	XMapRaised(display, minecraftWindow);
	XSetInputFocus(display, minecraftWindow, RevertToNone, CurrentTime);
}

void ReaderController::TempDisableAllKeys() {
	for (char c : heldCharacters)
		XTestFakeKeyEvent(display, charsToKeySyms.at(c), keyUp, CurrentTime);
	XFlush(display);
}

void ReaderController::ReenableAllKeys() {
	for (char c : heldCharacters)
		XTestFakeKeyEvent(display, charsToKeySyms.at(c), keyDown, CurrentTime);
	XFlush(display);
}

void ReaderController::NextCycle() {
	while (!MinecraftIsFocused()) {
		TempDisableAllKeys();
		std::cerr << "Minecraft window is not focused, this cycle will not progress until it is, retrying in 5s...\n";
		sleep(5);
	}
	std::optional<CoordsT> coordsAttempt = std::nullopt;
	while (1) {
		GetXImage();
		coordsAttempt = InterpretCoord();
		if (coordsAttempt) break;
		TempDisableAllKeys();
		std::cerr << "F3 not available, will not progress cycle until it is, retrying in 5s...\n";
		sleep(5);
	}
	coords = *coordsAttempt;
	//std::clog << "Before: " << QueryHeldCharacters() << "\n";
	// If a character is to be released and pressed, it must've been requested to pressed more recently

	if (!queuedText.empty()) {
		TempDisableAllKeys();
		std::string front;
		while (!queuedText.empty()) {
			std::clog << queuedText.front() << "\n";
			front = queuedText.front();
			for (int i = 0; i < front.size(); ++i) {
				SendKeyEvent(front[i]);
				if (i == 0)
					usleep(50*1000);
			}
			queuedText.pop();
		}
	}
	ReenableAllKeys();
	for (char c : toReleaseCharacters) {
		XTestFakeKeyEvent(display, charsToKeySyms.at(c), keyUp, CurrentTime);
	}
	for (char c : heldCharacters) {
		XTestFakeKeyEvent(display, charsToKeySyms.at(c), keyDown, CurrentTime);
	}
	XFlush(display);
	toReleaseCharacters.clear();
	//std::clog << "After: " << QueryHeldCharacters() << "\n";
}

ReaderController::CoordT ReaderController::GetX() const {
	// Requires nextCycle to have up-to-date value
	return coords.at(0);
}

ReaderController::CoordT ReaderController::GetY() const {
	return coords.at(1);
}

ReaderController::CoordT ReaderController::GetZ() const {
	return coords.at(2);
}

void ReaderController::QueueText(const char * line) {
	queuedText.push(line);
}

template <typename... Ts>
void ReaderController::QueueCall(DelayedFunction<>::NumT remaining, DelayedFunction<>::FunctionT const & function, Ts... args) {
	auto df = new DelayedFunction<Ts...>(remaining, function, args...);
	delayedFunctions.push(df);
}

void ReaderController::DecrementOrCallDelayedFunction(void *ptr) {
	DelayedFunction<>::NumT& value = *(DelayedFunction<>::NumT*)ptr;
	std::clog << value << "\n";
	--value;
}
