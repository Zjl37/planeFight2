#pragma once
#include <iostream>
#include <iomanip>
#include <windows.h>
#include "pfLang.hpp"
using namespace std;
short getX_(HANDLE);
short getY_(HANDLE);
void gotoX_(short,HANDLE);
void gotoY_(short,HANDLE);
void gotoXY_(short,short,HANDLE);

#ifndef _ConsoleColor_
#define _ConsoleColor_
#define black (short)0
#define darkBlue (short)1
#define darkGreen (short)2
#define darkAqua (short)3
#define darkRed (short)4
#define purple (short)5
#define darkYellow (short)6
#define lightGrey (short)7
#define grey (short)8
#define blue (short)9
#define green (short)10
#define aqua (short)11
#define red (short)12
#define pink (short)13
#define yellow (short)14
#define white (short)15
#endif // _ConsoleColor_

#define dfc lightGrey
#define dbc black
#define setDefaultColor_(h) setColor_(dfc,dbc,h)
#define setDefaultColor() setColor(dfc,dbc)

void setColor_(short fgc, short bgc, HANDLE hStdout);
void clear();
void showCursor_(bool f, HANDLE hStdout);

extern HANDLE hIn,hOut;

#define getX() getX_(hOut)
#define getY() getY_(hOut)
#define gotoX(x) gotoX_(x,hOut)
#define gotoY(y) gotoY_(y,hOut)
#define gotoXY(x,y) gotoXY_(x,y,hOut)
#define setColor(fgc,bgc) setColor_(fgc,bgc,hOut)
#define showCursor() showCursor_(true,hOut)
#define hideCursor() showCursor_(false,hOut)

void banner(const pfTextElem&,short,short,short);

typedef struct pfLabel {
	pfTextElem t;
	short x,y,fgc,bgc,fgc2,bgc2;
    bool w;
	void (*clickFunc)(void);
	pfLabel(): x(-1), y(-1), fgc(0), bgc(0), fgc2(0), bgc2(0), w(0), clickFunc(nullptr) {}
	pfLabel(pfTextElem _t, short _x, short _y, short _c0, short _c1, short _c2, short _c3, bool isw): t(_t), x(_x), y(_y), fgc(_c0), bgc(_c1), fgc2(_c2), bgc2(_c3), w(isw), clickFunc(nullptr) {}
	void draw();
    void _click();
	bool click(short,short);
	short right();
} pfLabel;

void box(short,short,short,short,short);

void clearR(short,short,short,short);