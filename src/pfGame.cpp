#include "pfGame.hpp"
#include "pfLocale.hpp"

extern std::mt19937 rng;

pfGameInfo curGame{ 10, 10, 3, false, false };
std::shared_ptr<PfPlayer> player[2];

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

PfGame::PfGame(): state(0) {}
PfGame::PfGame(pfGameInfo gi, unsigned id, bool ff): id(id), gamerules(gi), isFirst(ff), state(0) {}

bool PfGame::isMyTurn() const {
	return (turn & 1) ^ 1 ^ isFirst;
}

bool PfGame::Over() const {
	return state & me_surrender || state & other_surrender || nDestroyedMine == gamerules.n || nDestroyedOthers == gamerules.n;
}

const PfGame &PfPlayer::GetGame() const {
	return game;
}

void PfPlayer::NewGame(const pfGameInfo &gi, unsigned id, bool isFirst) {
	game = {gi, id, isFirst};
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
	if(auto o = other.lock()) {
		o->OnOtherReady();
	}
}

void PfPlayer::OnOtherReady() {
	game.state |= game.other_ready;
	if((game.state & game.ready) == game.ready) {
		OnGameStart();
	}
}

void PfPlayer::Giveup() {
	if(auto o = other.lock()) {
		o->OnOtherGiveup();
	}
}

void PfPlayer::Surrender() {
	game.state |= game.me_surrender;
	OnGameover();
	if(auto o = other.lock()) {
		o->OnOtherSurrender();
	}
}

void PfPlayer::OnOtherSurrender() {
	game.state |= game.other_surrender;
	OnGameover();
}

void PfPlayer::Attack(short x, short y) {
	if(auto o = other.lock()) {
		o->BeingAttacked(x, y);
	}
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
	if(auto o = other.lock()) {
		o->AttackResulted(res);
	}
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
	if(auto o = other.lock()) {
		o->SetOthersBF(myBf.pl);
	}
}

void PfPlayer::SetOthersBF(const std::vector<short> &pl) {
	othersBf.pl = pl;
}

void PfPlayer::OnOtherGiveup() {}
void PfPlayer::OnGameover() {}

const PfBF &PfPlayer::GetMyBF() const {
	return myBf;
}
const PfBF &PfPlayer::GetOthersBF() const {
	return othersBf;
}

PfLocalPlayer::PfLocalPlayer(const std::string &t): PfPlayer() {
	name = t;
}

void PfLocalPlayer::NewGame(const pfGameInfo &gi, unsigned id, bool isFirst) {
	PfPlayer::NewGame(gi, id, isFirst);
	SetPage(PfPage::prepare);
}

void PfLocalPlayer::OnGameStart() {
	PfPlayer::OnGameStart();
	UiGameStart();
}

void PfLocalPlayer::ArrangeReady(const PfBF &ar) {
	myBf = ar;
	PfPlayer::ArrangeReady();
}

void PfLocalPlayer::OnGameover() {
	UiGameover();
}

void PfLocalPlayer::Attack(short x, short y) {
	{
		// std::lock_guard<std::mutex> _lg(mtxCout);
		BlinkCoord(x, y, 1);
	}
	lastAtk = {x, y};
	PfPlayer::Attack(x, y);
}

void PfLocalPlayer::AttackResulted(PfAtkRes res) {
	PfPlayer::AttackResulted(res);
	if(res == PfAtkRes::destroy) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = PfBF::destroy;
	} else if(res == PfAtkRes::hit) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = PfBF::hit;
	} else if(res == PfAtkRes::empty) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = PfBF::empty;
	}
	{
		// std::lock_guard<std::mutex> _lg(mtxCout);
		UiShowAtkRes(res);
	}
	RefreshPage();
	if(res == PfAtkRes::destroy && game.nDestroyedOthers == curGame.n) {
		if(auto o = other.lock()) {
			o->MapRequested();
		}
	}
}

void PfLocalPlayer::BeingAttacked(short x, short y) {
	{
		// std::lock_guard<std::mutex> _lg(mtxCout);
		BlinkCoord(x, y, 0);
	}
	PfPlayer::BeingAttacked(x, y);
	{
		// std::lock_guard<std::mutex> _lg(mtxCout);
		auto dummy = myBf.mk[x + y * myBf.w];
		PfAtkRes res = dummy == PfBF::destroy ? PfAtkRes::destroy :
		               dummy == PfBF::hit     ? PfAtkRes::hit :
                                          PfAtkRes::empty;
		UiShowAtkRes(res);
		if(game.nDestroyedMine == game.gamerules.n) {
			if(auto o = other.lock()) {
				o->MapRequested();
			}
		}
	}
	RefreshPage();
}

void PfLocalPlayer::OnOtherSurrender() {
	if(auto o = other.lock()) {
		o->MapRequested();
	}
	PfPlayer::OnOtherSurrender();
}

void PfLocalPlayer::OnOtherGiveup() {
	showErrorMsg(TT("The other player gave up this game."), PfPage::main);
}

void PfLocalPlayer::Surrender() {
	if(auto o = other.lock()) {
		o->MapRequested();
	}
	PfPlayer::Surrender();
}

void PfLocalPlayer::SetOthersBF(const std::vector<short> &pl) {
	PfPlayer::SetOthersBF(pl);
	for(int i = 0; i < othersBf.h; i++)
		for(int j = 0; j < othersBf.w; j++)
			if(othersBf.pl[j + i * othersBf.w] & 8) {
				othersBf.basic_placeplane(j, i, othersBf.pl[j + i * GetGame().gamerules.w] & 3, GetGame().gamerules.cw);
			}
	if(game.Over()) {
		SetPage(PfPage::gameover);
	}
}
