#pragma once
#include <random>
#include <string>
#include <mutex>
#include <iostream>

extern std::mutex mtxCout;

std::pair<int, int> getXY();
int getX();
int getY();
void CurLeft1();
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

void setDefaultColor();
void setColor(int fgc, int bgc);
void clear();
void ClearLineRight();
void showCursor_(bool f);
void UseAltScrBuf();
void UseMainScrBuf();

#define showCursor() showCursor_(true)
#define hideCursor() showCursor_(false)

std::pair<int, int> GetConScrSize();

void ConInit();
void ConReset();