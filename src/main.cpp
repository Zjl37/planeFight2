#include <ctime>
#include "pfRemotePlayer.hpp"
#include "pfGame.hpp"
#include "pfUI.hpp"
#include "pfLang.hpp"
#include "pfAI.hpp"
#include "vtsFilter.hpp"

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
const int P1_NNLUE = 5;

int bdcOpt = 1;

mt19937 rng(time(nullptr)); // random number generator by MT19937 algorithm

bool isFirst;
int p2npl;
int p10exchgMapLn;

int tab[16], nue;
pfLabel ue[128];
extern pfTextElem errMsg;
pfTextElem p1name;
pfBF bg, bf1;

string sIP;

mutex mtxCout;

VtsInputFilter vtIn;

enum {
	pf_local_game,
	pf_remote_game_client,
	pf_remote_game_server,
} curGameType;

// TODO: change this, the asio way
bool GetIP() {
	static char buf[65536];
	int ret = gethostname(buf, 65536);
	if(ret == -1) return false;
	struct hostent *host;
	host = gethostbyname(buf);
	if(host == NULL) return false;
	sIP = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);
	return true;
}

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
	gotoXY(BF1_X(), 2), std::cout << player[0]->GetName().s << std::endl;
	gotoXY(BF2_X(), 2), std::cout << player[1]->GetName().s << std::endl;
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

void p0GenBg() {
	bg.resize(scrW / 2, scrH);
	for(int i = 0; i < (int)bg.w * bg.h / 25; i++)
		bg.placeplane(rng() % bg.w, rng() % bg.h, rng() & 3, curGame.cw);
}

void p0InputOK() {
	if(p1name.s.empty()) {
		setDefaultColor();
		refreshPage();
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
	try {
		player[0].reset(new PfLocalPlayer(p1name));
		PfServerInit();
		curGameType = pf_remote_game_server;
		GetIP();
		NextPage(PfPage::server_init);
	} catch(const pfTextElem &t) {
		showErrorMsg(t);
		return;
	}
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
	refreshPage();
}

void PfClientConnect() {
	try {
		player[0].reset(new PfLocalPlayer(p1name));
		player[1] = PfCreateRemoteServer(sIP, *player[0]);
		player[0]->other = player[1];
		player[1]->other = player[0];
		curGameType = pf_remote_game_client;
	} catch(const pfTextElem &t) {
		showErrorMsg(t);
		return;
	}
}

void p2Ready() {
	if(p2npl != curGame.n) return;
	if(curGameType == pf_local_game) {
		if(!curGame.n) return;
		pfBF bf2(curGame.w, curGame.h);
		bool tmp = bf2.AutoArrange();
		if(tmp == false) {
			refreshPage();
			return;
		}
		unsigned gameId = rng();
		player[0]->NewGame(curGame, gameId, isFirst);
		player[1]->NewGame(curGame, gameId, !isFirst);

		dynamic_cast<PfAI*>(&*player[1])->ArrangeReady(bf2);
		dynamic_cast<PfLocalPlayer *>(&*player[0])->ArrangeReady(bf1);
	} else {
		if(!(player[0]->GetGame().state & PfGame::me_ready)) {
			dynamic_cast<PfLocalPlayer *>(&*player[0])->ArrangeReady(bf1);
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
	refreshPage();
}
void p2AddPl(short x) {
	if(curGame.n + x < 1 || curGame.n + x > curGame.w * curGame.h / 10) {
		refreshPage();
		return;
	}
	curGame.n += x;
	if(p2npl > curGame.n) {
		p2npl = 0;
		bf1.clear();
	}
	refreshPage();
}
void p2Giveup() {
	player[0]->Giveup();
	PrevPage();
}
void ClearMyBf() {
	if(!(player[0]->GetGame().state & PfGame::me_ready)) {
		bf1.clear(), p2npl = 0;
		refreshPage();
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
	tmp << left << setw(scrW + text[0].d) << text[0].s; // inner title
	ue[1] = pfLabel(pfTextElem(tmp.str(), text[0].d), 0, 0, white, blue, 0, 0, false);
	switch(stPage.top()) {
	case PfPage::welcome: {
		tmp.str("");
		tmp << setw((scrW - text[2].len()) / 2) << "" << left << setw(scrW + text[2].d - (scrW - text[2].len()) / 2) << text[2].s;
		ue[2] = pfLabel(pfTextElem(tmp.str(), text[2].d), 0, 5, white, pink, 0, 0, true);
		ue[3] = pfLabel(text[7], 2, 10, black, yellow, black, darkYellow, false);
		ue[3].clickFunc = p0InputOK;
		ue[4] = pfLabel(text[6] + p1name, 2, 8, dfc, dbc, 0, 0, false);
		ue[5] = pfLabel(text[92], ue[3].right() + 2, 10, white, red, white, darkRed, false);
		ue[5].clickFunc = [] { pfExit(); };

		ue[P1_NNLUE + 1] = pfLabel(lf[0].langName, 2, 12, dfc, dbc, grey, black, false);
		ue[P1_NNLUE + 1].clickFunc = [] {
			pfLangRead(lf[0].dir.c_str()), refreshPage();
		};
		for(int i = 1; i < (int)lf.size(); i++) {
			if(ue[P1_NNLUE + i].right() + 2 + lf[i].langName.len() > scrW) {
				ue[P1_NNLUE + 1 + i] = pfLabel(lf[i].langName, 2, ue[P1_NNLUE + i].y + 1, dfc, dbc, grey, black, false);
			} else {
				ue[P1_NNLUE + 1 + i] = pfLabel(lf[i].langName, ue[P1_NNLUE + i].right() + 2, ue[P1_NNLUE + i].y, dfc, dbc, grey, black, false);
			}
			ue[P1_NNLUE + 1 + i].clickFunc = [i] { pfLangRead(lf[i].dir.c_str()), refreshPage(); };
		}
		nue = P1_NNLUE + lf.size();
		break;
	}
	case PfPage::main: {
		tmp.str("");
		tmp << setw(scrW - 1 + text[8].d) << text[8].s;
		ue[2] = pfLabel(pfTextElem(tmp.str(), text[8].d), 3, 3, black, white, black, lightGrey, true);
		ue[2].clickFunc = StartLocalGame;
		tmp.str("");
		tmp << setw(scrW - 1 + text[9].d) << text[9].s;
		ue[3] = pfLabel(pfTextElem(tmp.str(), text[9].d), 6, 7, black, white, black, lightGrey, true);
		ue[3].clickFunc = p1Play2;
		tmp.str("");
		tmp << setw(scrW - 1 + text[10].d) << text[10].s;
		ue[4] = pfLabel(pfTextElem(tmp.str(), text[10].d), 9, 11, black, white, black, lightGrey, true);
		ue[4].clickFunc = [] { NextPage(PfPage::about); };
		nue = 4;

		ue[5] = pfLabel(text[16], 12, 15, white, red, white, darkRed, true);
		ue[5].clickFunc = [] { pfExit(); };
		if(ue[5].right() + 2 + text[17].len() < scrW)
			ue[6] = pfLabel(text[17], ue[5].right() + 2, 15, black, white, black, lightGrey, true);
		else
			ue[6] = pfLabel(text[17], ue[5].x, 19, black, white, black, lightGrey, true);
		ue[6].clickFunc = [] { PrevPage(); };
		nue = 6;
		if(tab[0]) {
			ue[7] = pfLabel(text[20], 8, 10, dfc, dbc, grey, dbc, false);
			ue[7].clickFunc = p1Play21;
			ue[8] = pfLabel(text[21], 8, 11, dfc, dbc, grey, dbc, false);
			ue[8].clickFunc = p1Play22;
			nue = 8;
		}
		break;
	}
	case PfPage::prepare: {
		if(curGameType == pf_local_game) {
			ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
			ue[2].clickFunc = [] { PrevPage(); };
		} else {
			ue[2] = pfLabel(text[76], 0, 1, black, yellow, black, darkYellow, false); // give up
			ue[2].clickFunc = p2Giveup;
		}
		ue[3] = pfLabel(text[22], 3, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[3].clickFunc = [] { tab[0]=0; refreshPage(); };
		ue[4] = pfLabel(text[23], ue[3].x + text[22].len() + 2, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[4].clickFunc = [] { tab[0]=1; refreshPage(); };
		ue[5] = pfLabel(); // reserved for [Preferences] tab
		if(tab[0] >= 0 && tab[0] <= 2)
			ue[3 + tab[0]].fgc = black, ue[3 + tab[0]].bgc = green;
		if(p2npl == curGame.n)
			ue[6] = pfLabel(text[curGameType == pf_local_game ? 25 : 77], (scrW - text[curGameType == pf_local_game ? 25 : 77].len()) / 2, 10, black, yellow, black, darkYellow, true);
		else
			ue[6] = pfLabel();
		ue[6].clickFunc = p2Ready;
		if(tab[0] == 0) {
			if(player[0]->GetGame().state & PfGame::me_ready) {
				nue = 6;
			} else {
				ue[7] = pfLabel(text[26], 12, 18 + curGame.h, black, yellow, black, darkYellow, false);
				ue[7].clickFunc = ClearMyBf;
				nue = 7;
			}
		} else if(tab[0] == 1) {
			if(curGameType == pf_local_game) {
				ue[7] = pfLabel((curGame.cw ? text[30] : text[29]) + text[31], 4, 10 + curGame.h, dfc, dbc, grey, black, false);
				ue[7].clickFunc = p2SwitchCw;
			} else if(curGame.cw) {
				ue[7] = pfLabel(text[74], 4, 10 + curGame.h, dfc, dbc, grey, black, false);
			} else {
				ue[7] = pfLabel();
			}
			if(curGameType == pf_local_game) {
				ue[8] = pfLabel((curGame.cd ? text[30] : text[29]) + text[83], 4, 12 + curGame.h, dfc, dbc, grey, black, false);
				ue[8].clickFunc = [] { curGame.cd=!curGame.cd; refreshPage(); };
			} else if(curGame.cd) {
				ue[8] = pfLabel(text[84], 4, 12 + curGame.h, dfc, dbc, grey, black, false);
			} else
				ue[8] = pfLabel();
			if(curGameType == pf_local_game) {
				ue[9] = pfLabel(text[32], 4, 11 + curGame.h, black, yellow, black, darkYellow, false);
				ue[9].clickFunc = [] { p2AddPl(-1); };
				tmp.str("");
				tmp << text[33].s << curGame.n;
				ue[10] = pfLabel(pfTextElem(tmp.str(), text[33].d), ue[9].right() + 2, 11 + curGame.h, dfc, dbc, 0, 0, false);
				ue[11] = pfLabel(text[34], ue[10].right() + 2, 11 + curGame.h, black, yellow, black, darkYellow, false);
				ue[11].clickFunc = [] { p2AddPl(1); };
				ue[12] = pfLabel(text[89] + (isFirst ? text[87] : text[88]), 4, 14 + curGame.h, dfc, dbc, grey, black, false);
				ue[12].clickFunc = [] { isFirst = !isFirst, refreshPage(); };
			} else {
				tmp.str("");
				tmp << text[75].s << curGame.n;
				ue[9] = pfLabel(pfTextElem(tmp.str(), text[75].d), 4, 11 + curGame.h, dfc, dbc, 0, 0, false);
				ue[10] = pfLabel();
				ue[11] = pfLabel();

				// NOTE: Due to a protocol design flaw, a client in a remote
				// game does not know its 'isFirst' here. This lable is
				// disabled until we upgrade the network protocol.
				ue[12] = pfLabel();
				// ue[12] = pfLabel(text[89] + (isFirst ? text[87] : text[88]), 4, 14 + curGame.h, dfc, dbc, grey, black, false);
			}
			tmp.str("");
			tmp << curGame.h << "*" << curGame.w;
			if(curGameType == pf_local_game)
				ue[13] = pfLabel(text[90] + tmp.str(), 4, 16 + curGame.h, black, yellow, black, darkYellow, false),
				ue[13].clickFunc = [] { NextPage(PfPage::adjust_map); };
			else
				ue[13] = pfLabel(text[90] + tmp.str(), 4, 16 + curGame.h, dfc, dbc, grey, black, false);
			nue = 13;
		}
		if(player[0]->GetGame().state & PfGame::other_ready) {
			ue[++nue] = pfLabel(text[91].s, (scrW - text[91].len()) / 2, scrH * 2 / 3, yellow, dbc, darkYellow, dbc, false);
		} else if(player[0]->GetGame().state & PfGame::me_ready) {
			ue[++nue] = pfLabel(text[82].s, (scrW - text[82].len()) / 2, scrH * 2 / 3, yellow, dbc, darkYellow, dbc, false);
		}
		break;
	}
	case PfPage::adjust_map: {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] {
			bf1.resize(curGame.w, curGame.h);
			PrevPage();
		};
		tmp.str("");
		tmp << curGame.h << "*" << curGame.w;
		ue[3] = pfLabel(text[90] + tmp.str(), 4, 2, dfc, dbc, grey, black, false);
		nue = 3;
		break;
	}
	case PfPage::about: {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] { PrevPage(); };
		ue[3] = pfLabel(text[19], (scrW - text[19].len()) / 2, 10, black, yellow, black, darkYellow, false);
		ue[3].clickFunc = [] {
			system("explorer https://github.com/Zjl37/planeFight2");
			// TODO: recover console mode
			// conSetMode();
			refreshPage();
		};
		ue[4] = pfLabel(text[12] + pfTextElem(pfVersion) + text[13], 1, 2, dfc, dbc, 0, 0, false);
		setDefaultColor();
		ue[5] = pfLabel(text[14], 3, 4, dfc, dbc, 0, 0, false);
		ue[6] = pfLabel(text[15], 3, 5, dfc, dbc, 0, 0, false);
		nue = 6;
		break;
	}
	case PfPage::game: {
		ue[2] = pfLabel(text[38], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = p10Surrender;
		ue[3] = pfLabel(text[39], 3, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[3].clickFunc = [] { tab[0] = 0, refreshPage(); };
		ue[4] = pfLabel(text[49], ue[3].right() + 2, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[4].clickFunc = [] { tab[0] = 1, refreshPage(); };
		ue[5] = pfLabel(text[50], ue[4].right() + 2, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[5].clickFunc = [] { tab[0] = 2, refreshPage(); };
		if(tab[0] >= 0 && tab[0] <= 2)
			ue[3 + tab[0]].fgc = black, ue[3 + tab[0]].bgc = green;
		if(tab[0] == 1) {
			for(int i = 0; i < PF_NMARKER; i++) {
				ue[6 + i] = pfLabel(pfTextElem(marker[i], 1), 3 + i % 7 * 4, 10 + curGame.h + i / 7 * 2, i == tab[1] ? white : black, i == tab[1] ? darkYellow : yellow, white, darkYellow, false);
				ue[6 + i].clickFunc = [i] { tab[1]=i; refreshPage(); };
			}
			nue = 5 + PF_NMARKER;
		} else
			nue = 5;
		break;
	}
	case PfPage::gameover: {
		if(player[0]->GetGame().state & PfGame::other_surrender)
			ue[2] = pfLabel(text[47], (scrW - text[47].len()) / 2, 4, white, darkGreen, grey, darkGreen, true);
		else if(player[0]->GetGame().state & PfGame::me_surrender)
			ue[2] = pfLabel(text[46], (scrW - text[46].len()) / 2, 4, white, darkRed, grey, darkRed, true);
		else if(player[0]->GetGame().nDestroyedMine == curGame.n)
			ue[2] = pfLabel(text[45], (scrW - text[45].len()) / 2, 4, white, red, grey, red, true);
		else if(player[0]->GetGame().nDestroyedOthers == curGame.n)
			ue[2] = pfLabel(text[44], (scrW - text[44].len()) / 2, 4, black, green, black, darkGreen, true);
		else
			ue[2] = pfLabel();
		ue[3] = pfLabel(text[48], (scrW - text[48].len()) / 2, scrH * 2 / 3, black, yellow, black, darkYellow, true);
		ue[3].clickFunc = PrevPage;
		nue = 3;
		break;
	}
	case PfPage::gamerule_setting_server: {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = PrevPage;
		ue[3] = pfLabel(text[23], 4, 5, dfc, dbc, 0, 0, false);
		ue[4] = pfLabel(text[25], (scrW - text[25].len()) / 2, scrH * 2 / 3, black, yellow, black, darkYellow, true);
		ue[4].clickFunc = StartServer;
		ue[5] = pfLabel((curGame.cw ? text[30] : text[29]) + text[31], 4, 7, dfc, dbc, grey, black, false);
		ue[5].clickFunc = [] {
			curGame.cw = !curGame.cw;
			refreshPage();
		};
		ue[6] = pfLabel(text[32], 4, 9, black, yellow, black, darkYellow, false);
		ue[6].clickFunc = [] {
			curGame.n = max(1, curGame.n - 1);
			refreshPage();
		};
		tmp.str("");
		tmp << text[33].s << curGame.n;
		ue[7] = pfLabel(pfTextElem(tmp.str(), text[33].d), ue[6].right() + 2, 9, dfc, dbc, 0, 0, false);
		ue[8] = pfLabel(text[34], ue[7].right() + 2, 9, black, yellow, black, darkYellow, false);
		ue[8].clickFunc = [] {
			curGame.n = min(curGame.n + 1, curGame.w * curGame.h / 10);
			refreshPage();
		};
		ue[9] = pfLabel((curGame.cd ? text[30] : text[29]) + text[83], 4, 11, dfc, dbc, grey, black, false);
		ue[9].clickFunc = [] {
			curGame.cd = !curGame.cd;
			refreshPage();
		};
		ue[10] = pfLabel(text[89] + (isFirst ? text[87] : text[88]), 4, 13, dfc, dbc, grey, black, false);
		ue[10].clickFunc = [] { isFirst = !isFirst, refreshPage(); };
		tmp.str("");
		tmp << curGame.h << "*" << curGame.w;
		ue[11] = pfLabel(text[90] + tmp.str(), 4, 15, black, yellow, black, darkYellow, false),
		ue[11].clickFunc = [] { NextPage(PfPage::adjust_map); };
		nue = 11;
		break;
	}
	case PfPage::server_init: {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] {
			player[1].reset();
			PfServerStop();
			PrevPage();
		};
		if(sIP.empty())
			ue[3] = pfLabel(text[55], 0, 4, dfc, dbc, dfc, dbc, false);
		else
			ue[3] = pfLabel(text[54] + sIP, 0, 4, dfc, dbc, dfc, dbc, false);
		ue[4] = pfLabel(text[61], 0, 6, dfc, dbc, dfc, dbc, false);
		nue = 4;
		break;
	}
	case PfPage::client_init: {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] {
			player[1].reset();
			PrevPage();
		};
		ue[3] = pfLabel(text[67], 4, 5, dfc, dbc, 0, 0, false);
		nue = 3;
		break;
	}
	case PfPage::error: {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = PrevPage;
		nue = 2;
		return;
	}
	default: nue = 1;
	}
}

void refreshPage() {
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
					refreshPage(); // so that the play button will appear
			} else {
				lock_guard<mutex> _lg(mtxCout);
				setColor(red, black);
				gotoXY(2, 7 + curGame.h), cout << text[curGame.cw ? 27 : 28].s;
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
				refreshPage();
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
					gotoXY(BF2_X() + ((mx - BF2_X()) | 1) - 1, my), cout << text[40].s;
					mtxCout.unlock();
				}
			}
		}
	}
}

void KeyHandler(const string &s, const vector<int> &v) {
	if(s.empty() && v.size()) {
		if(v[0] == 15) {
			refreshPage();
		}
	} else {
		if(stPage.top() == PfPage::welcome) {
			if(s == "\n") {
				p1name.s = vtIn.ReadLine();
				gotoXY(2, 8);
				cout << text[6].s << p1name.s;
				auto pos = getXY();
				p1name.d = p1name.s.length() - (pos.first - (2 + text[6].len()) + (pos.second - 8) * scrW);

				ue[3]._click();
			} else if(s == "\x7f") {
				gotoXY(2, 8);
				ClearLineRight();
				cout << text[6].s << vtIn.PeekLine();
			}
		} else if(stPage.top() == PfPage::client_init) {
			if(s == "\n") {
				sIP = vtIn.ReadLine();
				PfClientConnect();
			} else if(s == "\x7f") {
				gotoXY(5, 7);
				ClearLineRight();
				cout << vtIn.PeekLine();
			}
		}
	}
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
				cout << pfVerStr << endl; // output version
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
	// srand(time(0)); // deprecated
	ConInit(); // initialising console
	atexit(PfAtExit);
	string langDir;
	{
		char buf[65536];
		GetModuleFileNameA(NULL, buf, 65536);
		langDir = buf;
	}
	while(*langDir.rbegin() != '\\')
		langDir.pop_back();
	langDir.append("lang\\");
	if(!pfLangDetect(langDir)) {
		cout << pfVerStr << endl;
		setColor(red, black);
		cout << "Language file not found! Make sure folder \"lang\" is in the same directory as the executeable is. Or go to https://github.com/Zjl37/planeFight2 and re-download the game." << endl;
		setDefaultColor();
		return 1;
	}
	pfCompatibility();
	p0GenBg();
	NextPage(PfPage::welcome);

	VtEnableMouseTrackingAny();
	vtIn.mouseHandler = MouseHandler;
	vtIn.keyHandler = KeyHandler;
	vtIn.Work();
	// TODO:
	// window size event;
	return 0;
}