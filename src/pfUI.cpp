#include "pfGame.hpp"
#include "pfUI.hpp"
#include "pfConsole.hpp"
#include "pfLang.hpp"
#include "vtsFilter.hpp"
#include <mutex>
using namespace std;

extern std::string mapEdge[256];

int scrW, scrH;

void banner(const pfTextElem &msg, short h, short fgc, short bgc) {
	setColor(fgc, bgc);
	gotoXY(0, h);
	cout << setw(scrW) << " ";
	gotoXY(0, h + 1);
	cout << setw(scrW) << " ";
	gotoXY(0, h + 2);
	cout << setw(scrW) << " ";
	gotoXY((scrW - msg.len()) / 2, h + 1);
	cout << msg.s;
	setDefaultColor();
}

void pfLabel::draw() {
	if(!~x || !~y) return;
	setColor(fgc, bgc);
	if(w) {
		gotoXY(x, y - 1), cout << setw(t.len()) << "";
		gotoXY(x, y + 1), cout << setw(t.len()) << "";
	}
	gotoXY(x, y), cout << t.s;
}

void pfLabel::_click() {
	mtxCout.lock();
	setColor(fgc2, bgc2);
	if(w) {
		gotoXY(x, y - 1), cout << setw(t.len()) << "";
		gotoXY(x, y + 1), cout << setw(t.len()) << "";
	}
	gotoXY(x, y), cout << t.s;
	mtxCout.unlock();
	Sleep(100);
	clickFunc();
}

bool pfLabel::click(short mx, short my) {
	if(clickFunc == nullptr) return false;
	if(my < y - 1 || my > y + 1 || (!w && my != y) || mx < x || mx > x + t.len()) return false;
	_click();
	return true;
}

short pfLabel::right() {
	return x + t.len();
}

void box(short x, short y, short w, short h, short edge) {
	// edge style: 0=single 1=bold 2=double
	for(int i = x; i < x + w; i += 2)
		if(edge == 0) {
			gotoXY(i, y - 1), cout << mapEdge[0];
			gotoXY(i, y + h), cout << mapEdge[0];
		} else if(edge == 1) {
			gotoXY(i, y - 1), cout << mapEdge[1];
			gotoXY(i, y + h), cout << mapEdge[1];
		} else if(edge == 2) {
			gotoXY(i, y - 1), cout << mapEdge[6];
			gotoXY(i, y + h), cout << mapEdge[6];
		}
	for(int j = y; j < y + h; j++)
		if(edge == 0) {
			gotoXY(x - 2, j), cout << mapEdge[2];
			gotoXY(x + w, j), cout << mapEdge[2];
		} else if(edge == 1) {
			gotoXY(x - 2, j), cout << mapEdge[3];
			gotoXY(x + w, j), cout << mapEdge[3];
		} else if(edge == 2) {
			gotoXY(x - 2, j), cout << mapEdge[7];
			gotoXY(x + w, j), cout << mapEdge[7];
		}
	if(edge == 0) {
		gotoXY(x - 2, y - 1), cout << mapEdge[4];
		gotoXY(x + w, y - 1), cout << mapEdge[12];
		gotoXY(x - 2, y + h), cout << mapEdge[13];
		gotoXY(x + w, y + h), cout << mapEdge[14];
	} else if(edge == 1) {
		gotoXY(x - 2, y - 1), cout << mapEdge[5];
		gotoXY(x + w, y - 1), cout << mapEdge[15];
		gotoXY(x - 2, y + h), cout << mapEdge[16];
		gotoXY(x + w, y + h), cout << mapEdge[17];
	} else if(edge == 2) {
		gotoXY(x - 2, y - 1), cout << mapEdge[8];
		gotoXY(x + w, y - 1), cout << mapEdge[9];
		gotoXY(x - 2, y + h), cout << mapEdge[10];
		gotoXY(x + w, y + h), cout << mapEdge[11];
	}
}

void BlinkCoord(short ax, short ay, bool signDir) {
	stringstream tmp1("");
	if(signDir)
		tmp1 << ">>>>  ";
	else
		tmp1 << "<<<<  ";
	tmp1 << "(" << ax << "," << ay << ")";
	if(signDir)
		tmp1 << "  >>>>";
	else
		tmp1 << "  <<<<";
	string tmp2 = tmp1.str();
	setColor(lightGrey, dbc);
	gotoXY((scrW - tmp2.length()) / 2, 9);
	cout << tmp2;
	Sleep(rng() % 250);
	setColor(black, dbc);
	gotoXY((scrW - tmp2.length()) / 2, 9);
	cout << tmp2;
	Sleep(rng() % 250);
	setColor(grey, dbc);
	gotoXY((scrW - tmp2.length()) / 2, 9);
	cout << tmp2;
	Sleep(rng() % 250);
	setColor(white, dbc);
	gotoXY((scrW - tmp2.length()) / 2, 9);
	cout << tmp2;
	Sleep(500 + rng() % 250);
}

void drawPlane(short x, short y, short d, bool r) {
	// direction: 0=up 1=right 2=down 3=left
	if(r && !curGame.cw)
		for(int i = 0; i < 10; i++)
			if(x + plShape[d][i].dx * 2 >= bf1.x + bf1.w * 2 || x + plShape[d][i].dx * 2 < bf1.x || y + plShape[d][i].dy >= bf1.y + bf1.h || y + plShape[d][i].dy < bf1.y)
				return;
	for(int i = 0; i < 10; i++) {
		int tx = x + plShape[d][i].dx * 2, ty = y + plShape[d][i].dy;
		if(r) {
			if(tx < bf1.x) tx += bf1.w * 2;
			if(tx >= bf1.x + bf1.w * 2) tx -= bf1.w * 2;
			if(ty < bf1.y) ty += bf1.h * 2;
			if(ty >= bf1.y + bf1.h) ty -= bf1.h;
		}
		gotoXY(tx, ty);
		cout << plShape[d][i].ch;
	}
}

void drawPark(int selDir) {
	if(selDir == 0)
		setColor(black, aqua);
	else
		setColor(black, white);
	clearR(4, 10 + curGame.h, 17, 16 + curGame.h);
	drawPlane(10, 12 + curGame.h, 0, false);
	if(selDir == 1)
		setColor(black, aqua);
	else
		setColor(black, white);
	clearR(20, 10 + curGame.h, 33, 16 + curGame.h);
	drawPlane(28, 13 + curGame.h, 1, false);
	if(selDir == 2)
		setColor(black, aqua);
	else
		setColor(black, white);
	clearR(36, 10 + curGame.h, 49, 16 + curGame.h);
	drawPlane(42, 14 + curGame.h, 2, false);
	if(selDir == 3)
		setColor(black, aqua);
	else
		setColor(black, white);
	clearR(52, 10 + curGame.h, 65, 16 + curGame.h);
	drawPlane(56, 13 + curGame.h, 3, false);
}

void drawBF(bool showBf2) {
	bf1.x = 4, bf1.y = bf2.y = bf3.y = 5;
	bf2.x = bf3.x = scrW - 4 - bf2.w * 2;
	setDefaultColor();
	box(bf1.x, bf1.y, bf1.w * 2, bf1.h, 2);
	box(bf2.x, bf2.y, bf2.w * 2, bf2.h, 2);
	for(int i = 0; i < curGame.w; i++) {
		int j = i;
		while(j && j % 10 == 0) j /= 10;
		gotoXY(bf1.x + i * 2, bf1.y - 2), cout << setw(2) << j % 10;
		gotoXY(bf2.x + i * 2, bf2.y - 2), cout << setw(2) << j % 10;
	}
	for(int i = 0; i < curGame.h; i++) {
		gotoXY(bf1.x - 4, bf1.y + i), cout << i;
		gotoXY(bf2.x - 4, bf2.y + i), cout << i;
	}
	gotoXY(bf1.x, 2), cout << playername.s << endl;
	gotoXY(bf2.x, 2), cout << enemyname.s << endl;
	bf1.draw(true);
	if(showBf2)
		bf2.draw(true);
	else
		bf3.draw(true);
}
