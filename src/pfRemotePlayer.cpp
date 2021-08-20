#include "pfRemotePlayer.hpp"

extern std::mt19937 rng;

WSADATA wsaData;
SOCKET sockServer;
SOCKADDR_IN addrClient;

// TODO: reduce abuse of C string
char buf[65536], sendbuf[65536] = "pf", tmpbuf[65536];

// temporary implementation
bool pfCheckMsg(const char *msg, const char *i) {
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

bool pfServerAccept() {
	int len = sizeof(SOCKADDR), ret = 0;
	sockClient = accept(sockServer, (SOCKADDR *)&addrClient, &len);
	if(sockClient == INVALID_SOCKET)
		return false;
	closesocket(sockServer);
	gotoXY(0, getY() + 1), std::cout << text[63].s;
	ret = recv(sockClient, buf, 65536, 0);
	// check return value
	ret = pfCheckVer(buf);
	if(ret == -1) {
		strcpy(sendbuf + 6, "!ver");
		send(sockClient, sendbuf, 11, 0);
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[64] + (std::string)buf, PfPage::main);
		return false;
	} else if(ret == 1) {
		gotoXY(0, getY() + 1), std::cout << text[65].s << buf;
	}
	// hg = *(int *)(sendbuf + 2) = rng();
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
	NextPage(PfPage::prepare);
	closesocket(sockServer);
	if(tSockClient.joinable()) tSockClient.join();
	tSockClient = thread(pfSockHandler);
	return true;
}
bool pfServerInit() {
	setDefaultColor(), clear();
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		showErrorMsg(text[52], PfPage::main);
		return false;
	}
	sIP = "";
	getIP();
	sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockServer == INVALID_SOCKET) {
		WSACleanup();
		showErrorMsg(text[56], PfPage::main);
		return false;
	}
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(51937);
	int ret = bind(sockServer, (SOCKADDR *)&addrServer, sizeof(SOCKADDR));
	if(ret == SOCKET_ERROR) {
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[58], PfPage::main);
		return false;
	}
	ret = listen(sockServer, 1);
	if(ret == SOCKET_ERROR) {
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[60], PfPage::main);
		return false;
	}
	if(tSockServer.joinable()) tSockServer.join();
	tSockServer = thread(pfServerAccept);
	return true;
}
#endif

std::shared_ptr<PfRemotePlayer> PfCreateRemoteServer(std::string sIP, const PfPlayer &o) {
	auto p = std::make_shared<PfRemotePlayer>();
	p->as = p->pos_client;
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
	p->game.id = *(int *)(sendbuf + 2) = *(int *)(buf + 2);
	ret = recv(p->sock, buf, 14 + sizeof curGame, 0);
	if(!pfCheckMsg(buf, "gameinfo"))
		throw text[71];
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
	p->tSock = std::thread([&]() {
		p->SockHandler();
	});
	return p;
}

void PfRemotePlayer::ArrangeReady() {
	PfPlayer::ArrangeReady();
}

void PfRemotePlayer::OnGameStart() {
	if(as == pos_client) {
		strcpy(sendbuf + 6, "start");
		*(bool *)(sendbuf + 11) = game.isFirst;
		send(sock, sendbuf, 12, 0);
	}
}

void PfRemotePlayer::SockHandler() {
	while(1) {
		int ret = recv(sock, buf, 65536, 0);
		if(ret < 0) {
			showErrorMsg(text[86], PfPage::main);
			break;
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
			game.isFirst = !*(bool *)(buf + 11);
			// p2Start();
			// legacy. No need to do anything here.
			continue;
		}
		tmpbuf[2] = 0;
		if(strcmp(tmpbuf, "mp") == 0) {
			if(ret > 10) {
				std::clog << "[!] in " << __PRETTY_FUNCTION__ << ", received `mp` msg of length: " << ret << " which is larget than 10.\n";
			}
			myBf.pl[exchgMapLn++] = *(short *)(buf + 8);
			if(exchgMapLn < myBf.pl.size()) {
				strcpy(sendbuf + 6, "mp");
				*(short *)(sendbuf + 8) = bf1.pl[exchgMapLn];
				int ret = send(sock, sendbuf, 10, 0);
				if(ret < 0) {
					showErrorMsg(text[85], PfPage::main);
					break;
				}
			} else {
				if(auto o = other.lock()) {
					o->SetOthersBF(myBf.pl);
				}
				break;
			}
			continue;
		}
		showErrorMsg(text[71]);
	}
	closesocket(sock);
	sock = -1;
}

void PfRemotePlayer::OnOtherReady() {
	int ret = send(sock, sendbuf, 12, 0);
	strcpy(sendbuf + 6, "ready");
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
	exchgMapLn = 0;
	othersBf.pl = pl;
	strcpy(sendbuf + 6, "mp");
	*(short *)(sendbuf + 8) = pl[0];
	int ret = send(sock, sendbuf, 10, 0);
	if(ret < 0) {
		throw text[85];
	}
}

PfRemotePlayer::~PfRemotePlayer() {
	if(sock != -1) {
		closesocket(sock);
	}
	if(tSock.joinable()) tSock.join();
	// closesocket(sockServer);
	// if(tSockServer.joinable()) tSockServer.join();
}
