#pragma once
#include "pfGame.hpp"
#include <thread>

// bool pfCheckMsg(const char *msg, const char *i);
// void pfSockHandler(); // to remove
bool pfServerAccept();
bool pfServerInit();
// bool pfClientInit();
bool pfClientConnect();
// void pfExchangeMap();

class PfRemotePlayer: public PfPlayer {
	enum {
		pos_null = 0,
		pos_client = -1,
		pos_server = -2,
	} as;

	SOCKET sock;
	std::thread tSock;
	int exchgMapLn;

	void OnGameStart();
	void ArrangeReady();
	void OnOtherReady();
	void OnOtherGiveup();
	void OnOtherSurrender();
	void BeingAttacked(short, short);
	void AttackResulted(PfAtkRes);
	void MapRequested();
	void SetOthersBF(const std::vector<short> &pl);

	void SockHandler();

	public:
	PfRemotePlayer(): as(pos_null) {}
	~PfRemotePlayer();
	friend std::shared_ptr<PfRemotePlayer> PfCreateRemoteServer(std::string sIP, const PfPlayer &opponent);
	friend std::shared_ptr<PfRemotePlayer> PfCreateRemoteClient();
};

std::shared_ptr<PfRemotePlayer> PfCreateRemoteServer(std::string sIP, const PfPlayer &opponent);
std::shared_ptr<PfRemotePlayer> PfCreateRemoteClient();