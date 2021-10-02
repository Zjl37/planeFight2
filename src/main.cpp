#include <ctime>
#include "pfRemotePlayer.hpp"
#include "pfLocale.hpp"
#include "pfAI.hpp"
#include "pfConio.hpp"

using namespace std;

const int PF_NMARKER = 23;

string marker[PF_NMARKER] = {
	"\u2501", "\u2503", "\u254b", "\u2523", "\u252b", "\u2533", "\u253b",
	"\u2500", "\u2502", "\u253c", "\u251c", "\u2524", "\u252c", "\u2534",
	"\u2550", "\u2551", "\u256c", "\u2560", "\u2563", "\u2566", "\u2569",
	"\uff1f", "\uff01"
};

string mapEdge[256] = {
	"\u2500", "\u2501", "\u2502", "\u2503", "\u250c", "\u250f",
	"\u2550", "\u2551", "\u2554", "\u2557", "\u255a", "\u255d",
	"\u2510", "\u2514", "\u2518", "\u2513", "\u2517", "\u251b"
};
int bdcOpt = 1;

mt19937 rng(time(nullptr)); // random number generator by MT19937 algorithm

bool isFirst;
int p2npl;

int tab[16], nue;
pfLabel ue[128];
extern std::string errMsg;
std::string p1name;
pfBF bg, bf1;

string sIP;

mutex mtxCout;

PfConioContext vtIn;

enum {
	pf_local_game,
	pf_remote_game_client,
	pf_remote_game_server,
} curGameType;

void uptCursorState() {
	lock_guard<mutex> _lg(mtxCout);
	if(stPage.top() == PfPage::welcome || stPage.top() == PfPage::client_init)
		showCursor();
	else
		hideCursor();
}

bool _SetPage(PfPage x) {
	// TODO: _SetPage should not do anything about control.
	vtIn.fTextInputMode = 0;
	switch(x) {
	case PfPage::welcome:
		vtIn.fTextInputMode = 1;
		break;
	case PfPage::prepare:
		memset(tab, 0, sizeof tab);
		p2npl = 0;
		break;
	case PfPage::game:
		memset(tab, 0, sizeof tab);
		break;
	case PfPage::client_init:
		vtIn.fTextInputMode = 1;
		break;
	default: break;
	}
	return true;
}

#define BF1_X() 4
#define BF1_Y() 5
#define BF2_X() (scrW - 4 - (player[0]->GetOthersBF().w) * 2)
#define BF2_Y() 5

void DrawBF() {
	if(stPage.top() == PfPage::prepare) {
		DrawBF(bf1, player[0]->GetOthersBF(), BF1_X(), BF1_Y(), BF2_X(), BF2_Y());
	} else {
		DrawBF(player[0]->GetMyBF(), player[0]->GetOthersBF(), BF1_X(), BF1_Y(), BF2_X(), BF2_Y());
	}
}

inline void DrawPlayerNames() {
	gotoXY(BF1_X(), 2), std::cout << player[0]->GetName() << std::endl;
	gotoXY(BF2_X(), 2), std::cout << player[1]->GetName() << std::endl;
}

void drawUiElem() {
	lock_guard<mutex> _lg(mtxCout);
	setDefaultColor(), clear();
	if(stPage.top() <= PfPage(1))
		bg.Draw(0, 0, false);
	for(int i = 1; i <= nue; i++)
		ue[i].draw();
	switch(stPage.top()) {
	case PfPage::welcome:
		gotoXY(ue[4].right(), ue[4].y);
		break;
	case PfPage::prepare:
		DrawBF();
		DrawPlayerNames();
		if(tab[0] == 0)
			drawPark(tab[1], curGame.h + 10);
		break;
	case PfPage::adjust_map:
		box(BF1_X(), BF1_Y(), curGame.w * 2, curGame.h, 2);
		break;
	case PfPage::game:
		DrawBF();
		DrawPlayerNames();
		gotoXY(scrW - 10, 1), cout << "#" << player[0]->GetGame().turn;
		break;
	case PfPage::gameover:
		DrawBF();
		DrawPlayerNames();
		gotoXY(scrW - 10, 1), cout << "#" << player[0]->GetGame().turn;
		break;
	case PfPage::client_init:
		gotoXY(5, 7), cout << sIP;
		break;
	case PfPage::error:
		banner(errMsg, scrH / 3, white, red);
		return;
	default: break;
	}
}

void GenerateBackground() {
	bg.resize(scrW / 2, scrH);
	for(int i = 0; i < (int)bg.w * bg.h / 25; i++)
		bg.placeplane(rng() % bg.w, rng() % bg.h, rng() & 3, curGame.cw);
}

void p0InputOK() {
	if(p1name.empty()) {
		setDefaultColor();
		RefreshPage();
		return;
	}
	setDefaultColor();
	NextPage(PfPage::main);
}

void StartLocalGame() {
	player[0].reset(new PfLocalPlayer(p1name));
	player[1].reset(new PfAI());
	player[0]->other = player[1];
	player[1]->other = player[0];
	curGameType = pf_local_game;
	isFirst = rng() & 1;
	bf1.clear();
	NextPage(PfPage::prepare);
}

void StartServer() {
	player[0].reset(new PfLocalPlayer(p1name));
	curGameType = pf_remote_game_server;
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

void p1Play2() {
	tab[0] = !tab[0];
	RefreshPage();
}

void StartClient() {
	static bool connecting = 0;
	if(connecting) {
		return;
	} else {
		connecting = 1;
	}
	try {
		player[0].reset(new PfLocalPlayer(p1name));
		player[1] = PfCreateRemoteServer(sIP, *player[0]);
		player[0]->other = player[1];
		player[1]->other = player[0];
		curGameType = pf_remote_game_client;
	} catch(const std::string &t) {
		showErrorMsg(t);
	} catch(const boost::system::system_error &e) {
		showErrorMsg(TT("Error: Cannot connect to server. Please check if the IP is correct and if the server is running."));
	}
	connecting = 0;
}

void p2Ready() {
	if(p2npl != curGame.n) return;
	if(curGameType == pf_local_game) {
		if(!curGame.n) return;
		pfBF bf2(curGame.w, curGame.h);
		bool tmp = bf2.AutoArrange();
		if(tmp == false) {
			RefreshPage();
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
void p2SwitchCw() {
	if(curGame.cw) {
		bf1.clear();
		p2npl = 0;
		curGame.cw = false;
	} else {
		curGame.cw = true;
	}
	RefreshPage();
}
void p2AddPl(short x) {
	if(curGame.n + x < 1 || curGame.n + x > curGame.w * curGame.h / 10) {
		RefreshPage();
		return;
	}
	curGame.n += x;
	if(p2npl > curGame.n) {
		p2npl = 0;
		bf1.clear();
	}
	RefreshPage();
}
void p2Giveup() {
	player[0]->Giveup();
	PrevPage();
}
void ClearMyBf() {
	if(!(player[0]->GetGame().state & PfGame::me_ready)) {
		bf1.clear(), p2npl = 0;
		RefreshPage();
	}
}

void p10Surrender() {
	player[0]->Surrender();
}

void pfExit() {
	vtIn.fWork = 0;
}

void buildUiElem() {
	stringstream tmp("");
	tmp << left << setw(scrW) << TT(" PlaneFight - Console Game");
	ue[1] = pfLabel(tmp.str(), 0, 0, white, blue, 0, 0, false);
	switch(stPage.top()) {
	case PfPage::welcome: {
		tmp.str("");
		string t2 = TT("Welcome to planeFight Console Game!");
		tmp << setw((scrW - t2.length()) / 2) << "" << left << setw(scrW - (scrW - t2.length()) / 2) << t2;
		
		ue[2] = pfLabel(tmp.str(), 0, 5, white, pink, 0, 0, true);
		ue[3] = pfLabel(TT(" Confirm "), 2, 10, black, yellow, black, darkYellow, false);
		ue[3].clickFunc = p0InputOK;
		ue[4] = pfLabel(TT("Enter your username: ").str() + p1name, 2, 8, dfc, dbc, 0, 0, false);
		ue[5] = pfLabel(TT(" Exit "), ue[3].right() + 2, 10, white, red, white, darkRed, false);
		ue[5].clickFunc = [] { pfExit(); };

		// const int P1_NNLUE = 5;
		// ue[P1_NNLUE + 1] = pfLabel(lf[0].langName, 2, 12, dfc, dbc, grey, black, false);
		// ue[P1_NNLUE + 1].clickFunc = [] {
		// 	pfLangRead(lf[0].dir.c_str()), refreshPage();
		// };
		// for(int i = 1; i < (int)lf.size(); i++) {
		// 	if(ue[P1_NNLUE + i].right() + 2 + lf[i].langName.length() > scrW) {
		// 		ue[P1_NNLUE + 1 + i] = pfLabel(lf[i].langName, 2, ue[P1_NNLUE + i].y + 1, dfc, dbc, grey, black, false);
		// 	} else {
		// 		ue[P1_NNLUE + 1 + i] = pfLabel(lf[i].langName, ue[P1_NNLUE + i].right() + 2, ue[P1_NNLUE + i].y, dfc, dbc, grey, black, false);
		// 	}
		// 	ue[P1_NNLUE + 1 + i].clickFunc = [i] { pfLangRead(lf[i].dir.c_str()), refreshPage(); };
		// }
		// nue = P1_NNLUE + lf.size();

		nue = 5;
		break;
	}
	case PfPage::main: {
		tmp.str("");
		tmp << setw(scrW - 1) << TT(" ※ Play against computer");
		ue[2] = pfLabel(tmp.str(), 3, 3, black, white, black, lightGrey, true);
		ue[2].clickFunc = StartLocalGame;
		tmp.str("");
		tmp << setw(scrW - 1) << TT(" ※ Multiplayer game");
		ue[3] = pfLabel(tmp.str(), 6, 7, black, white, black, lightGrey, true);
		ue[3].clickFunc = p1Play2;
		tmp.str("");
		tmp << setw(scrW - 1) << TT(" ※ Gamerules / About");
		ue[4] = pfLabel(tmp.str(), 9, 11, black, white, black, lightGrey, true);
		ue[4].clickFunc = [] { NextPage(PfPage::about); };
		nue = 4;

		ue[5] = pfLabel(TT("  Exit  "), 12, 15, white, red, white, darkRed, true);
		ue[5].clickFunc = [] { pfExit(); };

		const std::string &t17 = TT("  Language  ");
		if(ue[5].right() + 2 + t17.length() < scrW)
			ue[6] = pfLabel(t17, ue[5].right() + 2, 15, black, white, black, lightGrey, true);
		else
			ue[6] = pfLabel(t17, ue[5].x, 19, black, white, black, lightGrey, true);
		ue[6].clickFunc = [] { PrevPage(); };
		nue = 6;
		if(tab[0]) {
			ue[7] = pfLabel(TT("> Start a server"), 8, 10, dfc, dbc, grey, dbc, false);
			ue[7].clickFunc = p1Play21;
			ue[8] = pfLabel(TT("> Join a game"), 8, 11, dfc, dbc, grey, dbc, false);
			ue[8].clickFunc = p1Play22;
			nue = 8;
		}
		break;
	}
	case PfPage::prepare: {
		if(curGameType == pf_local_game) {
			ue[2] = pfLabel(TT("<<Back"), 0, 1, black, yellow, black, darkYellow, false);
			ue[2].clickFunc = [] { PrevPage(); };
		} else {
			ue[2] = pfLabel(TT("<<Give up"), 0, 1, black, yellow, black, darkYellow, false);
			ue[2].clickFunc = p2Giveup;
		}
		ue[3] = pfLabel(TT(" PARK "), 3, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[3].clickFunc = [] { tab[0] = 0; RefreshPage(); };
		ue[4] = pfLabel(TT(" GAMERULES "), ue[3].right() + 2, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[4].clickFunc = [] { tab[0] = 1; RefreshPage(); };
		ue[5] = pfLabel(); // reserved for [Preferences] tab
		if(tab[0] >= 0 && tab[0] <= 2)
			ue[3 + tab[0]].fgc = black, ue[3 + tab[0]].bgc = green;
		if(p2npl == curGame.n) {
			const string tplay = curGameType == pf_local_game ? TT("  PLAY  ") : TT("  I'm ready  ");
			ue[6] = pfLabel(tplay, (scrW - tplay.length()) / 2, 10, black, yellow, black, darkYellow, true);
			ue[6].clickFunc = p2Ready;
		} else {
			ue[6] = pfLabel();
		}
		if(tab[0] == 0) {
			if(player[0]->GetGame().state & PfGame::me_ready) {
				nue = 6;
			} else {
				ue[7] = pfLabel(TT(" CLEAR "), 12, 18 + curGame.h, black, yellow, black, darkYellow, false);
				ue[7].clickFunc = ClearMyBf;
				nue = 7;
			}
		} else if(tab[0] == 1) {
			if(curGameType == pf_local_game) {
				ue[7] = pfLabel((curGame.cw ? TT("✓ Enable cross-border mode") : TT("□ Enable cross-border mode")), 4, 10 + curGame.h, dfc, dbc, grey, black, false);
				ue[7].clickFunc = p2SwitchCw;
			} else if(curGame.cw) {
				ue[7] = pfLabel(TT("✓ Cross-border mode enabled"), 4, 10 + curGame.h, dfc, dbc, grey, black, false);
			} else {
				ue[7] = pfLabel();
			}
			if(curGameType == pf_local_game) {
				ue[8] = pfLabel((curGame.cd ? TT("✓ Enable completely-destroy") : TT("□ Enable completely-destroy")), 4, 12 + curGame.h, dfc, dbc, grey, black, false);
				ue[8].clickFunc = [] { curGame.cd=!curGame.cd; RefreshPage(); };
			} else if(curGame.cd) {
				ue[8] = pfLabel(TT("✓ Completely-destroy enabled"), 4, 12 + curGame.h, dfc, dbc, grey, black, false);
			} else
				ue[8] = pfLabel();
			if(curGameType == pf_local_game) {
				ue[9] = pfLabel(TT("－"), 4, 11 + curGame.h, black, yellow, black, darkYellow, false);
				ue[9].clickFunc = [] { p2AddPl(-1); };
				tmp.str("");
				tmp << TT("Number of planes: ") << curGame.n;
				ue[10] = pfLabel(tmp.str(), ue[9].right() + 2, 11 + curGame.h, dfc, dbc, 0, 0, false);
				ue[11] = pfLabel(TT("＋"), ue[10].right() + 2, 11 + curGame.h, black, yellow, black, darkYellow, false);
				ue[11].clickFunc = [] { p2AddPl(1); };
				ue[12] = pfLabel((isFirst ? TT("○ I go first") : TT("○ The other player goes first")), 4, 14 + curGame.h, dfc, dbc, grey, black, false);
				ue[12].clickFunc = [] { isFirst = !isFirst, RefreshPage(); };
			} else {
				tmp.str("");
				tmp << TT("Number of planes: ") << curGame.n;
				ue[9] = pfLabel(tmp.str(), 4, 11 + curGame.h, dfc, dbc, 0, 0, false);
				ue[10] = pfLabel();
				ue[11] = pfLabel();
				ue[12] = pfLabel((isFirst ? TT("○ I go first") : TT("○ The other player goes first")), 4, 14 + curGame.h, dfc, dbc, grey, black, false);
			}
			tmp.str("");
			tmp << curGame.h << "*" << curGame.w;
			if(curGameType == pf_local_game)
				ue[13] = pfLabel(TT("Map size: ").str() + tmp.str(), 4, 16 + curGame.h, black, yellow, black, darkYellow, false),
				ue[13].clickFunc = [] { NextPage(PfPage::adjust_map); };
			else
				ue[13] = pfLabel(TT("Map size: ").str() + tmp.str(), 4, 16 + curGame.h, dfc, dbc, grey, black, false);
			nue = 13;
		}
		if(player[0]->GetGame().state & PfGame::other_ready) {
			const string twait = TT("The other player is ready.");
			ue[++nue] = pfLabel(twait, (scrW - twait.length()) / 2, scrH * 2 / 3, yellow, dbc, darkYellow, dbc, false);
		} else if(player[0]->GetGame().state & PfGame::me_ready) {
			const string twait = TT("Waiting the other player to get ready...");
			ue[++nue] = pfLabel(twait, (scrW - twait.length()) / 2, scrH * 2 / 3, yellow, dbc, darkYellow, dbc, false);
		}
		break;
	}
	case PfPage::adjust_map: {
		ue[2] = pfLabel(TT("<<Back"), 0, 1, black, yellow, black, darkYellow, false);
		ue[2].clickFunc = [] {
			bf1.resize(curGame.w, curGame.h);
			PrevPage();
		};
		tmp.str("");
		tmp << curGame.h << "*" << curGame.w;
		ue[3] = pfLabel(TT("Map size: ").str() + tmp.str(), 4, 2, dfc, dbc, grey, black, false);
		nue = 3;
		break;
	}
	case PfPage::about: {
		ue[2] = pfLabel(TT("<<Back"), 0, 1, black, yellow, black, darkYellow, false);
		ue[2].clickFunc = [] { PrevPage(); };
		const string t19 = TT(" Visit GitHub repo ");
		ue[3] = pfLabel(t19, (scrW - t19.length()) / 2, 10, black, yellow, black, darkYellow, false);
		ue[3].clickFunc = [] {
			system("explorer https://github.com/Zjl37/planeFight2");
			// TODO: recover console mode
			// conSetMode();
			RefreshPage();
		};
		ue[4] = pfLabel(pfUA, 1, 2, dfc, dbc, 0, 0, false);
		setDefaultColor();
		ue[5] = pfLabel(TT(" This program is open source on Github, Go to https://github.com/Zjl37/planeFight2 for gamerules and more info."), 3, 4, dfc, dbc, 0, 0, false);
		ue[6] = pfLabel();
		nue = 6;
		break;
	}
	case PfPage::game: {
		ue[2] = pfLabel(TT("<<Surrender"), 0, 1, black, yellow, black, darkYellow, false);
		ue[2].clickFunc = p10Surrender;
		ue[3] = pfLabel(TT(" ATTACK "), 3, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[3].clickFunc = [] { tab[0] = 0, RefreshPage(); };
		ue[4] = pfLabel(TT(" MARK "), ue[3].right() + 2, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[4].clickFunc = [] { tab[0] = 1, RefreshPage(); };
		ue[5] = pfLabel(TT(" ERASE "), ue[4].right() + 2, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[5].clickFunc = [] { tab[0] = 2, RefreshPage(); };
		if(tab[0] >= 0 && tab[0] <= 2)
			ue[3 + tab[0]].fgc = black, ue[3 + tab[0]].bgc = green;
		if(tab[0] == 1) {
			for(int i = 0; i < PF_NMARKER; i++) {
				ue[6 + i] = pfLabel(marker[i], 3 + i % 7 * 4, 10 + curGame.h + i / 7 * 2, i == tab[1] ? white : black, i == tab[1] ? darkYellow : yellow, white, darkYellow, false);
				ue[6 + i].clickFunc = [i] { tab[1]=i; RefreshPage(); };
			}
			nue = 5 + PF_NMARKER;
		} else
			nue = 5;
		break;
	}
	case PfPage::gameover: {
		if(player[0]->GetGame().state & PfGame::other_surrender) {
			const string t1 = TT(" The other player surrendered. ");
			ue[2] = pfLabel(t1, (scrW - t1.length()) / 2, 4, white, darkGreen, grey, darkGreen, true);
		} else if(player[0]->GetGame().state & PfGame::me_surrender) {
			const string t1 = TT(" You surrendered. ");
			ue[2] = pfLabel(t1, (scrW - t1.length()) / 2, 4, white, darkRed, grey, darkRed, true);
		} else if(player[0]->GetGame().nDestroyedMine == curGame.n) {
			const string t1 = TT(" You lose. ");
			ue[2] = pfLabel(t1, (scrW - t1.length()) / 2, 4, white, red, grey, red, true);
		} else if(player[0]->GetGame().nDestroyedOthers == curGame.n) {
			const string t1 = TT(" You won! ");
			ue[2] = pfLabel(t1, (scrW - t1.length()) / 2, 4, black, green, black, darkGreen, true);
		}
		const string tback = TT(" Back to main page ");
		ue[3] = pfLabel(tback, (scrW - tback.length()) / 2, scrH * 2 / 3, black, yellow, black, darkYellow, true);
		ue[3].clickFunc = PrevPage;
		nue = 3;
		break;
	}
	case PfPage::gamerule_setting_server: {
		ue[2] = pfLabel(TT("<<Back"), 0, 1, black, yellow, black, darkYellow, false);
		ue[2].clickFunc = PrevPage;
		ue[3] = pfLabel(TT(" GAMERULES "), 4, 5, dfc, dbc, 0, 0, false);
		const string tstart = TT("  START  ");
		ue[4] = pfLabel(tstart, (scrW - tstart.length()) / 2, scrH * 2 / 3, black, yellow, black, darkYellow, true);
		ue[4].clickFunc = StartServer;
		ue[5] = pfLabel((curGame.cw ? TT("✓ Enable cross-border mode") : TT("□ Enable cross-border mode")), 4, 7, dfc, dbc, grey, black, false);
		ue[5].clickFunc = [] {
			curGame.cw = !curGame.cw;
			RefreshPage();
		};
		ue[6] = pfLabel(TT("－"), 4, 9, black, yellow, black, darkYellow, false);
		ue[6].clickFunc = [] {
			curGame.n = max(1, curGame.n - 1);
			RefreshPage();
		};
		tmp.str("");
		tmp << TT("Number of planes: ") << curGame.n;
		ue[7] = pfLabel(tmp.str(), ue[6].right() + 2, 9, dfc, dbc, 0, 0, false);
		ue[8] = pfLabel(TT("＋"), ue[7].right() + 2, 9, black, yellow, black, darkYellow, false);
		ue[8].clickFunc = [] {
			curGame.n = min(curGame.n + 1, curGame.w * curGame.h / 10);
			RefreshPage();
		};
		ue[9] = pfLabel((curGame.cd ? TT("✓ Enable completely-destroy") : TT("□ Enable completely-destroy")), 4, 11, dfc, dbc, grey, black, false);
		ue[9].clickFunc = [] {
			curGame.cd = !curGame.cd;
			RefreshPage();
		};
		ue[10] = pfLabel((isFirst ? TT("○ I go first") : TT("○ The other player goes first")), 4, 13, dfc, dbc, grey, black, false);
		ue[10].clickFunc = [] { isFirst = !isFirst, RefreshPage(); };
		tmp.str("");
		tmp << curGame.h << "*" << curGame.w;
		ue[11] = pfLabel(TT("Map size: ").str() + tmp.str(), 4, 15, black, yellow, black, darkYellow, false),
		ue[11].clickFunc = [] { NextPage(PfPage::adjust_map); };
		nue = 11;
		break;
	}
	case PfPage::server_init: {
		ue[2] = pfLabel(TT("<<Back"), 0, 1, black, yellow, black, darkYellow, false);
		ue[2].clickFunc = [] {
			player[1].reset();
			PrevPage();
			PfServerStop();
		};
		ue[3] = pfLabel(TT("[i] Run ipconfig in command line to check your IP address and tell your friend."), 0, 4, dfc, dbc, dfc, dbc, false);
		ue[4] = pfLabel(TT("[i] Waiting for connections from client..."), 0, 6, dfc, dbc, dfc, dbc, false);
		nue = 4;
		break;
	}
	case PfPage::client_init: {
		ue[2] = pfLabel(TT("<<Back"), 0, 1, black, yellow, black, darkYellow, false);
		ue[2].clickFunc = [] {
			player[1].reset();
			PrevPage();
			PfStopConnect();
		};
		ue[3] = pfLabel(TT("[i] Please enter server IP address: "), 4, 5, dfc, dbc, 0, 0, false);
		nue = 3;
		break;
	}
	case PfPage::error: {
		ue[2] = pfLabel(TT("<<Back"), 0, 1, black, yellow, black, darkYellow, false);
		ue[2].clickFunc = PrevPage;
		nue = 2;
		return;
	}
	default: nue = 1;
	}
}

void RefreshPage() {
	uptCursorState();
	if(scrW < 70 || scrH < 28) return;
	buildUiElem();
	drawUiElem();
}

void ProcessMouseClick(int mx, int my) {
	for(int i = nue; i >= 1; i--)
		if(ue[i].click(mx, my)) {
			return;
		}
	if(stPage.top() == PfPage::prepare && tab[0] == 0) {
		if(my >= 10 + curGame.h && my <= 16 + curGame.h) {
			if(mx >= 4 && mx <= 17)
				tab[1] = 0;
			else if(mx >= 20 && mx <= 33)
				tab[1] = 1;
			else if(mx >= 36 && mx <= 49)
				tab[1] = 2;
			else if(mx >= 52 && mx <= 65)
				tab[1] = 3;
			lock_guard<mutex> _lg(mtxCout);
			drawPark(tab[1], curGame.h + 10);
		}
		if(mx >= BF1_X() && mx < BF1_X() + bf1.w * 2 && my >= BF1_Y() && my < BF1_Y() + bf1.h
		   && p2npl < curGame.n && !(player[0]->GetGame().state & PfGame::me_ready)) { 
			if(bf1.placeplane((mx - BF1_X()) >> 1, my - BF1_Y(), tab[1], curGame.cw)) {
				++p2npl;
				if(p2npl == curGame.n)
					RefreshPage(); // so that the play button will appear
			} else {
				lock_guard<mutex> _lg(mtxCout);
				setColor(red, black);
				gotoXY(2, 7 + curGame.h);
				cout << (curGame.cw ? TT("Cannot place here: planes cannot overlap nor be placed out of border.") :
                                      TT("Cannot place here: planes cannot overlap."));
			}
			lock_guard<mutex> _lg(mtxCout);
			setDefaultColor();
			clearR(0, 7 + curGame.h, scrW, 7 + curGame.h);
			DrawBF();
		}
	} else if(stPage.top() == PfPage::adjust_map) {
		int nx = (mx - BF1_X()) / 2, ny = my - BF1_Y();
		if(nx >= 5 && nx <= scrW && ny >= 5)
			if(nx != curGame.w || ny != curGame.h) {
				curGame.w = nx;
				curGame.h = ny;
				RefreshPage();
			}
	} else if(stPage.top() == PfPage::game) {
		auto &w = player[0]->GetOthersBF().w;
		auto &h = player[0]->GetOthersBF().h;
		if(mx >= BF2_X() && mx < BF2_X() + w * 2 && my >= BF2_Y() && my < BF2_Y() + h) {
			auto &bf3 = player[0]->GetOthersBF();
			if(tab[0] == 0 && player[0]->GetGame().isMyTurn() && bf3.mk[(mx - BF2_X()) / 2 + (my - BF2_Y()) * w] == black) {
				player[0]->Attack((mx - BF2_X()) / 2, my - BF2_Y());
			} else if(tab[0] == 1) {
				bf3.ch[(mx - BF2_X()) / 2 + (my - BF2_Y()) * w] = marker[tab[1]];
				lock_guard<mutex> _lg(mtxCout);
				DrawBF();
			} else if(tab[0] == 2) {
				bf3.ch[(mx - BF2_X()) / 2 + (my - BF2_Y()) * w] = "";
				lock_guard<mutex> _lg(mtxCout);
				DrawBF();
			}
		}
	}
}

void MouseHandler(int stat, int mx, int my, bool fDown) {
	// clog<<"[INFO] in "<<__PRETTY_FUNCTION__<<" stat: "<<stat<<" mx: "<<mx<<" my: "<<my<<endl;
#define IsLmbDown() ((stat & 0x3) == 0 && fDown)
#define IsRelease() ((stat & 0x3) == 3)
	if(IsLmbDown()) {
		ProcessMouseClick(mx, my);
	} else if(IsRelease()) {
		if(stPage.top() == PfPage::prepare) {
			if(mx >= BF1_X() && mx < BF1_X() + bf1.w * 2 && my >= BF1_Y() && my < BF1_Y() + bf1.h
			   && tab[0] == 0 && p2npl < curGame.n && bf1.ch[(mx - BF1_X()) / 2 + (my - BF1_Y()) * bf1.w].empty()
			   && !(player[0]->GetGame().state & PfGame::me_ready)) {
				lock_guard<mutex> _lg(mtxCout);
				DrawBF();
				setColor(grey, dbc);
				if(curGame.cw) {
					DrawPlaneCw(BF1_X() + ((mx - BF1_X()) & (short)-2), my, tab[1], BF1_X(), BF1_Y(), curGame.w, curGame.h);
				} else {
					DrawPlane(BF1_X() + ((mx - BF1_X()) & (short)-2), my, tab[1], BF1_X(), BF1_Y(), curGame.w, curGame.h);
				}
			}
		} else if(stPage.top() == PfPage::game) {
			auto &w = player[0]->GetOthersBF().w;
			auto &h = player[0]->GetOthersBF().h;
			if(mx >= BF2_X() && mx < BF2_X() + w * 2 && my >= BF2_Y() && my < BF2_Y() + h
			   && player[1]->GetOthersBF().mk[(mx - BF2_X()) / 2 + (my - BF2_Y()) * w] == 0) {
				if(mtxCout.try_lock()) {
					setDefaultColor();
					player[0]->GetOthersBF().Draw(BF2_X(), BF2_Y(), true);
					setColor(grey, dbc);
					gotoXY(BF2_X() + ((mx - BF2_X()) | 1) - 1, my), cout << "×";
					mtxCout.unlock();
				}
			}
		}
	}
}

void KeyHandler(const string &s, const vector<int> &v) {
	if(s.empty() && v.size()) {
		if(v[0] == 15) {
			RefreshPage();
		}
	} else {
		if(stPage.top() == PfPage::welcome) {
			if(s == "\n") {
				p1name = vtIn.ReadLine();
				gotoXY(2, 8);
				cout << TT("Enter your username: ") << p1name;

				ue[3]._click();
			} else if(s == "\x7f") {
				gotoXY(2, 8);
				ClearLineRight();
				cout << TT("Enter your username: ") << vtIn.PeekLine();
			}
		} else if(stPage.top() == PfPage::client_init) {
			if(s == "\n") {
				sIP = vtIn.ReadLine();
				StartClient();
			} else if(s == "\x7f") {
				gotoXY(5, 7);
				ClearLineRight();
				cout << vtIn.PeekLine();
			}
		}
	}
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
			} else if(op == "-bdc") { // set code page
				if(i == argc - 1) {
					cout << "planefight: error: expected value after -cp option." << endl;
					exit(233);
				}
				string val = argv[i + 1];
				if(val == "full") { // -bdc full
					bdcOpt = 0;
				} else if(val == "pseudofull") { // -bdc pseudofull
					bdcOpt = 1;
				} else { // others
					cout << "planefight: error: unknown value " << val << " for option " << op << "." << endl;
					exit(233);
				}
				++i;
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

void pfCmptAddBdcSp() {
	for(int i = 0; i < 21; i++) {
		marker[i].append(" ");
	}
	for(int i = 0; i < 18; i++) {
		mapEdge[i].append(" ");
	}
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 10; j++) {
			plShape[i][j].ch.append(" ");
		}
	}
}

void pfCompatibility() {
	if(bdcOpt & 1) {
		pfCmptAddBdcSp();
	}
}

void PfAtExit() {
	vtIn.WaitForHandlerThreads();
	ConReset();
}

int main(int argc, char **argv) {
	freopen("planefight.log", "w", stderr);

	processArg(argc, argv); // parse command line arguments
	ConInit(); // initialising console
	atexit(PfAtExit);
	PfLocaleInit("");

	pfCompatibility();
	GenerateBackground();
	NextPage(PfPage::welcome);

	VtEnableMouseTrackingAny();
	vtIn.mouseHandler = MouseHandler;
	vtIn.keyHandler = KeyHandler;
	vtIn.resizeHandler = ResizeHandler;
	vtIn.Work();
	// TODO:
	// window size event;
	return 0;
}