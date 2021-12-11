#include "pfConsole.hpp"
#include <csignal>
#ifdef _WIN32
#	include <windows.h>
#else
#	include <termio.h>
#	include <unistd.h>
#endif

extern int scrW, scrH;

#define ESC "\x1b"

/* IMPORTANT NOTE:
 * WinAPI coord starts from 0, while VT coord starts from 1! Due to historical
 * reasons, all interfaces defined below takes coords as starting from 0!
 */

// VT code: CHA
// seq: ESC [ <n> G
void gotoX(int x) {
	std::cout << ESC "[" << x + 1 << "G";
}

// VT code: VPA
// seq: ESC [ <n> d
void gotoY(int y) {
	std::cout << ESC "[" << y + 1 << "d";
}

// VT code: CUP
// seq: ESC [ <y> ; <x> H
void gotoXY(int x, int y) {
	std::cout << ESC "[" << y + 1 << ";" << x + 1 << "H";
}

void clearR(short l, short t, short r, short b) {
	for(int j = t; j <= b; j++) {
		gotoXY(l, j);
		std::cout << std::string(r - l + 1, ' ');
	}
}

#ifdef _WIN32
HANDLE hIn, hOut;
CONSOLE_SCREEN_BUFFER_INFO csbi;
DWORD orgConInMode, orgConOutMode; // original console mode

std::pair<int, int> GetConScrSize() {
	// We do not want the buffer size:
	// e.Event.WindowBufferSizeEvent.dwSize.X and e.Event.WindowBufferSizeEvent.dwSize.Y;
	// instead, the window boundary size.
	GetConsoleScreenBufferInfo(hOut, &csbi);
	return {csbi.srWindow.Right, csbi.srWindow.Bottom};
}

#else

termios orgTermios;

void ResizeHandler(std::pair<int, int> size);

std::pair<int, int> GetConScrSize() {
	struct winsize winsz;
	ioctl(0, TIOCGWINSZ, &winsz);
	return {winsz.ws_col, winsz.ws_row};
}

void sigHandler(int sig) {
	if(sig == SIGWINCH) {
		ResizeHandler(GetConScrSize());
	}
}

#endif
