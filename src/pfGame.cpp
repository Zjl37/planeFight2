#include "pfGame.hpp"

extern std::mt19937 rng;

pfGameInfo curGame;
pfBF bg, bf1, bf2, bf3;
pfTextElem playername, enemyname;

pfRePosCh plShape[4][10] = {
	{ { 0, 0, "\u2503" }, { -2, 1, "\u2501" }, { -1, 1, "\u2501" }, { 0, 1, "\u254b" }, { 1, 1, "\u2501" }, { 2, 1, "\u2501" }, { 0, 2, "\u2503" }, { -1, 3, "\u2501" }, { 0, 3, "\u253b" }, { 1, 3, "\u2501" } },
	{ { -3, -1, "\u2503" }, { -3, 0, "\u2523" }, { -3, 1, "\u2503" }, { -2, 0, "\u2501" }, { -1, -2, "\u2503" }, { -1, -1, "\u2503" }, { -1, 0, "\u254b" }, { -1, 1, "\u2503" }, { -1, 2, "\u2503" }, { 0, 0, "\u2501" } },
	{ { -1, -3, "\u2501" }, { 0, -3, "\u2533" }, { 1, -3, "\u2501" }, { 0, -2, "\u2503" }, { -2, -1, "\u2501" }, { -1, -1, "\u2501" }, { 0, -1, "\u254b" }, { 1, -1, "\u2501" }, { 2, -1, "\u2501" }, { 0, 0, "\u2503" } },
	{ { 3, -1, "\u2503" }, { 3, 0, "\u252b" }, { 3, 1, "\u2503" }, { 2, 0, "\u2501" }, { 1, -2, "\u2503" }, { 1, -1, "\u2503" }, { 1, 0, "\u254b" }, { 1, 1, "\u2503" }, { 1, 2, "\u2503" }, { 0, 0, "\u2501" } }
};

void pfBF::resize(short nw, short nh) {
	w = nw, h = nh;
	ch.clear(), ch.resize(w * h);
	pl.clear(), pl.resize(w * h);
	mk.clear(), mk.resize(w * h);
}
void pfBF::clear() {
	resize(w, h);
}
void pfBF::draw(bool forceClear) {
	int _lastFgc = dfc, _lastBgc = dbc;
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
					cout << "  ";
				else
					cout << ch[i * w + j];
			}
		}
	} else {
		for(short i = 0; i < h; i++) {
			for(short j = 0; j < w; j++) {
				if(!ch[i * w + j].empty()) {
					gotoXY(x + j * 2, y + i);
					_uptColor(i, j);
					cout << ch[i * w + j];
				}
			}
		}
	}
}
void pfBF::basic_placeplane(short x, short y, short d, bool cw) {
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
}
bool pfBF::placeplane(short x, short y, short d, bool cw) {
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
