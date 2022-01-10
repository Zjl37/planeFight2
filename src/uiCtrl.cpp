/**
 * Copyright © 2021-2022 Zjl37 <2693911885@qq.com>
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
#include "remotePlayer.hpp"
#include "pfLocale.hpp"
#include "ai.hpp"
#include <boost/system/system_error.hpp>

extern std::mt19937 rng;

namespace pfui {

	int pageNum;
	std::string playername;

	namespace ctrl {
		std::string ipAddr;
		std::string errMsg;
		int p2SelectedFacing;

		enum {
			pf_local_game,
			pf_remote_game_client,
			pf_remote_game_server,
		} curGameType;

		void P0InputOK() {
			if(playername.empty()) return;
			NextPage(PfPage::main);
		}
		void P1PlayLocal() {
			player0.reset(new PfLocalPlayer(pfui::playername));
			player1.reset(new PfAI());
			player0->other = player1;
			player1->other = player0;
			curGameType = pf_local_game;
			pfui::p2IsNetworkGame = false;
			curGame.isFirst = rng() & 1;
			bf1.clear();
			NextPage(PfPage::prepare);
		}
		void P2Clear() {
			if(player0->GetGame().state & PfGame::me_ready) return;
			bf1.clear();
		}
		void P2Ready() {
			if(bf1.nPlaced != curGame.n) return;
			if(curGameType == pf_local_game) {
				if(!curGame.n) return;
				PfBF bf2(curGame.w, curGame.h);
				bool tmp = bf2.AutoArrange();
				if(tmp == false) {
					// RefreshPage();
					return;
				}
				unsigned gameId = rng();
				player0->NewGame(curGame, gameId);
				player1->NewGame(InvertIsFirst(curGame), gameId);
				static_cast<PfAI*>(&*player1)->ArrangeReady(bf2);
				player0->ArrangeReady(bf1);
			} else {
				if(!(player0->GetGame().state & PfGame::me_ready)) {
					player0->ArrangeReady(bf1);
				}
			}
		}
		void P5Surrender() {
			player0->Surrender();
		}
		void P7StartServer() {
			player0.reset(new PfLocalPlayer(pfui::playername));
			curGameType = pf_remote_game_server;
			pfui::p2IsNetworkGame = true;
			NextPage(PfPage::server_init);
			PfServerInit();
		}
		void P8ServerStop() {
			player1.reset();
			PrevPage();
			PfServerStop();
		}
		void P9ClientStop() {
			player1.reset();
			PrevPage();
			PfStopConnect();
		}
		void P9StartClient() {
			static bool connecting = 0;
			if(connecting) {
				return;
			} else {
				connecting = 1;
			}
			try {
				player0.reset(new PfLocalPlayer(pfui::playername));
				player1 = PfCreateRemoteServer(pfui::ctrl::ipAddr);
				player0->other = player1;
				player1->other = player0;
				curGameType = pf_remote_game_client;
				pfui::p2IsNetworkGame = true;
			} catch(const std::string &t) {
				showErrorMsg(t);
			} catch(const boost::system::system_error &e) {
				showErrorMsg(TT("Error: Cannot connect to server. Please check if the IP is correct and if the server is running."));
			}
			connecting = 0;
		}
	}
}