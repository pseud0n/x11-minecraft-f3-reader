#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

#include <cstdio>
#include <cstring>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/extensions/XTest.h>


#include "x11/ReaderController.hpp"

int main() {
	//DelayedFunction<> df(10, [](){ std::cout << "10 loops!\n"; });
	//for (int i = 0; i < 20; ++i) {
	//	df.CountDownOrSilence();
	//}
	ReaderController rc;
	//sleep(2);
	//rc.QueueText("/tellraw @a {\"text\":\"greetings\",\"color\":\"red\"}");
//	rc.NextCycle();

	//printf("%i, %i\n", ShiftMask, ControlMask);
	float y = 0;
	rc.KeyDown(" ");
	while(1) {
		auto start = std::chrono::high_resolution_clock::now();
		rc.NextCycle(); //df.countDownOrSilence();
		y = rc.GetY();
		auto stop = std::chrono::high_resolution_clock::now();
		if (y > 0)
			break;
		usleep(100*1000);
		std::clog << "Loop took " << duration_cast<std::chrono::milliseconds>(stop - start).count() << "\n";
	}
	rc.KeyUp(" ");
	rc.NextCycle();
	puts("Released");
}
