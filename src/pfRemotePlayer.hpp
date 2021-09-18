#pragma once
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/streambuf.hpp"
#include "pfGame.hpp"
#include <thread>

namespace asio = boost::asio;

void PfServerInit();
void PfServerStop();
void PfStopConnect();

class PfRemotePlayer: public PfPlayer {
	enum {
		pos_null = 0,
		pos_client = -1,
		pos_server = -2,
	} as;

	std::thread tSock;
	asio::ip::tcp::socket sock;
	asio::streambuf buf, sendbuf;
	bool expectDisconnect = 0;
	bool othersMapReceived = 0;

	void OnGameStart();
	void ArrangeReady();
	void OnOtherReady();
	void OnOtherGiveup();
	void OnOtherSurrender();
	void OnGameover();
	void BeingAttacked(short, short);
	void AttackResulted(PfAtkRes);
	void MapRequested();
	void SetOthersBF(const std::vector<short> &pl);

	void SockHandler();

	public:
	PfRemotePlayer();
	~PfRemotePlayer();
	friend std::shared_ptr<PfRemotePlayer> PfCreateRemoteServer(std::string sIP, const PfPlayer &opponent);
	friend std::shared_ptr<PfRemotePlayer> PfCreateRemoteClient(asio::ip::tcp::socket &&, const PfPlayer &opponent);
};

std::shared_ptr<PfRemotePlayer> PfCreateRemoteServer(std::string sIP, const PfPlayer &opponent);