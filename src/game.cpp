/**
 * Copyright © 2021-2022 Zjl37 <2693911885@qq.com>
 * Copyright © 2021 qwqAutomaton
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

#include "game.hpp"
#include "pfLocale.hpp"

extern std::mt19937 rng;

PfGameInfo curGame{ 10, 10, 3, false, false, false };
std::shared_ptr<PfLocalPlayer> player0;
std::shared_ptr<PfPlayer> player1;

PfRePosCh plShape[4][10] = {
	{{0, 0, "┃ "}, {-2, 1, "━━"}, {-1, 1, "━━"}, {0, 1, "╋━"}, {1, 1, "━━"}, {2, 1, "━ "}, {0, 2, "┃ "}, {-1, 3, "━━"}, {0, 3, "┻━"}, {1, 3, "━ "}},
	{{-3, -1, "┃ "}, {-3, 0, "┣━"}, {-3, 1, "┃ "}, {-2, 0, "━━"}, {-1, -2, "┃ "}, {-1, -1, "┃ "}, {-1, 0, "╋━"}, {-1, 1, "┃ "}, {-1, 2, "┃ "}, {0, 0, "━ "}},
	{{-1, -3, "━━"}, {0, -3, "┳━"}, {1, -3, "━ "}, {0, -2, "┃ "}, {-2, -1, "━━"}, {-1, -1, "━━"}, {0, -1, "╋━"}, {1, -1, "━━"}, {2, -1, "━ "}, {0, 0, "┃ "}},
	{{3, -1, "┃ "}, {3, 0, "┫ "}, {3, 1, "┃ "}, {2, 0, "━━"}, {1, -2, "┃ "}, {1, -1, "┃ "}, {1, 0, "╋━"}, {1, 1, "┃ "}, {1, 2, "┃ "}, {0, 0, "━━"}}
};

PfBF::PfBF(): w(10), h(10), ch(1*w*h), pl(1*w*h), mk(1*w*h) {}
PfBF::PfBF(int w, int h): w(w), h(h), ch(1*w*h), pl(1*w*h), mk(1*w*h) {}
void PfBF::resize(int nw, int nh) {
	w = nw, h = nh;
	nPlaced = 0;
	ch.clear(), ch.resize(w * h);
	pl.clear(), pl.resize(w * h);
	mk.clear(), mk.resize(w * h);
}
void PfBF::clear() {
	resize(w, h);
}
void PfBF::basic_placeplane(int x, int y, short d, bool cw) {
	for(int i = 0; i < 10; i++) {
		short tx = x + plShape[d][i].dx, ty = y + plShape[d][i].dy;
		if(cw) {
			if(tx < 0) tx += w;
			if(tx >= w) tx -= w;
			if(ty < 0) ty += h;
			if(ty >= h) ty -= h;
		}
		ch[ty * w + tx] = plShape[d][i].ch;
		pl[ty * w + tx] = d | 4;
	}
	pl[y * w + x] |= 8;
	++nPlaced;
}
bool PfBF::TestPlace(int x, int y, short d, bool cw) {
	if(!cw)
		for(int i = 0; i < 10; i++)
			if(x + plShape[d][i].dx >= w || x + plShape[d][i].dx < 0 || y + plShape[d][i].dy >= h || y + plShape[d][i].dy < 0) return false;
	for(int i = 0; i < 10; i++) {
		short tx = x + plShape[d][i].dx, ty = y + plShape[d][i].dy;
		if(cw) {
			if(tx < 0) tx += w;
			if(tx >= w) tx -= w;
			if(ty < 0) ty += h;
			if(ty >= h) ty -= h;
		}
		if(pl[ty * w + tx]) return false;
	}
	return true;
}
bool PfBF::placeplane(int x, int y, short d, bool cw) {
	if(!TestPlace(x, y, d, cw)) return false;
	basic_placeplane(x, y, d, cw);
	return true;
}
void PfBF::RemovePlane(int x, int y) {
	if(pl[x + y * w] & 8) {
		int d = pl[x + y * w] & 3;
		for(auto &[dx, dy, _]: plShape[d]) {
			auto tx = (x + dx + w) % w, ty = (y + dy + h) % h;
			ch[ty * w + tx] = "";
			pl[ty * w + tx] = 0;
		}
		--nPlaced;
	}
}
bool PfBF::AutoArrange() {
	nPlaced = 0;
	int ttry = 0;
	clear();
	while(nPlaced < curGame.n && ttry < 10000) {
		placeplane(rng() % w, rng() % h, rng() & 3, curGame.cw);
		++ttry;
	}
	if(nPlaced < curGame.n) {
		clear();
		return false;
	}
	return true;
}

PfGameInfo InvertIsFirst(const PfGameInfo &gi) {
	return {gi.w, gi.h, gi.n, gi.cw, gi.cd, !gi.isFirst};
}

PfGame::PfGame(): state(0) {}
PfGame::PfGame(PfGameInfo gi, unsigned id): id(id), gamerules(gi), state(0) {}

bool PfGame::isMyTurn() const {
	return (turn & 1) ^ 1 ^ gamerules.isFirst;
}

bool PfGame::Over() const {
	return state & me_surrender || state & other_surrender || nDestroyedMine == gamerules.n || nDestroyedOthers == gamerules.n;
}

void PfPlayer::NewGame(const PfGameInfo &gi, unsigned id) {
	game = {gi, id};
	game.nDestroyedOthers = game.nDestroyedMine = 0;
	othersBf.resize(gi.w, gi.h);
}

void PfPlayer::OnGameStart() {
	game.turn = 1;
}

void PfPlayer::ArrangeReady() {
	game.state |= game.me_ready;
	if((game.state & game.ready) == game.ready) {
		OnGameStart();
	}
	player0->OnOtherReady();
}

void PfPlayer::OnOtherReady() {
	game.state |= game.other_ready;
	if((game.state & game.ready) == game.ready) {
		OnGameStart();
	}
}

void PfPlayer::Giveup() {
	player0->OnOtherGiveup();
}

void PfPlayer::Surrender() {
	game.state |= game.me_surrender;
	OnGameover();
	player0->OnOtherSurrender();
}

void PfPlayer::OnOtherSurrender() {
	game.state |= game.other_surrender;
	OnGameover();
}

void PfPlayer::Attack(short x, short y) {
	player0->BeingAttacked(x, y);
}

void PfPlayer::BeingAttacked(short x, short y) {
	PfAtkRes res;
	const auto &w = myBf.w;
	const auto &h = myBf.h;
	if(myBf.pl[x + y * w] & 8) {
		if(curGame.cd) {
			short d = myBf.pl[x + y * w] & 3;
			for(int i = 0; i < 10; i++) {
				short tx = x + plShape[d][i].dx, ty = y + plShape[d][i].dy;
				if(curGame.cw) {
					if(tx < 0) tx += w;
					if(tx >= w) tx -= w;
					if(ty < 0) ty += h;
					if(ty >= h) ty -= h;
				}
				myBf.pl[tx + ty * w] |= 16;
			}
		}
		myBf.mk[x + y * w] = PfBF::destroy;
		res = PfAtkRes::destroy;
	} else if(myBf.pl[x + y * w] & 4 && !(myBf.pl[x + y * w] & 16)) {
		myBf.mk[x + y * w] = PfBF::hit;
		res = PfAtkRes::hit;
	} else {
		myBf.mk[x + y * w] = PfBF::empty;
		res = PfAtkRes::empty;
	}
	++game.turn;
	player0->AttackResulted(res);
	if(res == PfAtkRes::destroy) {
		++game.nDestroyedMine;
	}
}

void PfPlayer::AttackResulted(PfAtkRes res) {
	++game.turn;
	if(res == PfAtkRes::destroy) {
		++game.nDestroyedOthers;
	}
}

const std::string &PfPlayer::GetName() const {
	return name;
}

void PfPlayer::MapRequested() {
	player0->SetOthersBF(myBf.pl);
}

void PfPlayer::SetOthersBF(const std::vector<short> &pl) {
	othersBf.pl = pl;
}

void PfPlayer::OnOtherGiveup() {}
void PfPlayer::OnGameover() {}

PfLocalPlayer::PfLocalPlayer(const std::string &t) {
	name = t;
}

void PfLocalPlayer::NewGame(const PfGameInfo &gi, unsigned id) {
	game = {gi, id};
	game.nDestroyedOthers = game.nDestroyedMine = 0;
	othersBf.resize(gi.w, gi.h);
	SetPage(PfPage::prepare);
}

void PfLocalPlayer::Giveup() {
	player1->OnOtherGiveup();
}

void PfLocalPlayer::OnGameStart() {
	game.turn = 1;
	UiGameStart();
}

void PfLocalPlayer::ArrangeReady(const PfBF &ar) {
	myBf = ar;
	game.state |= game.me_ready;
	if((game.state & game.ready) == game.ready) {
		OnGameStart();
	}
	player1->OnOtherReady();
}

void PfLocalPlayer::OnOtherReady() {
	game.state |= game.other_ready;
	if((game.state & game.ready) == game.ready) {
		OnGameStart();
	}
}

void PfLocalPlayer::OnGameover() {
	UiGameover();
}

void PfLocalPlayer::Attack(short x, short y) {
	BlinkCoord(x, y, 1);
	lastAtk = {x, y};
	player1->BeingAttacked(x, y);
}

void PfLocalPlayer::AttackResulted(PfAtkRes res) {
	++game.turn;
	if(res == PfAtkRes::destroy) {
		++game.nDestroyedOthers;
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = PfBF::destroy;
	} else if(res == PfAtkRes::hit) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = PfBF::hit;
	} else if(res == PfAtkRes::empty) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = PfBF::empty;
	}
	UiShowAtkRes(res);
	RefreshPage();
	if(res == PfAtkRes::destroy && game.nDestroyedOthers == curGame.n) {
		player1->MapRequested();
	}
}

void PfLocalPlayer::BeingAttacked(short x, short y) {
	BlinkCoord(x, y, 0);

	PfAtkRes res;
	const auto &w = myBf.w, &h = myBf.h;
	if(myBf.pl[x + y * w] & 8) {
		if(curGame.cd) {
			short d = myBf.pl[x + y * w] & 3;
			for(int i = 0; i < 10; i++) {
				short tx = x + plShape[d][i].dx, ty = y + plShape[d][i].dy;
				if(curGame.cw) {
					if(tx < 0) tx += w;
					if(tx >= w) tx -= w;
					if(ty < 0) ty += h;
					if(ty >= h) ty -= h;
				}
				myBf.pl[tx + ty * w] |= 16;
			}
		}
		myBf.mk[x + y * w] = PfBF::destroy;
		res = PfAtkRes::destroy;
	} else if(myBf.pl[x + y * w] & 4 && !(myBf.pl[x + y * w] & 16)) {
		myBf.mk[x + y * w] = PfBF::hit;
		res = PfAtkRes::hit;
	} else {
		myBf.mk[x + y * w] = PfBF::empty;
		res = PfAtkRes::empty;
	}
	++game.turn;
	player1->AttackResulted(res);
	if(res == PfAtkRes::destroy) {
		++game.nDestroyedMine;
	}

	UiShowAtkRes(res);
	if(game.nDestroyedMine == game.gamerules.n) {
		player1->MapRequested();
	}
	RefreshPage();
}

void PfLocalPlayer::OnOtherSurrender() {
	player1->MapRequested();
	game.state |= game.other_surrender;
	OnGameover();
}

void PfLocalPlayer::OnOtherGiveup() {
	showErrorMsg(TT("The other player gave up this game."), PfPage::main);
}

void PfLocalPlayer::Surrender() {
	player1->MapRequested();
	game.state |= game.me_surrender;
	OnGameover();
	player1->OnOtherSurrender();
}

void PfLocalPlayer::SetOthersBF(const std::vector<short> &pl) {
	othersBf.pl = pl;
	for(int i = 0; i < othersBf.h; i++)
		for(int j = 0; j < othersBf.w; j++)
			if(othersBf.pl[j + i * othersBf.w] & 8) {
				othersBf.basic_placeplane(j, i, othersBf.pl[j + i * game.gamerules.w] & 3, game.gamerules.cw);
			}
	if(game.Over()) {
		SetPage(PfPage::gameover);
	}
}

const PfGame &PfLocalPlayer::GetGame() const {
	return game;
}
const std::string &PfLocalPlayer::GetName() const {
	return name;
}
const PfBF &PfLocalPlayer::GetMyBF() const {
	return myBf;
}
const PfBF &PfLocalPlayer::GetOthersBF() const {
	return othersBf;
}
