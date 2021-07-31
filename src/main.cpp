#include <cstdio>
#include <ctime>
#include <future>
#include <vector>
#include <random>
#include <sstream>
#include <mutex>
#include <winsock2.h>
#include "pfGame.hpp"
#include "pfUI.hpp"
#include "pfLang.hpp"
#include "pfAI.hpp"
#include "vtsFilter.hpp"

using namespace std;

#define pfVersion "2.4"
#define pfVerStr "planefight 2.4"

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
int prevPage;
int page, tab[16], nue, turn;
pfLabel ue[128];
pfTextElem errMsg;

int p2npl, p2isP1Ready, p2isP2Ready;
int p10des1, p10des2, p10srd;
int p10exchgMapLn;
WSADATA wsaData;
char buf[65536], sendbuf[65536] = "pf", tmpbuf[65536];
string sIP;

promise<short> p10AttackResPromise;
thread tSockServer, tSockClient;
mutex mtxCout;

VtsInputFilter vtIn;

void setPage(int);

int pfCheckVer(const string &s) {
	stringstream curVer(pfVerStr), iVer(s);
	char ch;
	int curVerSec1 = 0, iVerSec1 = 0, curVerSec2 = 0, iVerSec2 = 0;
	string curGName, iGName;
	curVer >> curGName >> curVerSec1 >> ch >> curVerSec2;
	iVer >> iGName >> iVerSec1 >> ch >> iVerSec2;
	if(curGName != iGName || curVerSec1 != iVerSec1)
		return -1;
	if(curVerSec2 != iVerSec2) return 1;
	return 0;
}

void showErrorMsg(const pfTextElem &t, int rpage) {
	errMsg = t;
	setPage(rpage ^ 0x80000000);
}

bool getIP() {
	int ret = gethostname(buf, 65536);
	if(ret == -1) return false;
	struct hostent *host;
	host = gethostbyname(buf);
	if(host == NULL) return false;
	sIP = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);
	return true;
}
SOCKET sockServer, sockClient;
SOCKADDR_IN addrServer, addrClient;
int hg;

// Todo: examine this.
bool pfCheckMsg(const char *msg, const char *i) {
	if(msg[0] != 'p' || msg[1] != 'f') {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71], 1);
		return false;
	}
	if(*(int *)(msg + 2) != hg) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71], 1);
		return false;
	}
	if(!i) return true;
	int len = strlen(i);
	memcpy(tmpbuf, msg + 6, len);
	tmpbuf[len] = 0;
	if(strcmp(tmpbuf, i)) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71], 1);
		return false;
	}
	return true;
}
void pfSockHandler();
bool pfServerAccept() {
	int len = sizeof(SOCKADDR), ret = 0;
	sockClient = accept(sockServer, (SOCKADDR *)&addrClient, &len);
	if(sockClient == INVALID_SOCKET)
		return false;
	closesocket(sockServer);
	gotoXY(0, getY() + 1), cout << text[63].s;
	ret = recv(sockClient, buf, 65536, 0);
	// check return value
	ret = pfCheckVer(buf);
	if(ret == -1) {
		strcpy(sendbuf + 6, "!ver");
		send(sockClient, sendbuf, 11, 0);
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[64] + (string)buf, 1);
		return false;
	} else if(ret == 1) {
		gotoXY(0, getY() + 1), cout << text[65].s << buf;
	}
	hg = *(int *)(sendbuf + 2) = rng();
	strcpy(sendbuf + 6, "hello");
	ret = send(sockClient, sendbuf, 11, 0);
	// check ret
	strcpy(sendbuf + 6, "gameinfo");
	memcpy(sendbuf + 14, &curGame, sizeof curGame);
	ret = send(sockClient, sendbuf, 14 + sizeof curGame, 0);
	// check ret
	strcpy(sendbuf + 6, "name");
	*(int *)(sendbuf + 10) = playername.d;
	strcpy(sendbuf + 14, playername.s.c_str());
	ret = send(sockClient, sendbuf, 15 + playername.s.length(), 0);
	// check ret
	ret = recv(sockClient, buf, 65535, 0);
	// check ret
	if(!pfCheckMsg(buf, "name"))
		return false;
	enemyname = pfTextElem(string(buf + 14), *(int *)(buf + 10));
	setPage(2);
	closesocket(sockServer);
	if(tSockClient.joinable()) tSockClient.join();
	tSockClient = thread(pfSockHandler);
	return true;
}
bool pfServerInit() {
	setDefaultColor(), clear();
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		showErrorMsg(text[52], 1);
		return false;
	}
	sIP = "";
	getIP();
	sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockServer == INVALID_SOCKET) {
		WSACleanup();
		showErrorMsg(text[56], 1);
		return false;
	}
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(51937);
	int ret = bind(sockServer, (SOCKADDR *)&addrServer, sizeof(SOCKADDR));
	if(ret == SOCKET_ERROR) {
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[58], 1);
		return false;
	}
	ret = listen(sockServer, 1);
	if(ret == SOCKET_ERROR) {
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[60], 1);
		return false;
	}
	if(tSockServer.joinable()) tSockServer.join();
	tSockServer = thread(pfServerAccept);
	return true;
}
bool pfClientInit() {
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		showErrorMsg(text[52], 1);
		return false;
	}
	gotoXY(0, 3), cout << text[53].s;
	sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockServer == INVALID_SOCKET) {
		WSACleanup();
		showErrorMsg(text[66], 1);
		return false;
	}
	gotoXY(0, getY() + 1), cout << text[67].s;
	gotoXY(0, getY() + 1);
	return true;
}
bool pfClientConnect() {
	addrServer.sin_addr.S_un.S_addr = inet_addr(sIP.c_str());
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(51937);
	int ret = connect(sockClient, (SOCKADDR *)&addrServer, sizeof(SOCKADDR));
	if(ret == SOCKET_ERROR) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[68], 1);
		return false;
	}
	gotoXY(0, getY() + 1), cout << text[69].s;
	ret = send(sockClient, pfVerStr, strlen(pfVerStr) + 1, 0);
	// check ret
	ret = recv(sockClient, buf, 11, 0);
	buf[11] = 0;
	if(strcmp(buf + 6, "!ver") == 0) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[70], 1);
		return false;
	} else if(buf[0] != 'p' || buf[1] != 'f') {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71], 1);
		return false;
	} else if(strcmp(buf + 6, "hello")) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71], 1);
		return false;
	}
	hg = *(int *)(sendbuf + 2) = *(int *)(buf + 2);
	ret = recv(sockClient, buf, 14 + sizeof curGame, 0);
	if(!pfCheckMsg(buf, "gameinfo"))
		return false;
	memcpy(&curGame, buf + 14, sizeof curGame);
	curGame.d = -1;
	strcpy(sendbuf + 6, "name");
	*(int *)(sendbuf + 10) = playername.d;
	strcpy(sendbuf + 14, playername.s.c_str());
	ret = send(sockClient, sendbuf, 15 + playername.s.length(), 0);
	// check ret
	ret = recv(sockClient, buf, 65535, 0);
	// check ret
	if(!pfCheckMsg(buf, "name"))
		return false;
	enemyname = pfTextElem(string(buf + 14), *(int *)(buf + 10));
	return true;
}

void pfExchangeMap() {
	p10exchgMapLn = 0;
	strcpy(sendbuf + 6, "mp");
	*(short *)(sendbuf + 8) = bf1.pl[0];
	int ret = send(sockClient, sendbuf, 10, 0);
	if(ret <= 0) {
		showErrorMsg(text[85], 1);
		return;
	}
}

bool isMyTurn() {
	return (turn & 1) ^ 1 ^ isFirst;
}

void refreshPage();

void uptCursorState() {
	lock_guard<mutex> _lg(mtxCout);
	if(page == 0 || page == 51)
		showCursor();
	else
		hideCursor();
}

void setPage(int x) {
	prevPage = page;
	vtIn.fTextInputMode = 0;
	if(x == 0) {
		vtIn.fTextInputMode = 1;
	} else if(x == 2) {
		memset(tab, 0, sizeof tab);
		isFirst = rng() & 1;
		if(curGame.d == 2) {
			if(!curGame.n) {
				curGame.n = 3;
				bf1.resize(10, 10), bf2.resize(10, 10), bf3.resize(10, 10);
				curGame.w = curGame.h = 10;
			} else {
				bf1.clear(), bf2.clear(), bf3.clear();
			}
		} else {
			bf1.resize(curGame.w, curGame.h),
			bf2.resize(curGame.w, curGame.h),
			bf3.resize(curGame.w, curGame.h);
		}
		p2isP1Ready = p2isP2Ready = p2npl = 0;
	} else if(x == 10) {
		memset(tab, 0, sizeof tab);
		p10des1 = p10des2 = p10srd = 0;
	} else if(x == 19) {
		bf2.mk = bf3.mk;
	} else if(x == 41) {
		if(!curGame.n) {
			curGame.n = 3;
			bf1.resize(10, 10), bf2.resize(10, 10), bf3.resize(10, 10);
			curGame.w = curGame.h = 10;
		} else {
			bf1.clear(), bf2.clear(), bf3.clear();
		}
	} else if(x == 42) {
		curGame.d = -2;
		if(!pfServerInit()) return;
	} else if(x == 51) {
		vtIn.fTextInputMode = 1;
		curGame.d = -1;
		sIP = "";
		if(!pfClientInit()) return;
	}
	page = x;
	refreshPage();
}

void drawUiElem() {
	lock_guard<mutex> _lg(mtxCout);
	setDefaultColor(), clear();
	if(page == 0 || page == 1)
		bg.draw(false);
	for(int i = 1; i <= nue; i++)
		ue[i].draw();
	if(page & 0x80000000) {
		banner(errMsg, scrH / 3, white, red);
		return;
	}
	if(page == 0) {
		gotoXY(ue[4].right(), ue[4].y);
	} else if(page == 2) {
		drawBF(false);
		if(tab[0] == 0)
			drawPark(tab[1]);
	} else if(page == 4) {
		bf1.x = 4, bf1.y = 5;
		box(bf1.x, bf1.y, curGame.w * 2, curGame.h, 2);
	} else if(page == 10) {
		drawBF(false);
		gotoXY(scrW - 10, 1), cout << "#" << turn;
	} else if(page == 19) {
		drawBF(true);
		gotoXY(scrW - 10, 1), cout << "#" << turn;
	} else if(page == 51) {
		gotoXY(5, 7), cout << sIP;
	}
}

bool bfInit(pfBF &bf) {
	int i = 0, ttry = 0;
	bf.clear();
	while(i < curGame.n && ttry < 10000) {
		if(bf.placeplane(rng() % bf.w, rng() % bf.h, rng() & 3, curGame.cw))
			++i;
		++ttry;
	}
	if(i < curGame.n)
		return bf.clear(), false;
	return true;
}

void p0GenBg() {
	bg.resize(scrW / 2, scrH);
	for(int i = 0; i < (int)bg.w * bg.h / 25; i++)
		bg.placeplane(rng() % bg.w, rng() % bg.h, rng() & 3, curGame.cw);
}

void p0InputOK() {
	if(playername.s.empty()) {
		setDefaultColor();
		refreshPage();
		return;
	}
	setDefaultColor();
	setPage(1);
}

void p1Play21() {
	setPage(41);
}
void p1Play22() {
	setPage(51);
}

void p1Play2() {
	tab[0] = !tab[0];
	refreshPage();
}

void p2Start() {
	turn = 1;
	banner(text[36], scrH / 3, white, pink);
	Sleep(1000);
	setPage(10);
}
void p2Ready() {
	if(p2npl != curGame.n) return;
	if(curGame.d > 0) {
		bool tmp = bfInit(bf2);
		if(tmp == false) {
			refreshPage();
			return;
		}
		p2Start();
	} else {
		int ret;
		if(!p2isP1Ready) {
			p2isP1Ready = 1;
			strcpy(sendbuf + 6, "ready");
			ret = send(sockClient, sendbuf, 12, 0);
			if(ret <= 0) {
				showErrorMsg(text[85], 1);
				return;
			}
			if(!(curGame.d & 1) && p2isP2Ready) {
				strcpy(sendbuf + 6, "start");
				*(bool *)(sendbuf + 11) = isFirst;
				ret = send(sockClient, sendbuf, 12, 0);
				p2Start();
			}
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
	strcpy(sendbuf + 6, "giveup");
	send(sockClient, sendbuf, 12, 0);
	closesocket(sockClient);
	if(tSockClient.joinable()) tSockClient.join();
	setPage(1);
}

void p10Surrender() {
	p10srd = 1;
	if(curGame.d <= 0) {
		strcpy(sendbuf + 6, "isurrender");
		send(sockClient, sendbuf, 16, 0);
		pfExchangeMap();
	} else {
		setPage(19);
	}
}

short attackL(int x, int y, vector<short> &pl, vector<short> &mk) {
	if(mk[x + y * curGame.w] != black)
		return -1;
	if(pl[x + y * curGame.w] & 8) {
		if(curGame.cd) {
			short d = pl[x + y * curGame.w] & 3;
			for(int i = 0; i < 10; i++) {
				short tx = x + plShape[d][i].dx, ty = y + plShape[d][i].dy;
				if(curGame.cw) {
					if(tx < 0) tx += curGame.w;
					if(tx >= curGame.w) tx -= curGame.w;
					if(ty < 0) ty += curGame.h;
					if(ty >= curGame.h) ty -= curGame.h;
				}
				pl[tx + ty * curGame.w] |= 16;
			}
		}
		return mk[x + y * curGame.w] = darkRed, 2;
	}
	if(pl[x + y * curGame.w] & 4 && !(pl[x + y * curGame.w] & 16))
		return mk[x + y * curGame.w] = red, 1;
	return mk[x + y * curGame.w] = green, 0;
}

short attackR(short x, short y) {
	auto attackResFuture = p10AttackResPromise.get_future();

	strcpy(sendbuf + 6, "attack");
	*(short *)(sendbuf + 12) = x, *(short *)(sendbuf + 14) = y;
	int ret = send(sockClient, sendbuf, 16, 0);
	if(ret <= 0) {
		showErrorMsg(text[85], 1);
		return -1;
	}
	short res = attackResFuture.get();
	p10AttackResPromise = promise<short>();
	if(res == 2) {
		bf3.mk[x + y * curGame.w] = darkRed;
	} else if(res == 1) {
		bf3.mk[x + y * curGame.w] = red;
	} else if(res == 0) {
		bf3.mk[x + y * curGame.w] = green;
	}
	return res;
}

void showAttackMsg(short res) {
	if(res == 0)
		setColor(black, green);
	else if(res == 1)
		setColor(white, red);
	else if(res == 2)
		setColor(white, darkRed);
	if(res >= 0 && res <= 2)
		gotoXY((scrW - text[41 + res].len()) / 2, 7), cout << text[41 + res].s;
	Sleep(1000);
}

void p10EnemyTurn() {
	// assume curGame.d > 0
	short res, ax, ay;
	short ret = pfAIdecide(curGame, bf1.mk, ax, ay);
	if(!ret) {
		p10srd = 2;
		setPage(19);
		return;
	}
	res = attackL(ax, ay, bf1.pl, bf1.mk);
	{
		lock_guard<mutex> _lg(mtxCout);
		BlinkCoord(ax, ay, 0);
		bf1.draw(false);
		showAttackMsg(res);
	}
	refreshPage();
	if(res == 2) {
		++p10des1;
		if(p10des1 == curGame.n) {
			setPage(19);
			return;
		}
	}
	++turn;
}

void attack(short x, short y) {
	short res;
	if(curGame.d > 0)
		res = attackL(x, y, bf2.pl, bf3.mk);
	else
		res = attackR(x, y);
	if(res < 0 || res > 2) return;
	{
		lock_guard<mutex> _lg(mtxCout);
		drawBF(false);
		BlinkCoord(x, y, 1);
		showAttackMsg(res);
	}
	refreshPage();

	if(res == 2) {
		++p10des2;
		if(p10des2 == curGame.n) {
			if(curGame.d <= 0) pfExchangeMap();
			else setPage(19);
			return;
		}
	}
	if(curGame.d > 0) {
		p10EnemyTurn();
	}
	++turn;
}

void pfSockHandler() {
	while(1) {
		int ret = recv(sockClient, buf, 65536, 0);
		if(ret < 0) {
			showErrorMsg(text[86], 1);
			break;
		}
		if(!pfCheckMsg(buf, NULL)) continue;
		memcpy(tmpbuf, buf + 6, 10);
		tmpbuf[10] = 0;
		if(strcmp(tmpbuf, "isurrender") == 0) {
			p10srd = 2;
			pfExchangeMap();
			continue;
		}
		tmpbuf[6] = 0;
		if(strcmp(tmpbuf, "giveup") == 0) {
			showErrorMsg(text[80], 1);
			break;
		}
		if(strcmp(tmpbuf, "attack") == 0) {
			short ax = *(short *)(buf + 12), ay = *(short *)(buf + 14);
			short res = attackL(ax, ay, bf1.pl, bf1.mk);
			strcpy(sendbuf + 6, "result");
			*(short *)(sendbuf + 12) = res;
			int ret = send(sockClient, sendbuf, 14, 0);
			if(ret <= 0) {
				showErrorMsg(text[85], 1);
				break;
			}
			{
				lock_guard<mutex> _lg(mtxCout);
				BlinkCoord(ax, ay, 0);
				bf1.draw(false);
				showAttackMsg(res);
			}
			refreshPage();
			if(res == 2) {
				++p10des1;
				if(p10des1 == curGame.n) {
					if(curGame.d <= 0) pfExchangeMap();
				}
			}
			++turn;
			continue;
		}
		if(strcmp(tmpbuf, "result") == 0) {
			short res = *(short *)(buf + 12);
			p10AttackResPromise.set_value(res);
			continue;
		}
		tmpbuf[5] = 0;
		if(strcmp(tmpbuf, "ready") == 0) {
			p2isP2Ready = 1;
			if(!(curGame.d & 1) && p2isP1Ready) {
				strcpy(sendbuf + 6, "start");
				*(bool *)(sendbuf + 11) = isFirst;
				ret = send(sockClient, sendbuf, 12, 0);
				p2Start();
			} else
				refreshPage();
			continue;
		}
		if((curGame.d & 1) && strcmp(tmpbuf, "start") == 0) {
			isFirst = !*(bool *)(buf + 11);
			p2Start();
			continue;
		}
		tmpbuf[2] = 0;
		if(strcmp(tmpbuf, "mp") == 0) {
			if(ret > 10) {
				std::clog << "[!] in " << __PRETTY_FUNCTION__ << ", received `mp` msg of length: " << ret << " which is larget than 10.\n";
			}
			bf2.pl[p10exchgMapLn++] = *(short *)(buf + 8);
			if(p10exchgMapLn < bf2.pl.size()) {
				strcpy(sendbuf + 6, "mp");
				*(short *)(sendbuf + 8) = bf1.pl[p10exchgMapLn];
				int ret = send(sockClient, sendbuf, 10, 0);
				if(ret <= 0) {
					showErrorMsg(text[85], 1);
					break;
				}
			} else {
				for(int i = 0; i < bf2.h; i++)
					for(int j = 0; j < bf2.w; j++)
						if(bf2.pl[j + i * bf2.w] & 8) {
							bf2.basic_placeplane(j, i, bf2.pl[j + i * bf2.w] & 3, curGame.cw);
						}
				vtIn.WaitForHandlerThreads();
				setPage(19);
				break;
			}
			continue;
		}
		showErrorMsg(text[71], 1);
	}
	closesocket(sockClient);
}

void pfExit() {
	vtIn.fWork = 0;
}

void buildUiElem() {
	stringstream tmp("");
	tmp << left << setw(scrW + text[0].d) << text[0].s; // inner title
	ue[1] = pfLabel(pfTextElem(tmp.str(), text[0].d), 0, 0, white, blue, 0, 0, false);
	if(page & 0x80000000) {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] { setPage(page ^ 0x80000000); };
		nue = 2;
		return;
	}
	if(page == 0) {
		tmp.str("");
		tmp << setw((scrW - text[2].len()) / 2) << "" << left << setw(scrW + text[2].d - (scrW - text[2].len()) / 2) << text[2].s;
		ue[2] = pfLabel(pfTextElem(tmp.str(), text[2].d), 0, 5, white, pink, 0, 0, true);
		ue[3] = pfLabel(text[7], 2, 10, black, yellow, black, darkYellow, false);
		ue[3].clickFunc = p0InputOK;
		ue[4] = pfLabel(text[6] + playername, 2, 8, dfc, dbc, 0, 0, false);
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
	} else if(page == 1) {
		tmp.str("");
		tmp << setw(scrW - 1 + text[8].d) << text[8].s;
		ue[2] = pfLabel(pfTextElem(tmp.str(), text[8].d), 3, 3, black, white, black, lightGrey, true);
		ue[2].clickFunc = [] { enemyname = text[37], curGame.d = 2, setPage(2); };
		tmp.str("");
		tmp << setw(scrW - 1 + text[9].d) << text[9].s;
		ue[3] = pfLabel(pfTextElem(tmp.str(), text[9].d), 6, 7, black, white, black, lightGrey, true);
		ue[3].clickFunc = p1Play2;
		tmp.str("");
		tmp << setw(scrW - 1 + text[10].d) << text[10].s;
		ue[4] = pfLabel(pfTextElem(tmp.str(), text[10].d), 9, 11, black, white, black, lightGrey, true);
		ue[4].clickFunc = [] { setPage(5); };
		nue = 4;

		ue[5] = pfLabel(text[16], 12, 15, white, red, white, darkRed, true);
		ue[5].clickFunc = [] { pfExit(); };
		if(ue[5].right() + 2 + text[17].len() < scrW)
			ue[6] = pfLabel(text[17], ue[5].right() + 2, 15, black, white, black, lightGrey, true);
		else
			ue[6] = pfLabel(text[17], ue[5].x, 19, black, white, black, lightGrey, true);
		ue[6].clickFunc = [] { setPage(0); };
		nue = 6;
		if(tab[0]) {
			ue[7] = pfLabel(text[20], 8, 10, dfc, dbc, grey, dbc, false);
			ue[7].clickFunc = p1Play21;
			ue[8] = pfLabel(text[21], 8, 11, dfc, dbc, grey, dbc, false);
			ue[8].clickFunc = p1Play22;
			nue = 8;
		}
	} else if(page == 2) {
		if(curGame.d > 0) {
			ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
			ue[2].clickFunc = [] { setPage(1); };
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
			ue[6] = pfLabel(text[curGame.d > 0 ? 25 : 77], (scrW - text[curGame.d > 0 ? 25 : 77].len()) / 2, 10, black, yellow, black, darkYellow, true);
		else
			ue[6] = pfLabel();
		ue[6].clickFunc = p2Ready;
		if(tab[0] == 0) {
			if(p2isP1Ready) {
				nue = 6;
			} else {
				ue[7] = pfLabel(text[26], 12, 18 + curGame.h, black, yellow, black, darkYellow, false);
				ue[7].clickFunc = [] { if(!p2isP1Ready) bf1.clear(), p2npl=0, refreshPage(); };
				nue = 7;
			}
		} else if(tab[0] == 1) {
			if(curGame.d > 0) {
				ue[7] = pfLabel((curGame.cw ? text[30] : text[29]) + text[31], 4, 10 + curGame.h, dfc, dbc, grey, black, false);
				ue[7].clickFunc = p2SwitchCw;
			} else if(curGame.cw) {
				ue[7] = pfLabel(text[74], 4, 10 + curGame.h, dfc, dbc, grey, black, false);
			} else {
				ue[7] = pfLabel();
			}
			if(curGame.d > 0) {
				ue[8] = pfLabel((curGame.cd ? text[30] : text[29]) + text[83], 4, 12 + curGame.h, dfc, dbc, grey, black, false);
				ue[8].clickFunc = [] { curGame.cd=!curGame.cd; refreshPage(); };
			} else if(curGame.cd) {
				ue[8] = pfLabel(text[84], 4, 12 + curGame.h, dfc, dbc, grey, black, false);
			} else
				ue[8] = pfLabel();
			if(curGame.d > 0) {
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
				ue[10] = pfLabel(text[curGame.d == -1 ? 78 : 79], 4, 13 + curGame.h, dfc, dbc, 0, 0, false);
				ue[11] = pfLabel();
				ue[12] = pfLabel(text[89] + (isFirst ? text[87] : text[88]), 4, 14 + curGame.h, dfc, dbc, grey, black, false);
			}
			tmp.str("");
			tmp << curGame.h << "*" << curGame.w;
			if(curGame.d > 0)
				ue[13] = pfLabel(text[90] + tmp.str(), 4, 16 + curGame.h, black, yellow, black, darkYellow, false),
				ue[13].clickFunc = [] { setPage(4); };
			else
				ue[13] = pfLabel(text[90] + tmp.str(), 4, 16 + curGame.h, dfc, dbc, grey, black, false);
			nue = 13;
		}
		if(p2isP2Ready)
			ue[++nue] = pfLabel(text[91].s, (scrW - text[91].len()) / 2, scrH * 2 / 3, yellow, dbc, darkYellow, dbc, false);
		else if(p2isP1Ready)
			ue[++nue] = pfLabel(text[82].s, (scrW - text[82].len()) / 2, scrH * 2 / 3, yellow, dbc, darkYellow, dbc, false);
	} else if(page == 4) {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] {
			bf1.resize(curGame.w, curGame.h), bf2.resize(curGame.w, curGame.h), bf3.resize(curGame.w, curGame.h);
			setPage(prevPage);
		};
		tmp.str("");
		tmp << curGame.h << "*" << curGame.w;
		ue[3] = pfLabel(text[90] + tmp.str(), 4, 2, dfc, dbc, grey, black, false);
		nue = 3;
	} else if(page == 5) {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] { setPage(1); };
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
	} else if(page == 10) {
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
	} else if(page == 19) {
		if(p10srd == 2)
			ue[2] = pfLabel(text[47], (scrW - text[47].len()) / 2, 4, white, darkGreen, grey, darkGreen, true);
		else if(p10srd == 1)
			ue[2] = pfLabel(text[46], (scrW - text[46].len()) / 2, 4, white, darkRed, grey, darkRed, true);
		else if(p10des1 == curGame.n)
			ue[2] = pfLabel(text[45], (scrW - text[45].len()) / 2, 4, white, red, grey, red, true);
		else if(p10des2 == curGame.n)
			ue[2] = pfLabel(text[44], (scrW - text[44].len()) / 2, 4, black, green, black, darkGreen, true);
		else
			ue[2] = pfLabel();
		ue[3] = pfLabel(text[48], (scrW - text[48].len()) / 2, scrH * 2 / 3, black, yellow, black, darkYellow, true);
		ue[3].clickFunc = [] { setPage(1); };
		nue = 3;
	} else if(page == 41) {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] { setPage(1); };
		ue[3] = pfLabel(text[23], 4, 5, dfc, dbc, 0, 0, false);
		ue[4] = pfLabel(text[25], (scrW - text[25].len()) / 2, scrH * 2 / 3, black, yellow, black, darkYellow, true);
		ue[4].clickFunc = [] { setPage(42); };
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
		ue[11].clickFunc = [] { setPage(4); };
		nue = 11;
	} else if(page == 42) {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] {
			closesocket(sockServer);
			if(tSockServer.joinable()) tSockServer.join();
			setPage(1);
		};
		if(sIP.empty())
			ue[3] = pfLabel(text[55], 0, 4, dfc, dbc, dfc, dbc, false);
		else
			ue[3] = pfLabel(text[54] + sIP, 0, 4, dfc, dbc, dfc, dbc, false);
		ue[4] = pfLabel(text[61], 0, 6, dfc, dbc, dfc, dbc, false);
		nue = 4;
	} else if(page == 51) {
		ue[2] = pfLabel(text[11], 0, 1, black, yellow, black, darkYellow, false); // back
		ue[2].clickFunc = [] { setPage(1); };
		ue[3] = pfLabel(text[67], 4, 5, dfc, dbc, 0, 0, false);
		nue = 3;
	} else
		nue = 1;
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
	if(page == 2 && tab[0] == 0) {
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
			drawPark(tab[1]);
		}
		if(mx >= bf1.x && mx < bf1.x + bf1.w * 2 && my >= bf1.y && my < bf1.y + bf1.h && p2npl < curGame.n && !p2isP1Ready) {
			if(bf1.placeplane((mx - bf1.x) >> 1, my - bf1.y, tab[1], curGame.cw)) {
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
			drawBF(false);
		}
	} else if(page == 4) {
		int nx = (mx - bf1.x) / 2, ny = my - bf1.y;
		if(nx >= 5 && nx <= scrW && ny >= 5)
			if(nx != curGame.w || ny != curGame.h) {
				curGame.w = nx;
				curGame.h = ny;
				refreshPage();
			}
	} else if(page == 10) {
		if(mx >= bf2.x && mx < bf2.x + bf2.w * 2 && my >= bf2.y && my < bf2.y + bf2.h) {
			if(tab[0] == 0 && isMyTurn() && bf3.mk[(mx - bf2.x) / 2 + (my - bf2.y) * bf3.w] == black) {
				attack((mx - bf2.x) / 2, my - bf2.y);
			} else if(tab[0] == 1) {
				bf3.ch[(mx - bf2.x) / 2 + (my - bf2.y) * bf3.w] = marker[tab[1]];
				lock_guard<mutex> _lg(mtxCout);
				drawBF(false);
			} else if(tab[0] == 2) {
				bf3.ch[(mx - bf2.x) / 2 + (my - bf2.y) * bf3.w] = "";
				lock_guard<mutex> _lg(mtxCout);
				drawBF(false);
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
		if(page == 2) {
			if(mx >= bf1.x && mx < bf1.x + bf1.w * 2 && my >= bf1.y && my < bf1.y + bf1.h && tab[0] == 0 && p2npl < curGame.n && bf1.ch[(mx - bf1.x) / 2 + (my - bf1.y) * bf1.w].empty() && !p2isP1Ready) {
				if(mtxCout.try_lock()) {
					drawBF(false);
					setColor(grey, dbc);
					drawPlane(bf1.x + ((mx - bf1.x) & (short)-2), my, tab[1], true);
					mtxCout.unlock();
				}
			}
		} else if(page == 10) {
			if(mx >= bf2.x && mx < bf2.x + bf2.w * 2 && my >= bf2.y && my < bf2.y + bf2.h && bf3.mk[(mx - bf2.x) / 2 + (my - bf2.y) * bf2.w] == 0) {
				if(mtxCout.try_lock()) {
					bf3.draw(true);
					setColor(grey, dbc);
					gotoXY(bf2.x + ((mx - bf2.x) | 1) - 1, my), cout << text[40].s;
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
		if(page == 0) {
			if(s == "\n") {
				playername.s = vtIn.ReadLine();
				gotoXY(2, 8);
				cout << text[6].s << playername.s;
				auto pos = getXY();
				playername.d = playername.s.length() - (pos.first - (2 + text[6].len()) + (pos.second - 8) * scrW);

				ue[3]._click();
			} else if(s == "\x7f") {
				gotoXY(2, 8);
				ClearLineRight();
				cout << text[6].s << vtIn.PeekLine();
			}
		} else if(page == 51) {
			if(s == "\n") {
				sIP = vtIn.ReadLine();
				if(pfClientConnect()) {
					setPage(2);
					if(tSockClient.joinable()) tSockClient.join();
					tSockClient = thread(pfSockHandler);
				}
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
	setPage(0);

	VtEnableMouseTrackingAny();
	vtIn.mouseHandler = MouseHandler;
	vtIn.keyHandler = KeyHandler;
	vtIn.Work();
	// TODO:
	// window size event;
	return 0;
}