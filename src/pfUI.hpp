#pragma once
#include <iostream>
#include <iomanip>
#include <functional>
#include <windows.h>
#include "pfLang.hpp"
using namespace std;
pair<int, int> getXY();
int getX();
int getY();
void gotoX(int);
void gotoY(int);
void gotoXY(int, int);

#ifndef _ConsoleColor_
#define _ConsoleColor_
#define black 0
#define darkBlue 1
#define darkGreen 2
#define darkAqua 3
#define darkCyan 3
#define darkRed 4
#define purple 5
#define magenta 5
#define darkYellow 6
#define lightGrey 7
#define grey 8
#define blue 9
#define green 10
#define aqua 11
#define cyan 11
#define red 12
#define pink 13
#define brightMagenta 13
#define yellow 14
#define white 15
// #define dfc lightGrey
// #define dbc black
#define dfc 16
#define dbc 16
#endif // _ConsoleColor_

// #define setDefaultColor_(h) setColor_(dfc, dbc, h)
// #define setDefaultColor() setColor(dfc, dbc)

void setDefaultColor();
void setColor(int fgc, int bgc);
void clear();
void showCursor_(bool f);

extern HANDLE hIn, hOut;

#define showCursor() showCursor_(true)
#define hideCursor() showCursor_(false)

void banner(const pfTextElem &, short, short, short);

typedef struct pfLabel {
	pfTextElem t;
	short x, y, fgc, bgc, fgc2, bgc2;
	bool w;
	function<void()> clickFunc;
	pfLabel(): x(-1), y(-1), fgc(0), bgc(0), fgc2(0), bgc2(0), w(0), clickFunc(nullptr) {}
	pfLabel(pfTextElem _t, short _x, short _y, short _c0, short _c1, short _c2, short _c3, bool isw): t(_t), x(_x), y(_y), fgc(_c0), bgc(_c1), fgc2(_c2), bgc2(_c3), w(isw), clickFunc(nullptr) {}
	void draw();
	void _click();
	bool click(short, short);
	short right();
} pfLabel;

void box(short, short, short, short, short);

void clearR(short, short, short, short);