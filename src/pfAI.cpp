#include "pfAI.hpp"
#include "pfLocale.hpp"
#include "pfConsole.hpp"
#include <random>

extern std::mt19937 rng;

pfBF vbf;

bool pfAIinit(const pfGameInfo &g, const std::vector<short> &mk) {
	vbf.clear();
	int k = 0;
	for(short i = 0; i < g.h; i++)
		for(short j = 0; j < g.w; j++)
			if(mk[j + i * g.w] == darkRed) {
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

bool pfAIcheck(const pfGameInfo &g, const std::vector<short> &mk) {
	for(short j = 0; j < g.h; j++)
		for(short i = 0; i < g.w; i++) {
			if(!g.cd && mk[i + j * g.w] == green && vbf.pl[i + j * g.w])
				return false;
			if(mk[i + j * g.w] == red && !vbf.pl[i + j * g.w])
				return false;
		}
	return true;
}

#define PFAI_MAXTRY 1000000
bool pfAIdecide(const pfGameInfo &g, const std::vector<short> &mk, short &tgx, short &tgy) {
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
			if(vbf.pl[j + i * g.w] & 8 && mk[j + i * g.w] != darkRed)
				pos.push_back({j, i});
	if(!pos.size()) return false;
	int i = rng() % pos.size();
	tgx = pos[i].first, tgy = pos[i].second;
	return true;
}

PfAI::PfAI(): PfPlayer() {
	name = TT("AI");
}

void PfAI::ArrangeReady(const pfBF &ar) {
	PfPlayer::ArrangeReady();
	myBf = ar;
}

void PfAI::BeingAttacked(short x, short y) {
	PfPlayer::BeingAttacked(x, y);
	if(!game.Over()) {
		short ax, ay;
		short ret = pfAIdecide(game.gamerules, othersBf.mk, ax, ay);
		if(!ret) {
			Surrender();
			return;
		}
		Attack(ax, ay);
	}
}

void PfAI::AttackResulted(PfAtkRes res) {
	PfPlayer::AttackResulted(res);
	if(res == PfAtkRes::destroy) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = darkRed;
	} else if(res == PfAtkRes::hit) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = red;
	} else if(res == PfAtkRes::empty) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = green;
	}
}

void PfAI::OnGameStart() {
	PfPlayer::OnGameStart();
	if(game.isFirst) {
		short ax, ay;
		short ret = pfAIdecide(game.gamerules, othersBf.mk, ax, ay);
		if(!ret) {
			Surrender();
			return;
		}
		Attack(ax, ay);
	}
}

void PfAI::Attack(short x, short y) {
	lastAtk = {x, y};
	PfPlayer::Attack(x, y);
}
