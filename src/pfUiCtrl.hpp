/**
 * Copyright © 2021 Zjl37 <2693911885@qq.com>
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
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/util/ref.hpp"
#include "pfCommon.hpp"

namespace pfui {

	extern int pageNum;
	extern std::string playername;
	extern ftxui::ScreenInteractive scr;
	extern int p2IsNetworkGame;
	extern bool p2ShowBanner;

	struct AttackIndicatorInfo {
		int x, y;
		bool signDir;
		enum {
			none,
			blink1,
			blink2,
			blink3,
			blink4,
			resulted
		} state = none;
		PfAtkRes res;

		std::string MakeCoordStr() const;
	};
	extern AttackIndicatorInfo p5AttackInfo;

	namespace ctrl {
		extern std::string ipAddr;
		extern std::string errMsg;
		extern int p2SelectedFacing;

		void P0InputOK();
		void P1PlayLocal();
		void P2Clear();
		void P2Ready();
		void P5Surrender();
		void P7StartServer();
		void P8ServerStop();
		void P9ClientStop();
		void P9InputOK();
	}

	void Build();
	void Loop();
}