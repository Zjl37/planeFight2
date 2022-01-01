/**
 * Copyright © 2020-2021 Zjl37 <2693911885@qq.com>
 *
 * This file is part of Zjl37/planeFight2.
 *
 * PlaneFight2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
