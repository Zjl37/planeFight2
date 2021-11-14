#include <ctime>
#include "pfRemotePlayer.hpp"
#include "pfLocale.hpp"
#include "pfAI.hpp"
#include "pfConio.hpp"
#include "pfUiCtrl.hpp"
#include "pfConsole.hpp"

using namespace std;

const int PF_NMARKER = 23;

string marker[PF_NMARKER] = {
	"━", "┃", "╋", "┣", "┫", "┳", "┻",
	"─", "│", "┼", "├", "┤", "┬", "┴",
	"═", "║", "╬", "╠", "╣", "╦", "╩",
	"？", "！"
};

string mapEdge[256] = {
	"─", "━", "│", "┃", "┌", "┏",
	"═", "║", "╔", "╗", "╚", "╝",
	"┐", "└", "┘", "┓", "┗", "┛"
};

mt19937 rng(time(nullptr)); // random number generator by MT19937 algorithm

bool isFirst;
int p2npl;

int tab[16], nue;
pfLabel ue[128];
// extern std::string errMsg;
// std::string p1name;
pfBF bg, bf1;

string sIP;

mutex mtxCout;

PfConioContext vtIn;

enum {
	pf_local_game,
	pf_remote_game_client,
	pf_remote_game_server,
} curGameType;

[[deprecated]] void uptCursorState() {
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

[[deprecated]]
void DrawBF() {
	if(stPage.top() == PfPage::prepare) {
		DrawBF(bf1, player[0]->GetOthersBF(), BF1_X(), BF1_Y(), BF2_X(), BF2_Y());
	} else {
		DrawBF(player[0]->GetMyBF(), player[0]->GetOthersBF(), BF1_X(), BF1_Y(), BF2_X(), BF2_Y());
	}
}

[[deprecated]]
inline void DrawPlayerNames() {
	gotoXY(BF1_X(), 2), std::cout << player[0]->GetName() << std::endl;
	gotoXY(BF2_X(), 2), std::cout << player[1]->GetName() << std::endl;
}

[[deprecated]]
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
		// banner(errMsg, scrH / 3, white, red);
		return;
	default: break;
	}
}

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
		pfBF bf2(curGame.w, curGame.h);
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

[[deprecated]] void buildUiElem() {
	// TODO: implement these using FTXUI
	switch(stPage.top()) {
	case PfPage::welcome: {
		break;
	}
	case PfPage::main: {
		break;
	}
	case PfPage::prepare: {
		ue[3] = pfLabel(TT(" PARK "), 3, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[3].clickFunc = [] { tab[0] = 0; RefreshPage(); };

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
		break;
	}
	case PfPage::about: {
		break;
	}
	case PfPage::game: {
		// ue[3] = pfLabel(TT(" ATTACK "), 3, 8 + curGame.h, white, darkGreen, black, green, false);
		// ue[3].clickFunc = [] { tab[0] = 0, RefreshPage(); };
		ue[4] = pfLabel(TT(" MARK "), ue[3].right() + 2, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[4].clickFunc = [] { tab[0] = 1, RefreshPage(); };
		ue[5] = pfLabel(TT(" ERASE "), ue[4].right() + 2, 8 + curGame.h, white, darkGreen, black, green, false);
		ue[5].clickFunc = [] { tab[0] = 2, RefreshPage(); };
		break;
	}
	case PfPage::gameover: {
		break;
	}
	case PfPage::gamerule_setting_server: {
		break;
	}
	case PfPage::server_init: {
		break;
	}
	case PfPage::client_init: {
		break;
	}
	case PfPage::error: {
		return;
	}
	default: nue = 1;
	}
}

void RefreshPage() {
	pfui::scr.PostEvent(ftxui::Event::Custom);
}

[[deprecated]] void ProcessMouseClick(int mx, int my) {
}

[[deprecated]] void MouseHandler(int stat, int mx, int my, bool fDown) {
}

[[deprecated]] void KeyHandler(const string &s, const vector<int> &v) {
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

[[deprecated]] void pfCmptAddBdcSp() {
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

void PfAtExit() {
	vtIn.WaitForHandlerThreads();
	ConReset();
}

int main(int argc, char **argv) {
	freopen("planefight.log", "w", stderr);

	processArg(argc, argv); // parse command line arguments
	// ConInit(); // initialising console
	// atexit(PfAtExit);
	PfLocaleInit("");

	// GenerateBackground();
	NextPage(PfPage::welcome);

	pfui::Build();
	pfui::Loop();
	return 0;
}
