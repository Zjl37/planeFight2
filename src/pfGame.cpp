#include "pfGame.hpp"
#include "pfLocale.hpp"
#include "pfConsole.hpp"

extern std::mt19937 rng;

pfGameInfo curGame{ 10, 10, 3, false, false };
std::shared_ptr<PfPlayer> player[2];

PfRePosCh plShape[4][10] = {
	{{0, 0, "┃ "}, {-2, 1, "━━"}, {-1, 1, "━━"}, {0, 1, "╋━"}, {1, 1, "━━"}, {2, 1, "━ "}, {0, 2, "┃ "}, {-1, 3, "━━"}, {0, 3, "┻━"}, {1, 3, "━ "}},
	{{-3, -1, "┃ "}, {-3, 0, "┣━"}, {-3, 1, "┃ "}, {-2, 0, "━━"}, {-1, -2, "┃ "}, {-1, -1, "┃ "}, {-1, 0, "╋━"}, {-1, 1, "┃ "}, {-1, 2, "┃ "}, {0, 0, "━ "}},
	{{-1, -3, "━━"}, {0, -3, "┳━"}, {1, -3, "━ "}, {0, -2, "┃ "}, {-2, -1, "━━"}, {-1, -1, "━━"}, {0, -1, "╋━"}, {1, -1, "━━"}, {2, -1, "━ "}, {0, 0, "┃ "}},
	{{3, -1, "┃ "}, {3, 0, "┫ "}, {3, 1, "┃ "}, {2, 0, "━━"}, {1, -2, "┃ "}, {1, -1, "┃ "}, {1, 0, "╋━"}, {1, 1, "┃ "}, {1, 2, "┃ "}, {0, 0, "━━"}}
};

pfBF::pfBF(): w(10), h(10), ch(1*w*h), pl(1*w*h), mk(1*w*h) {}
pfBF::pfBF(int w, int h): w(w), h(h), ch(1*w*h), pl(1*w*h), mk(1*w*h) {}
void pfBF::resize(int nw, int nh) {
	w = nw, h = nh;
	nPlaced = 0;
	ch.clear(), ch.resize(w * h);
	pl.clear(), pl.resize(w * h);
	mk.clear(), mk.resize(w * h);
}
void pfBF::clear() {
	resize(w, h);
}
void pfBF::Draw(int x, int y, bool forceClear) const {
	int _lastFgc = dfc, _lastBgc = dbc;
	setDefaultColor();
	// NOTE: the performance of colored output is not as fast as non-colored ones.
	// ref (on Windows): https://github.com/microsoft/terminal/issues/10362
	// avoid frequently changing color to get better perf.
	auto _uptColor = [&](int i, int j) {
		short bgc = mk[i * w + j] ? mk[i * w + j] : 16;
		if(_lastFgc != hcc[bgc] || _lastBgc != bgc) {
			_lastFgc = hcc[bgc], _lastBgc = bgc;
			setColor(hcc[bgc], bgc);
		}
	};
	if(forceClear) {
		for(short i = 0; i < h; i++) {
			gotoY(y + i);
			for(short j = 0; j < w; j++) {
				gotoX(x + j * 2);
				_uptColor(i, j);
				if(ch[i * w + j].empty())
					std::cout << "  ";
				else
					std::cout << ch[i * w + j];
			}
		}
	} else {
		for(short i = 0; i < h; i++) {
			for(short j = 0; j < w; j++) {
				if(!ch[i * w + j].empty()) {
					gotoXY(x + j * 2, y + i);
					_uptColor(i, j);
					std::cout << ch[i * w + j];
				}
			}
		}
	}
	std::cout << std::flush;
}
void pfBF::basic_placeplane(int x, int y, short d, bool cw) {
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
bool pfBF::placeplane(int x, int y, short d, bool cw) {
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
	basic_placeplane(x, y, d, cw);
	return true;
}
bool pfBF::AutoArrange() {
	nPlaced = 0;
	int ttry = 0;
	clear();
	while(nPlaced < curGame.n && ttry < 10000) {
		if(placeplane(rng() % w, rng() % h, rng() & 3, curGame.cw))
			++nPlaced;
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
		myBf.mk[x + y * w] = darkRed;
		res = PfAtkRes::destroy;
	} else if(myBf.pl[x + y * w] & 4 && !(myBf.pl[x + y * w] & 16)) {
		myBf.mk[x + y * w] = red;
		res = PfAtkRes::hit;
	} else {
		myBf.mk[x + y * w] = green;
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

const pfBF &PfPlayer::GetMyBF() const {
	return myBf;
}
const pfBF &PfPlayer::GetOthersBF() const {
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

void PfLocalPlayer::ArrangeReady(const pfBF &ar) {
	myBf = ar;
	PfPlayer::ArrangeReady();
}

void PfLocalPlayer::OnGameover() {
	UiGameover();
}

void PfLocalPlayer::Attack(short x, short y) {
	{
		std::lock_guard<std::mutex> _lg(mtxCout);
		BlinkCoord(x, y, 1);
	}
	lastAtk = {x, y};
	PfPlayer::Attack(x, y);
}

void PfLocalPlayer::AttackResulted(PfAtkRes res) {
	PfPlayer::AttackResulted(res);
	if(res == PfAtkRes::destroy) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = darkRed;
	} else if(res == PfAtkRes::hit) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = red;
	} else if(res == PfAtkRes::empty) {
		othersBf.mk[lastAtk.x + lastAtk.y * curGame.w] = green;
	}
	{
		std::lock_guard<std::mutex> _lg(mtxCout);
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
		std::lock_guard<std::mutex> _lg(mtxCout);
		BlinkCoord(x, y, 0);
	}
	PfPlayer::BeingAttacked(x, y);
	{
		std::lock_guard<std::mutex> _lg(mtxCout);
		auto dummy = myBf.mk[x + y * myBf.w];
		PfAtkRes res = dummy == darkRed ? PfAtkRes::destroy :
		               dummy == red     ? PfAtkRes::hit :
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
