/**
 * Copyright © 2021-2022 Zjl37 <2693911885@qq.com>
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

#include "remotePlayer.hpp"
#include "boost/asio.hpp"
#include "pfLocale.hpp"
#include <string>

using namespace std::string_literals;
extern std::mt19937 rng;

using asio::ip::tcp;

asio::io_context ioctxt;
tcp::acceptor acceptor(ioctxt);
tcp::socket sockClient(ioctxt);
std::thread tServer;

const std::string pfProtoNameStr = "Zjl37's planeFight protocol";
const std::string pfProtoVerStr = "0.3.0";
const int pfProtoVerMajor = 0, pfProtoVerMinor = 3;
[[maybe_unused]] const int pfProtoVerPatch = 0;

void PfCheckReqLn(std::istream &is) {
	std::string reqMethod, protoStr;
	is >> reqMethod;
	if(is.fail() || reqMethod != "PLAY") {
		// TODO: more detailed error message.
		throw TT("Error: Bad game message sent from the other player.");
	};
	is.ignore();
	std::getline(is, protoStr);
	if(is.fail() || protoStr.length() < pfProtoNameStr.length() || std::string_view(protoStr.data(), pfProtoNameStr.length()) != pfProtoNameStr) {
		throw TT("Error: Bad game message sent from the other player.");
	}
	{
		std::stringstream ss{protoStr.substr(pfProtoNameStr.length())};
		char ch;
		int vMaj, vMin, vPat;
		ss >> vMaj >> ch >> vMin >> ch >> vPat;
		if(vMaj != pfProtoVerMajor || vMin != pfProtoVerMinor) {
			throw TT("Error: Client version incompatible, which is ").str() + protoStr.substr(pfProtoNameStr.length());
		}
	}
}

std::shared_ptr<PfRemotePlayer> PfCreateRemoteClient(tcp::socket &&sockClient, const PfPlayer &o) {
	auto p = std::make_shared<PfRemotePlayer>();
	p->as = p->pos_client;
	auto &sock = p->sock = std::move(sockClient);

	std::ostream os(&p->sendbuf);
	os << "PLAY " << pfProtoNameStr << " " << pfProtoVerStr << "\r\n"
	   << "User-Agent: "s << pfUA << "\r\n"
	   << "\r\n";
	asio::write(p->sock, p->sendbuf);

	asio::streambuf buf;
	asio::read_until(sock, buf, "\r\n");

	std::istream is(&buf);
	PfCheckReqLn(is);

	// read and check header line.
	std::string hdLn;
	do {
		std::getline(is, hdLn);
		if(!hdLn.empty() && hdLn.back() == '\r') hdLn.pop_back();
		if(!hdLn.empty()) {
			auto p = hdLn.find(": ");
			if(p == std::string::npos) {
				std::clog << "[!] in " << __PRETTY_FUNCTION__ << ", read invalid header:\n";
				std::clog << hdLn << "\n";
			} else {
				/* TODO */
			}
		}
	} while(!hdLn.empty());

	p->NewGame(InvertIsFirst(curGame), rng());
	p->tSock = std::thread(&PfRemotePlayer::SockHandler, &*p);
	return p;
}

void PfServerAccept(const boost::system::error_code &ec) {
	if(ec) {
		std::clog << "in " << __PRETTY_FUNCTION__ << " network error (error code): " << ec.message() << std::endl;
		// Do not show error message when user stops server
		if(stPage.top() == PfPage::server_init) {
			showErrorMsg(TT("Error: Cannot accept client."), PfPage::gamerule_setting_server);
		}
		return;
	}
	try {
		acceptor.close();
		player[1] = PfCreateRemoteClient(std::move(sockClient), *player[0]);
		player[0]->other = player[1];
		player[1]->other = player[0];
	} catch(const boost::system::system_error &e) {
		std::clog << "in " << __PRETTY_FUNCTION__ << " network error (system error): " << e.what() << std::endl;
		showErrorMsg(TT("Error: Cannot accept client."), PfPage::gamerule_setting_server);
	} catch(const std::exception &e) {
		std::clog << "in " << __PRETTY_FUNCTION__ << " error: " << e.what() << std::endl;
		showErrorMsg(TT("Error: Cannot accept client."), PfPage::gamerule_setting_server);
	}
	return;
}

void PfServerInit() {
	try {
		acceptor = tcp::acceptor(ioctxt, tcp::endpoint(tcp::v4(), 51937));
		sockClient.close();
		if(tServer.joinable()) tServer.join();
		acceptor.async_accept(sockClient, PfServerAccept);
		if(ioctxt.stopped()) {
			ioctxt.restart();
		}
		tServer = std::thread([]() {
			ioctxt.run();
		});
	} catch(const boost::system::system_error &e) {
		showErrorMsg(TT("Error: Cannot create a server."), PfPage::gamerule_setting_server);
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " network error: " << e.what() << std::endl;
		PfServerStop();
	} catch(const std::exception &e) {
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " error: " << e.what() << std::endl;
	}
}

void PfServerStop() {
	acceptor.close();
	if(tServer.joinable()) tServer.join();
}

void PfStopConnect() {
	ioctxt.stop();
}

void ToBytes(std::ostream &os, const uint32_t &x) {
	os.put(x >> 24);
	os.put(x >> 16 & 0x7f);
	os.put(x >> 8 & 0x7f);
	os.put(x & 0x7f);
}

void ToBytes(std::ostream &os, const uint16_t &x) {
	os.put(x >> 8);
	os.put(x & 0x7f);
}

void ToBytes(std::ostream &os, const uint8_t &x) {
	os.put(x);
}

template<class T, class... Args>
void ToBytes(std::ostream &os, const T &x, const Args &...args) {
	ToBytes(os, x);
	ToBytes(os, args...);
}

void FromBytes(std::istream &is, uint32_t &x) {
	x = is.get() << 24;
	x |= is.get() << 16;
	x |= is.get() << 8;
	x |= is.get();
}

void FromBytes(std::istream &is, uint16_t &x) {
	x = is.get() << 8;
	x |= is.get();
}

void FromBytes(std::istream &is, uint8_t &x) {
	x = is.get();
}

template<class T, class... Args>
void FromBytes(std::istream &is, T &x, Args &...args) {
	FromBytes(is, x);
	FromBytes(is, args...);
}

enum class PfNwPacket : uint32_t {
	join = 0x6a6f696e,
	game_info = 0x676d6966,
	name = 0x6e616d65,
	give_up = 0x67767570,
	ready = 0x61727264,
	surrender = 0x69737264,
	attack = 0x6174616b,
	atk_result = 0x72736c74,
	exchg_map = 0x6d796d70,
};

void ToBytes(std::ostream &os, const PfNwPacket &x) {
	ToBytes(os, uint32_t(x));
}

void FromBytes(std::istream &is, PfNwPacket &x) {
	FromBytes(is, *reinterpret_cast<uint32_t *>(&x));
}

std::shared_ptr<PfRemotePlayer> PfCreateRemoteServer(std::string sIP, const PfPlayer &o) {
	auto p = std::make_shared<PfRemotePlayer>();
	p->as = p->pos_server;

	p->sock = tcp::socket(ioctxt); // TODO: check back later
	tcp::resolver resolver(ioctxt);
	asio::connect(p->sock, resolver.resolve(sIP, "51937"));

	std::ostream os(&p->sendbuf);
	os << "PLAY " << pfProtoNameStr << " " << pfProtoVerStr << "\r\n"
	   << "User-Agent: "s << pfUA << "\r\n"
	   << "\r\n";
	asio::write(p->sock, p->sendbuf);

	asio::read_until(p->sock, p->buf, "\r\n");

	std::istream is(&p->buf);
	PfCheckReqLn(is);

	// read and check header line.
	std::string hdLn;
	do {
		std::getline(is, hdLn);
		if(!hdLn.empty() && hdLn.back() == '\r') hdLn.pop_back();
		if(!hdLn.empty()) {
			auto p = hdLn.find(": ");
			if(p == std::string::npos) {
				std::clog << "[!] in " << __PRETTY_FUNCTION__ << ", read invalid header:\n";
				std::clog << hdLn << "\n";
			} else {
				/* TODO */
			}
		}
	} while(!hdLn.empty());

	ToBytes(os, 0u, PfNwPacket::join);
	asio::write(p->sock, p->sendbuf);

	p->tSock = std::thread(&PfRemotePlayer::SockHandler, &*p);
	return p;
}

PfRemotePlayer::PfRemotePlayer(): PfPlayer(), as(pos_null), sock(ioctxt) {}

void PfRemotePlayer::ArrangeReady() {
	PfPlayer::ArrangeReady();
}

void PfRemotePlayer::OnGameStart() {}

void PfRemotePlayer::SockHandler() {
	try {
		std::istream is(&buf);
		std::ostream os(&sendbuf);
		bool _run = 1;
		while(_run) {
			asio::read(sock, buf, asio::transfer_exactly(8));
			uint32_t len;
			PfNwPacket type;
			FromBytes(is, len, type);
			asio::read(sock, buf, asio::transfer_exactly(len));

			switch(type) {
			case PfNwPacket::join: {
				if(as == pos_client) {
					ToBytes(os,
					        11u,
					        PfNwPacket::game_info,
					        uint16_t(game.gamerules.w),
					        uint16_t(game.gamerules.h),
					        uint16_t(game.gamerules.n),
					        uint8_t(game.gamerules.cw | game.gamerules.cd << 1 | !game.gamerules.isFirst << 2),
					        uint32_t(game.id));
					bf1.resize(game.gamerules.w, game.gamerules.h);
					if(auto o = other.lock()) {
						o->NewGame(InvertIsFirst(game.gamerules), game.id);
						
						const auto &oName = o->GetName();
						ToBytes(os, uint32_t(oName.length()), PfNwPacket::name);
						os.write(oName.c_str(), oName.length());
					}
					asio::write(sock, sendbuf);
				} else {
					std::clog << "[!] in " << __PRETTY_FUNCTION__ << ":\nUnexpected Join packet received from server.\n";
				}
				is.ignore(len);
				break;
			}
			case PfNwPacket::game_info: {
				if(len < 7) {
					std::clog << "[!] in " << __PRETTY_FUNCTION__ << ":\nReceived too short Game-info packet whose len is " << len << ".\n";
					is.ignore(len);
					break;
				}
				uint16_t w, h, n;
				uint8_t st;
				uint32_t id;
				FromBytes(is, w, h, n, st);
				if(len < 11) {
					id = rng();
					is.ignore(len - 7);
				} else {
					FromBytes(is, id);
					is.ignore(len - 11);
				}
				if(as == pos_server) {
					curGame = {w, h, n, !!(st & 1), !!(st & 2), !!(st & 4)};
					this->NewGame(curGame, id);
					bf1.resize(game.gamerules.w, game.gamerules.h);
					curGame.isFirst = !(st & 4);
					if(auto o = other.lock()) {
						o->NewGame(curGame, id);
						const auto &oName = o->GetName();
						ToBytes(os, uint32_t(oName.length()), PfNwPacket::name);
						os.write(oName.c_str(), oName.length());
					}
				} else {
					std::clog << "[!] in " << __PRETTY_FUNCTION__ << ":\nUnexpected Game-info packet received from client.\n";
				}
				break;
			}
			case PfNwPacket::name: {
				name.resize(len);
				is.read(name.data(), len);
				RefreshPage();
				break;
			}
			case PfNwPacket::give_up: {
				_run = false;
				expectDisconnect = 1;
				is.ignore(len);
				Giveup();
				break;
			}
			case PfNwPacket::surrender: {
				Surrender();
				is.ignore(len);
				break;
			}
			case PfNwPacket::ready: {
				ArrangeReady();
				is.ignore(len);
				break;
			}
			case PfNwPacket::attack: {
				if(len != 4) {
					throw TT("Error: Bad game message sent from the other player.");
				}
				uint16_t ax, ay;
				FromBytes(is, ax, ay);
				Attack(ax, ay);
				break;
			}
			case PfNwPacket::atk_result: {
				if(len != 1) {
					throw TT("Error: Bad game message sent from the other player.");
				}
				uint8_t res;
				FromBytes(is, res);

				++game.turn;
				if(auto o = other.lock()) {
					o->AttackResulted(PfAtkRes(res));
				}
				if(PfAtkRes(res) == PfAtkRes::destroy) {
					++game.nDestroyedMine;
				}
				break;
			}
			case PfNwPacket::exchg_map: {
				myBf.resize(game.gamerules.w, game.gamerules.h);
				for(unsigned i = 0; i < len / 5; ++i) {
					uint16_t x, y;
					uint8_t dir;
					FromBytes(is, x, y, dir);
					myBf.basic_placeplane(x, y, dir, true);
				}
				if(auto o = other.lock()) {
					o->SetOthersBF(myBf.pl);
				}
				othersMapReceived = 1;
				if(game.Over()) {
					expectDisconnect = 1;
				}
				break;
			}
			default: {
				throw TT("Error: Bad game message sent from the other player.");
			}
			}
		}
	} catch(const std::string &t) {
		sock.shutdown(sock.shutdown_both);
		showErrorMsg(t);
		return;
	} catch(const boost::system::system_error &e) {
		if(!expectDisconnect) {
			showErrorMsg(TT("Failed to receive message: connection lost."));
		}
		return;
	} catch(const std::exception &e) {
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " some exception occured! " << e.what() << std::endl;
		return;
	}
	sock.shutdown(sock.shutdown_both);
}

void PfRemotePlayer::OnOtherReady() {
	try {
		std::ostream os(&sendbuf);
		ToBytes(os, 0u, PfNwPacket::ready);
		asio::write(sock, sendbuf);

		PfPlayer::OnOtherReady();
	} catch(const boost::system::system_error &e) {
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " network error: " << e.what() << std::endl;
		showErrorMsg(TT("Failed to send message: connection lost."), othersMapReceived ? PfPage::gameover : PfPage::main);
	}
}

void PfRemotePlayer::OnOtherGiveup() {
	try {
		expectDisconnect = 1;
		std::ostream os(&sendbuf);
		ToBytes(os, 0u, PfNwPacket::give_up);
		asio::write(sock, sendbuf);

		sock.shutdown(sock.shutdown_both);
	} catch(const boost::system::system_error &e) {
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " network error: " << e.what() << std::endl;
	}
}

void PfRemotePlayer::OnOtherSurrender() {
	PfPlayer::OnOtherSurrender();
	try {
		std::ostream os(&sendbuf);
		ToBytes(os, 0u, PfNwPacket::surrender);
		asio::write(sock, sendbuf);
	} catch(const boost::system::system_error &e) {
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " network error: " << e.what() << std::endl;
		showErrorMsg(TT("Failed to send message: connection lost."), othersMapReceived ? PfPage::gameover : PfPage::main);
	}
}

void PfRemotePlayer::OnGameover() {
	if(othersMapReceived) {
		expectDisconnect = 1;
	}
}

void PfRemotePlayer::BeingAttacked(short x, short y) {
	try {
		std::ostream os(&sendbuf);
		ToBytes(os, 4u, PfNwPacket::attack, (uint16_t)x, (uint16_t)y);
		asio::write(sock, sendbuf);
	} catch(const boost::system::system_error &e) {
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " network error: " << e.what() << std::endl;
		showErrorMsg(TT("Failed to send message: connection lost."), othersMapReceived ? PfPage::gameover : PfPage::main);
	}
	// Here base class BeingAttacked func cannot be called because it immediately
	// respond with an attack result according to myBf, which is unknown now.
	// Turn and destroy count increment is done at the arrival of network packet atk_result
}

void PfRemotePlayer::AttackResulted(PfAtkRes res) {
	try {
		std::ostream os(&sendbuf);
		ToBytes(os, 1u, PfNwPacket::atk_result, (uint8_t)res);
		asio::write(sock, sendbuf);
		
		PfPlayer::AttackResulted(res);
	} catch(const boost::system::system_error &e) {
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " network error: " << e.what() << std::endl;
		showErrorMsg(TT("Failed to send message: connection lost."), othersMapReceived ? PfPage::gameover : PfPage::main);
	}
}

void PfRemotePlayer::MapRequested() {
	if(auto o = other.lock()) {
		o->MapRequested();
	}
}

void PfRemotePlayer::SetOthersBF(const std::vector<short> &pl) {
	othersBf.pl = pl;
	try {
		std::ostream os(&sendbuf);
		ToBytes(os, uint32_t(game.gamerules.n * 5), PfNwPacket::exchg_map);
		const auto &w = game.gamerules.w;
		for(size_t i = 0; i < pl.size(); ++i) {
			if(pl[i] & 8) {
				ToBytes(os, uint16_t(i % w), uint16_t(i / w), uint8_t(pl[i] & 3));
			}
		}
		asio::write(sock, sendbuf);
	} catch(const boost::system::system_error &e) {
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " network error: " << e.what() << std::endl;
		showErrorMsg(TT("Failed to send message: connection lost."), othersMapReceived ? PfPage::gameover : PfPage::main);
	}
}

PfRemotePlayer::~PfRemotePlayer() {
	expectDisconnect = 1;
	try {
		sock.close();
		if(tSock.joinable()) tSock.join();
	} catch(const std::exception &e) {
		std::clog << "[E] in " << __PRETTY_FUNCTION__ << " error: " << e.what() << std::endl;
	}
}
