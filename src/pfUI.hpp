#pragma once
#include "pfCommon.hpp"
#include <iostream>
#include <iomanip>
#include <functional>
#include <stack>

extern int scrW, scrH;

void banner(const std::string &, short, short, short);

typedef struct pfLabel {
	std::string t;
	short x, y, fgc, bgc, fgc2, bgc2;
	bool w;
	std::function<void()> clickFunc;
	pfLabel(): x(-1), y(-1), fgc(0), bgc(0), fgc2(0), bgc2(0), w(0) {}
	pfLabel(const std::string &_t, short _x, short _y, short _c0, short _c1, short _c2, short _c3, bool isw): t(_t), x(_x), y(_y), fgc(_c0), bgc(_c1), fgc2(_c2), bgc2(_c3), w(isw), clickFunc(nullptr) {}
	void draw();
	void _click();
	bool click(short, short);
	short right();
} pfLabel;

void box(short, short, short, short, short);

void clearR(short, short, short, short);

void BlinkCoord(short, short, bool);
void DrawPlane(int x, int y, int d);
void DrawPlane(int x, int y, int d, int bx, int by, int bw, int bh);
void DrawPlaneCw(int x, int y, int d, int bx, int by, int bw, int bh);
void drawPark(int selDir, int y);
void DrawBF(const pfBF &bf1, const pfBF &bf2, int x1, int y1, int x2, int y2);

enum class PfPage_legacy {
	welcome = 0,
	main = 1,
	prepare = 2,
	adjust_map = 4,
	about = 5,
	game = 10,
	gameover = 19,
	gamerule_setting_server = 41,
	server_init = 42,
	client_init = 51,
	error,
};

enum class PfPage {
	welcome = 0,
	main = 1,
	prepare = 2,
	adjust_map = 3,
	about = 4,
	game = 5,
	gameover = 6,
	gamerule_setting_server = 7,
	server_init = 8,
	client_init = 9,
	error = 10,
};

void RefreshPage();
void showErrorMsg(const std::string &t, PfPage rpage);
void showErrorMsg(const std::string &t);
void SetPage(PfPage x);
void NextPage(PfPage x);
void PrevPage();

void UiGameStart();
void UiGameover();
void UiShowAtkRes(PfAtkRes);

extern std::stack<PfPage> stPage;
