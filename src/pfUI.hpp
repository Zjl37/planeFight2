#pragma once
#include "pfCommon.hpp"
#include <iostream>
#include <iomanip>
#include <functional>
#include <stack>

extern int scrW, scrH;

void DrawPlane(int x, int y, int d);
void DrawPlane(int x, int y, int d, int bx, int by, int bw, int bh);
void DrawPlaneCw(int x, int y, int d, int bx, int by, int bw, int bh);

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
void BlinkCoord(short ax, short ay, bool signDir);

extern std::stack<PfPage> stPage;
