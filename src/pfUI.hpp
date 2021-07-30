#pragma once
#include "pfLang.hpp"
#include "pfConsole.hpp"
#include <iostream>
#include <iomanip>
#include <functional>

extern int scrW, scrH;

void banner(const pfTextElem &, short, short, short);

typedef struct pfLabel {
	pfTextElem t;
	short x, y, fgc, bgc, fgc2, bgc2;
	bool w;
	std::function<void()> clickFunc;
	pfLabel(): x(-1), y(-1), fgc(0), bgc(0), fgc2(0), bgc2(0), w(0) {}
	pfLabel(pfTextElem _t, short _x, short _y, short _c0, short _c1, short _c2, short _c3, bool isw): t(_t), x(_x), y(_y), fgc(_c0), bgc(_c1), fgc2(_c2), bgc2(_c3), w(isw), clickFunc(nullptr) {}
	void draw();
	void _click();
	bool click(short, short);
	short right();
} pfLabel;

void box(short, short, short, short, short);

void clearR(short, short, short, short);

void BlinkCoord(short, short, bool);
void drawPlane(short x, short y, short d, bool r);
void drawPark(int selDir);
void drawBF(bool showBf2);