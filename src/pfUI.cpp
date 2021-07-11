#include "pfUI.hpp"
#include "pfLang.hpp"
using namespace std;

extern SMALL_RECT winr;
extern string mapEdge[256];

#define ESC "\x1b"

/* IMPORTANT NOTE:
 * WinAPI coord starts from 0, while VT coord starts from 1! Due to historical
 * reasons, all interfaces defined below takes coords as starting from 0!
 */

// VT code: DECXCPR
// query: ESC [ 6 n
// answer: ESC [ <r> ; <c> R
pair<int, int> getXY() {
	cout << ESC "[6n";
	char ch = 0;
	int y, x;
	cin >> ch >> ch >> y >> ch >> x >> ch;
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

// VT code: CHA
// seq: ESC [ <n> G
void gotoX(int x) {
	cout << ESC "[" << x+1 << "G";
}

// VT code: VPA
// seq: ESC [ <n> d
void gotoY(int y) {
	cout << ESC "[" << y+1 << "d";
}

// VT code: CUP
// seq: ESC [ <y> ; <x> H
void gotoXY(int x, int y) {
	cout << ESC "[" << y+1 << ";" << x+1 << "H";
}

// TODO: fully test these colors;
const int winapiFgcToVt[17] = { 30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97, 39 };
const int winapiBgcToVt[17] = { 40, 44, 42, 46, 41, 45, 43, 47, 100, 104, 102, 106, 101, 105, 103, 107, 49 };

/// VT code: SGR
void setColor(int fgc, int bgc) {
	cout << ESC "[" << winapiFgcToVt[fgc] << ";" << winapiBgcToVt[bgc] << "m";
}

void setDefaultColor() {
	cout << ESC "[39;49m";
}

void clear() {
	setDefaultColor();
	cout << ESC "[2J";
}

void clearR(short l, short t, short r, short b) {
	for(int j = t; j <= b; j++) {
		gotoXY(l, j);
		for(int i = l; i <= r; i++) cout << ' ';
	}
}

// VT code: DECTCEM
// seq for show: ESC [ ? 25 h
// seq for hide: ESC [ ? 25 l
void showCursor_(bool f) {
	cout << (f ? ESC "[?25h" : ESC "[?25l");
}

void banner(const pfTextElem &msg, short h, short fgc, short bgc) {
	setColor(fgc, bgc);
	gotoXY(0, h);
	cout << setw(winr.Right) << " ";
	gotoXY(0, h + 1);
	cout << setw(winr.Right) << " ";
	gotoXY(0, h + 2);
	cout << setw(winr.Right) << " ";
	gotoXY((winr.Right - msg.len()) / 2, h + 1);
	cout << msg.s;
	setDefaultColor();
}

void pfLabel::draw() {
	if(!~x || !~y) return;
	setColor(fgc, bgc);
	if(w) {
		gotoXY(x, y - 1), cout << setw(t.len()) << "";
		gotoXY(x, y + 1), cout << setw(t.len()) << "";
	}
	gotoXY(x, y), cout << t.s;
}

void pfLabel::_click() {
	setColor(fgc2, bgc2);
	if(w) {
		gotoXY(x, y - 1), cout << setw(t.len()) << "";
		gotoXY(x, y + 1), cout << setw(t.len()) << "";
	}
	gotoXY(x, y), cout << t.s;
	Sleep(100);
	clickFunc();
}

bool pfLabel::click(short mx, short my) {
	if(clickFunc == nullptr) return false;
	if(my < y - 1 || my > y + 1 || (!w && my != y) || mx < x || mx > x + t.len()) return false;
	_click();
	return true;
}

short pfLabel::right() {
	return x + t.len();
}

void box(short x, short y, short w, short h, short edge) {
	// edge style: 0=single 1=bold 2=double
	for(int i = x; i < x + w; i += 2)
		if(edge == 0) {
			gotoXY(i, y - 1), cout << mapEdge[0];
			gotoXY(i, y + h), cout << mapEdge[0];
		} else if(edge == 1) {
			gotoXY(i, y - 1), cout << mapEdge[1];
			gotoXY(i, y + h), cout << mapEdge[1];
		} else if(edge == 2) {
			gotoXY(i, y - 1), cout << mapEdge[6];
			gotoXY(i, y + h), cout << mapEdge[6];
		}
	for(int j = y; j < y + h; j++)
		if(edge == 0) {
			gotoXY(x - 2, j), cout << mapEdge[2];
			gotoXY(x + w, j), cout << mapEdge[2];
		} else if(edge == 1) {
			gotoXY(x - 2, j), cout << mapEdge[3];
			gotoXY(x + w, j), cout << mapEdge[3];
		} else if(edge == 2) {
			gotoXY(x - 2, j), cout << mapEdge[7];
			gotoXY(x + w, j), cout << mapEdge[7];
		}
	if(edge == 0) {
		gotoXY(x - 2, y - 1), cout << mapEdge[4];
		gotoXY(x + w, y - 1), cout << mapEdge[12];
		gotoXY(x - 2, y + h), cout << mapEdge[13];
		gotoXY(x + w, y + h), cout << mapEdge[14];
	} else if(edge == 1) {
		gotoXY(x - 2, y - 1), cout << mapEdge[5];
		gotoXY(x + w, y - 1), cout << mapEdge[15];
		gotoXY(x - 2, y + h), cout << mapEdge[16];
		gotoXY(x + w, y + h), cout << mapEdge[17];
	} else if(edge == 2) {
		gotoXY(x - 2, y - 1), cout << mapEdge[8];
		gotoXY(x + w, y - 1), cout << mapEdge[9];
		gotoXY(x - 2, y + h), cout << mapEdge[10];
		gotoXY(x + w, y + h), cout << mapEdge[11];
	}
}
