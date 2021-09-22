#include "pfConsole.hpp"
#include "vtsFilter.hpp"
#ifdef _WIN32
#	include <windows.h>
#else
#	include <termio.h>
#	include <unistd.h>
#endif

extern std::mt19937 rng;
extern VtsInputFilter vtIn;
extern int scrW, scrH;

#define ESC "\x1b"

/* IMPORTANT NOTE:
 * WinAPI coord starts from 0, while VT coord starts from 1! Due to historical
 * reasons, all interfaces defined below takes coords as starting from 0!
 */

// VT code: DECXCPR
// query: ESC [ 6 n
// answer: ESC [ <r> ; <c> R
std::pair<int, int> getXY() {
	std::cout << ESC "[6n";
	if(vtIn.fWork) {
		return vtIn.GetLastCurPos();
	}
	char ch = 0;
	int y, x;
	std::cin >> ch >> ch >> y >> ch >> x >> ch;
	return { x - 1, y - 1 };
}

// NOTE: To get both X and Y coord at the same time, better call getXY instead of
// calling getX then getY

int getX() {
	return getXY().first;
}
int getY() {
	return getXY().second;
}

// VT code: CUB
void CurLeft1() {
	std::cout << ESC "D";
}

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

// TODO: fully test these colors;
const int winapiFgcToVt[17] = { 30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97, 39 };
const int winapiBgcToVt[17] = { 40, 44, 42, 46, 41, 45, 43, 47, 100, 104, 102, 106, 101, 105, 103, 107, 49 };

/// VT code: SGR
void setColor(int fgc, int bgc) {
	std::cout << ESC "[" << winapiFgcToVt[fgc] << ";" << winapiBgcToVt[bgc] << "m";
}

void setDefaultColor() {
	std::cout << ESC "[39;49m";
}

void clear() {
	setDefaultColor();
	std::cout << ESC "[2J";
}

void ClearLineRight() {
	std::cout << ESC "[0K";
}

void clearR(short l, short t, short r, short b) {
	for(int j = t; j <= b; j++) {
		gotoXY(l, j);
		std::cout << std::string(r - l + 1, ' ');
	}
}

// VT code: DECTCEM
// seq for show: ESC [ ? 25 h
// seq for hide: ESC [ ? 25 l
void showCursor_(bool f) {
	std::cout << (f ? ESC "[?25h" : ESC "[?25l");
}

void UseAltScrBuf() {
	std::cout << ESC "[?1049h";
}

void UseMainScrBuf() {
	std::cout << ESC "[?1049l";
}

#ifdef _WIN32
HANDLE hIn, hOut;
CONSOLE_SCREEN_BUFFER_INFO csbi;
DWORD orgConInMode, orgConOutMode; // original console mode

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

void ConInit() {
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE); // get standard handles
	DWORD mode;
	// detect legacy console
	GetConsoleMode(hOut, &mode);
	orgConOutMode = mode;
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // enable virtual terminal
	if(!SetConsoleMode(hOut, mode)) { // is legacy console
		std::cout << "planefight: error: You are using legacy console, planeFight won't be able to run." << std::endl;
		std::cout << "    We recommend that you upgrade to the latest version of Windows 10, or if you're already using one, check if legacy console is enabled." << std::endl;
		exit(1);
	}

	GetConsoleMode(hIn, &mode);
	orgConInMode = mode;
	mode |= ENABLE_PROCESSED_INPUT;
	// mode |= ENABLE_MOUSE_INPUT;
	mode &= ~ENABLE_QUICK_EDIT_MODE;
	mode |= ENABLE_WINDOW_INPUT;
	mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
	mode &= ~ENABLE_LINE_INPUT;
	mode &= ~ENABLE_ECHO_INPUT;
	SetConsoleMode(hIn, mode);

	SetConsoleCP(65001); // change to utf-8
	SetConsoleOutputCP(65001);

	GetConsoleScreenBufferInfo(hOut, &csbi);
	csbi.dwSize.X = std::max((short)80, csbi.dwSize.X);
	csbi.dwSize.Y = std::max((short)30, csbi.dwSize.Y);
	if(csbi.srWindow.Right < 80 || csbi.srWindow.Bottom < 30) {
		SetConsoleScreenBufferSize(hOut, csbi.dwSize);
		csbi.srWindow.Right = csbi.dwSize.X - 1;
		csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
		SetConsoleWindowInfo(hOut, true, &csbi.srWindow);
	}
	scrW = csbi.srWindow.Right, scrH = csbi.srWindow.Bottom;

	UseAltScrBuf(); 

}

void ConReset() {
	UseMainScrBuf();
	VtDisableMouseTracking();
	SetConsoleMode(hOut, orgConOutMode);
	SetConsoleMode(hIn, orgConInMode);
}
#else

termios orgTermios;

void ConInit() {
	tcgetattr(STDIN_FILENO, &orgTermios);
	termios raw = orgTermios;
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

	// TODO: get console window size
	scrW = 80, scrH = 30;

	UseAltScrBuf();
}

void ConReset() {
	UseMainScrBuf();
	VtDisableMouseTracking();
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orgTermios);
}

#endif
