/**
 * Copyright © 2020-2021 Zjl37 <2693911885@qq.com>
 * Copyright © 2021 qwqAutomaton
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

#include <ctime>
#include "pfRemotePlayer.hpp"
#include "pfLocale.hpp"
#include "pfAI.hpp"
#include "pfUiCtrl.hpp"

using namespace std;

mt19937 rng(time(nullptr)); // random number generator by MT19937 algorithm

bool isFirst;
int scrW, scrH;
PfBF bg, bf1;

string sIP;

mutex mtxCout;

enum {
	pf_local_game,
	pf_remote_game_client,
	pf_remote_game_server,
} curGameType;

// TODO: determine screen size in FTXUI
void GenerateBackground() {
	bg.resize(scrW / 2, scrH);
	for(int i = 0; i < (int)bg.w * bg.h / 25; i++)
		bg.placeplane(rng() % bg.w, rng() % bg.h, rng() & 3, curGame.cw);
}

void StartLocalGame() {
	player[0].reset(new PfLocalPlayer(pfui::playername));
	player[1].reset(new PfAI());
	player[0]->other = player[1];
	player[1]->other = player[0];
	curGameType = pf_local_game;
	pfui::p2IsNetworkGame = false;
	isFirst = rng() & 1;
	bf1.clear();
	NextPage(PfPage::prepare);
}

void StartServer() {
	player[0].reset(new PfLocalPlayer(pfui::playername));
	curGameType = pf_remote_game_server;
	pfui::p2IsNetworkGame = true;
	NextPage(PfPage::server_init);
	PfServerInit();
}

void p1Play21() {
	isFirst = rng() & 1;
	NextPage(PfPage::gamerule_setting_server);
}
void p1Play22() {
	sIP = "";
	NextPage(PfPage::client_init);
}

void StartClient() {
	static bool connecting = 0;
	if(connecting) {
		return;
	} else {
		connecting = 1;
	}
	try {
		player[0].reset(new PfLocalPlayer(pfui::playername));
		player[1] = PfCreateRemoteServer(pfui::ctrl::ipAddr, *player[0]);
		player[0]->other = player[1];
		player[1]->other = player[0];
		curGameType = pf_remote_game_client;
		pfui::p2IsNetworkGame = true;
	} catch(const std::string &t) {
		showErrorMsg(t);
	} catch(const boost::system::system_error &e) {
		showErrorMsg(TT("Error: Cannot connect to server. Please check if the IP is correct and if the server is running."));
	}
	connecting = 0;
}

void p2Ready() {
	if(curGameType == pf_local_game) {
		if(!curGame.n) return;
		PfBF bf2(curGame.w, curGame.h);
		bool tmp = bf2.AutoArrange();
		if(tmp == false) {
			// RefreshPage();
			return;
		}
		unsigned gameId = rng();
		player[0]->NewGame(curGame, gameId, isFirst);
		player[1]->NewGame(curGame, gameId, !isFirst);

		static_cast<PfAI*>(&*player[1])->ArrangeReady(bf2);
		static_cast<PfLocalPlayer *>(&*player[0])->ArrangeReady(bf1);
	} else {
		if(!(player[0]->GetGame().state & PfGame::me_ready)) {
			static_cast<PfLocalPlayer *>(&*player[0])->ArrangeReady(bf1);
		}
	}
}
void p2Giveup() {
	player[0]->Giveup();
	PrevPage();
}
void ClearMyBf() {
	if(!(player[0]->GetGame().state & PfGame::me_ready)) {
		bf1.clear();
		RefreshPage();
	}
}

void p10Surrender() {
	player[0]->Surrender();
}

void RefreshPage() {
	pfui::scr.PostEvent(ftxui::Event::Custom);
}

void ResizeHandler(std::pair<int, int> size) {
	std::tie(scrW, scrH) = size;
	GenerateBackground();
	RefreshPage();
}

// parse command line arguments
void processArg(int argc, char **argv) {
	if(argc < 2) // no arguments
		return;
	bool pause = 0;
	for(int i = 1; i < argc; i++) {
		string op = argv[i];
		if(op[0] != '-') { // invalid parameter
			cout << "planefight: ignoring parameter " << op << endl;
			pause = 1;
		} else if(op[1] != '-') { // short parameter
			if(op == "-v") { // version
				cout << pfUA << endl; // output version
				exit(0); // exit
			} else { // others
				cout << "planefight: unknown option " << op << endl;
				pause = 1;
			}
		} else { // long parameter, unexpected
			cout << "planefight: unknown option " << op << endl;
			pause = 1;
		}
	}
	if(pause) { // pause
		cout << "Press enter to continue...";
		cin.get();
	}
}

void PfAtExit() {
}

int main(int argc, char **argv) {
	freopen("planefight.log", "w", stderr);

	processArg(argc, argv); // parse command line arguments
	// atexit(PfAtExit);
	PfLocaleInit("");

	// GenerateBackground();
	NextPage(PfPage::welcome);

	pfui::Build();
	pfui::Loop();
	return 0;
}
