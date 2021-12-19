#include "pfAI.hpp"
#include "pfLocale.hpp"
#include <random>
#include <future>
#include <thread>

extern std::mt19937 rng;

PfBF vbf;


bool pfAIinit(const pfGameInfo &g, const std::vector<PfBF::AttackRecord> &mk) {
	vbf.clear();
	int k = 0;
	for(short i = 0; i < g.h; i++)
		for(short j = 0; j < g.w; j++)
			if(mk[j + i * g.w] == PfBF::destroy) {
				if(vbf.pl[j + i * g.w]) return false;
				bool ret = 0;
				char r = 0;
				do {
					short d = rng() & 3;
					ret = vbf.placeplane(j, i, d, g.cw);
					if(!ret) r |= 1 << d;
				} while(!ret && r != 15);
				if(r == 15) return false;
				++k;
			}
	int wcl23 = 0;
	while(k < g.n) {
		short x = rng() % g.w, y = rng() % g.h;
		while(vbf.pl[x + y * g.w] || mk[x + y * g.w]) {
			x = rng() % g.w, y = rng() % g.h;
		}
		if(vbf.placeplane(x, y, rng() & 3, g.cw))
			++k;
		++wcl23;
		if(wcl23 > 64)
			return false;
	}
	return true;
}

bool pfAIcheck(const pfGameInfo &g, const std::vector<PfBF::AttackRecord> &mk) {
	for(short j = 0; j < g.h; j++)
		for(short i = 0; i < g.w; i++) {
			if(!g.cd && mk[i + j * g.w] == PfBF::empty && vbf.pl[i + j * g.w])
				return false;
			if(mk[i + j * g.w] == PfBF::hit && !vbf.pl[i + j * g.w])
				return false;
		}
	return true;
}

#define PFAI_MAXTRY 1000000
bool pfAIdecide(const pfGameInfo &g, const std::vector<PfBF::AttackRecord> &mk, short &tgx, short &tgy) {
	vbf.resize(g.w, g.h);
	int ttt = 0;
	do {
		bool ret = false;
		while(!ret && ttt < PFAI_MAXTRY) {
			ret = pfAIinit(g, mk);
			++ttt;
		}
	} while(!pfAIcheck(g, mk) && ttt < PFAI_MAXTRY);
	if(ttt >= PFAI_MAXTRY) {
		return false;
	}
	std::vector<std::pair<int, int>> pos;
	for(short i = 0; i < g.h; i++)
		for(short j = 0; j < g.w; j++)
			if(vbf.pl[j + i * g.w] & 8 && mk[j + i * g.w] != PfBF::destroy)
				pos.push_back({j, i});
	if(!pos.size()) return false;
	int i = rng() % pos.size();
	tgx = pos[i].first, tgy = pos[i].second;
	return true;
}

PfAI::PfAI(): PfPlayer() {
	name = TT("AI");
}

void PfAI::ArrangeReady(const PfBF &ar) {
	PfPlayer::ArrangeReady();
	myBf = ar;
}

void PfAI::BeingAttacked(short x, short y) {
	PfPlayer::BeingAttacked(x, y);
	if(!game.Over()) {
		// asyncrhonously make a decision, so that the main thread can handle UI.
		static std::future<void> fut;
		if(fut.valid()) fut.wait();
		fut = std::async(std::launch::async, [&]() {
			using namespace std::chrono_literals;
			
			auto t0 = std::chrono::system_clock::now();
			short ax, ay;
			short ret = pfAIdecide(game.gamerules, othersBf.mk, ax, ay);
			std::this_thread::sleep_until(t0 + 2s);
			if(ret) {
				Attack(ax, ay);
			} else {
				Surrender();
			}
		});
	}
}

void PfAI::AttackResulted(PfAtkRes res) {
	PfPlayer::AttackResulted(res);
	if(res == PfAtkRes::destroy) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = PfBF::destroy;
	} else if(res == PfAtkRes::hit) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = PfBF::hit;
	} else if(res == PfAtkRes::empty) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = PfBF::empty;
	}
}

void PfAI::OnGameStart() {
	PfPlayer::OnGameStart();
	if(game.isFirst) {
		// asyncrhonously make a decision, so that the main thread can handle UI.
		static std::future<void> fut;
		if(fut.valid()) fut.wait();
		fut = std::async(std::launch::async, [&]() {
			using namespace std::chrono_literals;
			
			auto t0 = std::chrono::system_clock::now();
			short ax, ay;
			short ret = pfAIdecide(game.gamerules, othersBf.mk, ax, ay);
			std::this_thread::sleep_until(t0 + 2s);
			if(ret) {
				Attack(ax, ay);
			} else {
				Surrender();
			}
		});
	}
}

void PfAI::Attack(short x, short y) {
	lastAtk = {x, y};
	PfPlayer::Attack(x, y);
}
