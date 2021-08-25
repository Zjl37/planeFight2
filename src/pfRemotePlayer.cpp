#include "pfRemotePlayer.hpp"

extern std::mt19937 rng;

WSADATA wsaData;
SOCKET sockServer;
SOCKADDR_IN addrClient;
std::thread tSockServer;

// TODO: reduce abuse of C string
char buf[65536], sendbuf[65536] = "pf", tmpbuf[65536];

// temporary implementation
bool pfCheckMsg(const char *msg, const char *i) {
	if(!i) return true;
	return std::string(msg+6, strlen(i)) == i;
}

#if 0
// Todo: examine this.
bool pfCheckMsg(const char *msg, const char *i) {
	if(msg[0] != 'p' || msg[1] != 'f') {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71], PfPage::main);
		return false;
	}
	// if(*(int *)(msg + 2) != hg) {
	// 	closesocket(sockClient);
	// 	WSACleanup();
	// 	showErrorMsg(text[71], PfPage::main);
	// 	return false;
	// }
	if(!i) return true;
	std::string msgName(msg + 6, strlen(i));
	if(msgName != i) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71], PfPage::main);
		return false;
	}
	return true;
}
#endif

int pfCheckVer(const std::string &s) {
	std::stringstream curVer(pfVerStr), iVer(s);
	char ch;
	int curVerSec1 = 0, iVerSec1 = 0, curVerSec2 = 0, iVerSec2 = 0;
	std::string curGName, iGName;
	curVer >> curGName >> curVerSec1 >> ch >> curVerSec2;
	iVer >> iGName >> iVerSec1 >> ch >> iVerSec2;
	if(curGName != iGName || curVerSec1 != iVerSec1)
		return -1;
	if(curVerSec2 != iVerSec2) return 1;
	return 0;
}

std::shared_ptr<PfRemotePlayer> PfCreateRemoteClient(SOCKET sockClient, const PfPlayer &o) {
	auto p = std::make_shared<PfRemotePlayer>();
	p->as = p->pos_client;
	p->sock = sockClient;

	gotoXY(0, getY() + 1), std::cout << text[63].s;
	int ret = recv(p->sock, buf, 65536, 0);
	if(ret < 0) {
		throw text[86];
	}
	ret = pfCheckVer(buf);
	if(ret == -1) {
		strcpy(sendbuf + 6, "!ver");
		send(p->sock, sendbuf, 11, 0);
		closesocket(sockServer);
		WSACleanup();
		throw(text[64] + (std::string)buf);
	} else if(ret == 1) {
		gotoXY(0, getY() + 1), std::cout << text[65].s << buf;
	}
	p->game.id = *(unsigned *)(sendbuf + 2) = rng();
	strcpy(sendbuf + 6, "hello");
	ret = send(p->sock, sendbuf, 11, 0);
	// check ret
	strcpy(sendbuf + 6, "gameinfo");
	memcpy(sendbuf + 14, &curGame, sizeof curGame);
	curGame.d = -2;
	ret = send(p->sock, sendbuf, 14 + sizeof curGame, 0);
	// check ret
	strcpy(sendbuf + 6, "name");
	// TODO: d value should not be transmitted over the net
	*(int *)(sendbuf + 10) = o.GetName().d;
	strcpy(sendbuf + 14, o.GetName().s.c_str());
	ret = send(p->sock, sendbuf, 15 + o.GetName().s.length(), 0);
	// check ret
	ret = recv(p->sock, buf, 65535, 0);
	// check ret
	if(!pfCheckMsg(buf, "name"))
		throw text[71];
	p->name = pfTextElem(std::string(buf + 14), *(int *)(buf + 10));
	extern bool isFirst; // why this can't be a part of GameInfo !!??
	p->NewGame(curGame, *(unsigned *)(buf + 2), !isFirst);
	p->tSock = std::thread(&PfRemotePlayer::SockHandler, &*p);
	return p;
}

bool PfServerAccept() {
	int len = sizeof(SOCKADDR);
	SOCKET sockClient = accept(sockServer, (SOCKADDR *)&addrClient, &len);
	closesocket(sockServer);
	if(sockClient == INVALID_SOCKET)
		return false;
	try {
		player[1] = PfCreateRemoteClient(sockClient, *player[0]);
		extern bool isFirst; // why !!??
		player[0]->NewGame(curGame, player[1]->GetGame().id, isFirst);
		player[0]->other = player[1];
		player[1]->other = player[0];
		bf1.resize(curGame.w, curGame.h);
		SetPage(PfPage::prepare);
	} catch(const pfTextElem &t) {
		showErrorMsg(t);
		return false;
	}
	return true;
}

void PfServerInit() {
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		throw text[52];
	}
	sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockServer == INVALID_SOCKET) {
		WSACleanup();
		throw text[56];
	}
	SOCKADDR_IN addrServer;
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(51937);
	int ret = bind(sockServer, (SOCKADDR *)&addrServer, sizeof(SOCKADDR));
	if(ret == SOCKET_ERROR) {
		closesocket(sockServer);
		sockServer = INVALID_SOCKET;
		WSACleanup();
		throw text[58];
	}
	ret = listen(sockServer, 1);
	if(ret == SOCKET_ERROR) {
		closesocket(sockServer);
		sockServer = INVALID_SOCKET;
		WSACleanup();
		throw text[60];
	}
	if(tSockServer.joinable()) tSockServer.join();
	tSockServer = std::thread(PfServerAccept);
}

void PfServerStop() {
	if(sockServer != INVALID_SOCKET) {
		closesocket(sockServer);
	}
	if(tSockServer.joinable()) {
		tSockServer.join();
	}
}

std::shared_ptr<PfRemotePlayer> PfCreateRemoteServer(std::string sIP, const PfPlayer &o) {
	auto p = std::make_shared<PfRemotePlayer>();
	p->as = p->pos_server;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		throw text[52];
	}
	p->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(p->sock == INVALID_SOCKET) {
		WSACleanup();
		throw text[66];
	}

	SOCKADDR_IN addrServer;
	addrServer.sin_addr.S_un.S_addr = inet_addr(sIP.c_str());
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(51937);
	int ret = connect(p->sock, (SOCKADDR *)&addrServer, sizeof(SOCKADDR));
	if(ret == SOCKET_ERROR) {
		closesocket(p->sock);
		WSACleanup();
		throw text[68];
	}
	// TODO: move these send else where
	ret = send(p->sock, pfVerStr, strlen(pfVerStr) + 1, 0);
	ret = recv(p->sock, buf, 11, 0);
	buf[11] = 0;
	if(strcmp(buf + 6, "!ver") == 0) {
		closesocket(p->sock);
		WSACleanup();
		throw text[70];
	} else if(buf[0] != 'p' || buf[1] != 'f') {
		closesocket(p->sock);
		WSACleanup();
		throw text[71];
	} else if(strcmp(buf + 6, "hello")) {
		closesocket(p->sock);
		WSACleanup();
		throw text[71];
	}
	*(unsigned *)(sendbuf + 2) = *(unsigned *)(buf + 2);
	ret = recv(p->sock, buf, 14 + sizeof curGame, 0);
	if(!pfCheckMsg(buf, "gameinfo"))
		throw text[71];
	// TODO: don't memcpy a struct.
	memcpy(&curGame, buf + 14, sizeof curGame);
	curGame.d = -1;
	// TODO: d value should not be transmitted over the net
	strcpy(sendbuf + 6, "name");
	*(int *)(sendbuf + 10) = o.GetName().d;
	strcpy(sendbuf + 14, o.GetName().s.c_str());
	ret = send(p->sock, sendbuf, 15 + o.GetName().s.length(), 0);
	// check ret
	ret = recv(p->sock, buf, 65535, 0);
	// check ret
	if(!pfCheckMsg(buf, "name"))
		throw text[71];
	p->name = pfTextElem(std::string(buf + 14), *(int *)(buf + 10));
	p->NewGame(curGame, *(unsigned int *)(buf + 2));
	p->tSock = std::thread(&PfRemotePlayer::SockHandler, &*p);
	return p;
}

PfRemotePlayer::PfRemotePlayer(): PfPlayer(), as(pos_null), exchgMapLn(0) {}

void PfRemotePlayer::ArrangeReady() {
	PfPlayer::ArrangeReady();
}

void PfRemotePlayer::OnGameStart() {
	if(as == pos_client) {
		strcpy(sendbuf + 6, "start");
		*(bool *)(sendbuf + 11) = !game.isFirst;
		send(sock, sendbuf, 12, 0);
	}
}

void PfRemotePlayer::SockHandler() {
	// TODO: deal with sticking TCP message
	try {
		while(1) {
			int ret = recv(sock, buf, 65536, 0);
			if(ret < 0) {
				throw text[86];
			}
			if(!pfCheckMsg(buf, NULL)) continue;
			memcpy(tmpbuf, buf + 6, 10);
			tmpbuf[10] = 0;
			if(strcmp(tmpbuf, "isurrender") == 0) {
				Surrender();
				continue;
			}
			tmpbuf[6] = 0;
			if(strcmp(tmpbuf, "giveup") == 0) {
				Giveup();
				break;
			}
			if(strcmp(tmpbuf, "attack") == 0) {
				short ax = *(short *)(buf + 12), ay = *(short *)(buf + 14);
				Attack(ax, ay);
				continue;
			}
			if(strcmp(tmpbuf, "result") == 0) {
				short res = *(short *)(buf + 12);
				if(auto o = other.lock()) {
					o->AttackResulted(PfAtkRes(res));
				}
				continue;
			}
			tmpbuf[5] = 0;
			if(strcmp(tmpbuf, "ready") == 0) {
				ArrangeReady();
				continue;
			}
			if(as == pos_server && strcmp(tmpbuf, "start") == 0) {
				game.isFirst = *(bool *)(buf + 11);
				if(auto o = other.lock()) {
					o->SetFirst(!game.isFirst);
					// 'isFirst' logic is like nightmare!!!
				}
				continue;
			}
			tmpbuf[2] = 0;
			if(strcmp(tmpbuf, "mp") == 0) {
				// NOTE: a temporary solution of sticking message.
				auto procSingleMpMsg = [&](short pli) {
					myBf.pl[exchgMapLn++] = *(short *)(buf + 8);
					if(exchgMapLn < myBf.pl.size()) {
						strcpy(sendbuf + 6, "mp");
						*(short *)(sendbuf + 8) = bf1.pl[exchgMapLn];
						int ret = send(sock, sendbuf, 10, 0);
						if(ret < 0) {
							throw text[85];
						}
					}
				};
				for(int j = 0; j + 10 <= ret; j += 10) {
					procSingleMpMsg(*(short *)(buf + j + 8));
				}
				if(exchgMapLn >= myBf.pl.size()) {
					if(auto o = other.lock()) {
						o->SetOthersBF(myBf.pl);
					}
				}
				if(ret % 10) {
					std::clog << "[!] in " << __PRETTY_FUNCTION__ << ", received `mp` msg of length: " << ret << " which is not multiple of 10.\n";
				}
				continue;
			}
			throw text[71];
		}
	} catch(const pfTextElem &t) {
		showErrorMsg(t);
		return;
	}
	shutdown(sock, SD_BOTH);
}

void PfRemotePlayer::OnOtherReady() {
	strcpy(sendbuf + 6, "ready");
	int ret = send(sock, sendbuf, 12, 0);
	if(ret < 0) {
		showErrorMsg(text[85], PfPage::main);
		return;
	}
	PfPlayer::OnOtherReady();
}

void PfRemotePlayer::OnOtherGiveup() {
	strcpy(sendbuf + 6, "giveup");
	send(sock, sendbuf, 12, 0);
	closesocket(sock);
	sock = -1;
}

void PfRemotePlayer::OnOtherSurrender() {
	PfPlayer::OnOtherSurrender();
	strcpy(sendbuf + 6, "isurrender");
	send(sock, sendbuf, 16, 0);
}

void PfRemotePlayer::BeingAttacked(short x, short y) {
	strcpy(sendbuf + 6, "attack");
	*(short *)(sendbuf + 12) = x, *(short *)(sendbuf + 14) = y;
	int ret = send(sock, sendbuf, 16, 0);
	if(ret < 0) {
		showErrorMsg(text[85], PfPage::main);
		return;
	}
}

void PfRemotePlayer::AttackResulted(PfAtkRes res) {
	strcpy(sendbuf + 6, "result");
	*(short *)(sendbuf + 12) = (short)res;
	int ret = send(sock, sendbuf, 14, 0);
	if(ret < 0) {
		showErrorMsg(text[85], PfPage::main);
		return;
	}
}

void PfRemotePlayer::MapRequested() {
	if(auto o = other.lock()) {
		o->MapRequested();
	}
}

void PfRemotePlayer::SetOthersBF(const std::vector<short> &pl) {
	othersBf.pl = pl;
	strcpy(sendbuf + 6, "mp");
	*(short *)(sendbuf + 8) = pl[0];
	int ret = send(sock, sendbuf, 10, 0);
	if(ret < 0) {
		throw text[85];
	}
}

PfRemotePlayer::~PfRemotePlayer() {
	if(sock != INVALID_SOCKET) {
		closesocket(sock);
	}
	if(tSock.joinable()) tSock.join();
}
