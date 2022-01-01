/**
 * Copyright © 2021 Zjl37 <2693911885@qq.com>
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

#pragma once
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/streambuf.hpp"
#include "game.hpp"
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