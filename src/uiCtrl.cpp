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

#include "uiCtrl.hpp"
#include "ui.hpp"
#include "game.hpp"
#include "ai.hpp"

extern std::mt19937 rng;

void StartLocalGame();
void p2Ready();
void StartServer();
void PfServerStop();
void PfStopConnect();
void StartClient();
// TODO: move more implementation here

namespace pfui {

	int pageNum;
	std::string playername;

	namespace ctrl {
		std::string ipAddr;
		std::string errMsg;
		int p2SelectedFacing;

		void P0InputOK() {
			if(playername.empty()) return;
			NextPage(PfPage::main);
		}
		void P1PlayLocal() {
			StartLocalGame();
		}
		void P2Clear() {
			if(player[0]->GetGame().state & PfGame::me_ready) return;
			bf1.clear();
		}
		void P2Ready() {
			if(bf1.nPlaced != curGame.n) return;
			p2Ready();
		}
		void P5Surrender() {
			player[0]->Surrender();
		}
		void P7StartServer() {
			StartServer();
		}
		void P8ServerStop() {
			player[1].reset();
			PrevPage();
			PfServerStop();
		}
		void P9ClientStop() {
			player[1].reset();
			PrevPage();
			PfStopConnect();
		}
		void P9InputOK() {
			StartClient();
		}
	}
}