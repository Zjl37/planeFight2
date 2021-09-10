#include "pfUI.hpp"
#include "vtsFilter.hpp"
#include <mutex>
#include <thread>
using namespace std;

extern std::string mapEdge[256];
extern std::mt19937 rng;

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
	this_thread::sleep_for(100ms);
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
	this_thread::sleep_for(chrono::milliseconds(rng() % 250));
	setColor(black, dbc);
	gotoXY((scrW - tmp2.length()) / 2, 9);
	cout << tmp2;
	this_thread::sleep_for(chrono::milliseconds(rng() % 250));
	setColor(grey, dbc);
	gotoXY((scrW - tmp2.length()) / 2, 9);
	cout << tmp2;
	this_thread::sleep_for(chrono::milliseconds(rng() % 250));
	setColor(white, dbc);
	gotoXY((scrW - tmp2.length()) / 2, 9);
	cout << tmp2;
	this_thread::sleep_for(chrono::milliseconds(500 + rng() % 250));
}

void DrawPlane(int x, int y, int d) {
	for(const auto &ps: plShape[d]) {
		int tx = x + ps.dx * 2, ty = y + ps.dy;
		gotoXY(tx, ty);
		cout << ps.ch;
	}
}
void DrawPlane(int x, int y, int d, int bx, int by, int bw, int bh) {
	for(const auto &ps: plShape[d])
		if(x + ps.dx * 2 >= bx + bw * 2 || x + ps.dx * 2 < bx || y + ps.dy >= by + bh || y + ps.dy < by)
			return;
	for(const auto &ps: plShape[d]) {
		int tx = x + ps.dx * 2, ty = y + ps.dy;
		gotoXY(tx, ty);
		cout << ps.ch;
	}
}
void DrawPlaneCw(int x, int y, int d, int bx, int by, int bw, int bh) {
	for(const auto &ps: plShape[d]) {
		int tx = x + ps.dx * 2, ty = y + ps.dy;
		if(tx < bx) tx += bw * 2;
		if(tx >= bx + bw * 2) tx -= bw * 2;
		if(ty < by) ty += bh * 2;
		if(ty >= by + bh) ty -= bh;
		gotoXY(tx, ty);
		cout << ps.ch;
	}
}

void drawPark(int selDir, int y) {
	if(selDir == 0)
		setColor(black, aqua);
	else
		setColor(black, white);
	clearR(4, y, 17, y + 6);
	DrawPlane(10, y + 2, 0);
	if(selDir == 1)
		setColor(black, aqua);
	else
		setColor(black, white);
	clearR(20, y, 33, y + 6);
	DrawPlane(28, y + 3, 1);
	if(selDir == 2)
		setColor(black, aqua);
	else
		setColor(black, white);
	clearR(36, y, 49, y + 6);
	DrawPlane(42, y + 4, 2);
	if(selDir == 3)
		setColor(black, aqua);
	else
		setColor(black, white);
	clearR(52, y, 65, y + 6);
	DrawPlane(56, y + 3, 3);
}

void DrawBF(const pfBF &bf1, const pfBF &bf2, int x1, int y1, int x2, int y2) {
	setDefaultColor();
	box(x1, y1, bf1.w * 2, bf1.h, 2);
	box(x2, y2, bf2.w * 2, bf2.h, 2);
	for(int i = 0; i < bf1.w; i++) {
		int j = i;
		while(j && j % 10 == 0) j /= 10;
		gotoXY(x1 + i * 2, y1 - 2), std::cout << std::setw(2) << j % 10;
		gotoXY(x2 + i * 2, y2 - 2), std::cout << std::setw(2) << j % 10;
	}
	for(int i = 0; i < bf1.h; i++) {
		gotoXY(x1 - 4, y1 + i), std::cout << i;
		gotoXY(x2 - 4, y2 + i), std::cout << i;
	}
	bf1.Draw(x1, y1, true);
	bf2.Draw(x2, y2, true);
}

///////////////////////////////////////////////////////////////////////////////

pfTextElem errMsg;
std::stack<PfPage> stPage;

bool _SetPage(PfPage);

void showErrorMsg(const pfTextElem &t, PfPage rpage) {
	errMsg = t;
	while(stPage.size() > 1) stPage.pop();
	stPage.push(rpage);
	_SetPage(PfPage::error);
	stPage.push(PfPage::error);
	refreshPage();
}
void showErrorMsg(const pfTextElem &t) {
	errMsg = t;
	_SetPage(PfPage::error);
	stPage.push(PfPage::error);
	refreshPage();
}

void SetPage(PfPage x) {
	if(_SetPage(x)) {
		if(stPage.size()) stPage.pop();
		stPage.push(x);
	}
	refreshPage();
}

void NextPage(PfPage x) {
	if(_SetPage(x))
		stPage.push(x);
	refreshPage();
}

void PrevPage() {
	if(stPage.size()) {
		stPage.pop();
		_SetPage(stPage.top());
	} else {
		NextPage(PfPage::main);
	}
	refreshPage();
}

void UiGameStart() {
	banner(text[36], scrH / 3, white, pink);
	this_thread::sleep_for(1s);
	SetPage(PfPage::game);
}

void UiGameover() {
	SetPage(PfPage::gameover);
}

void UiShowAtkRes(PfAtkRes res) {
	if(res == PfAtkRes::empty) {
		setColor(black, green);
	} else if(res == PfAtkRes::hit) {
		setColor(white, red);
	} else if(res == PfAtkRes::destroy) {
		setColor(white, darkRed);
	}
	gotoXY((scrW - text[41 + (int)res].len()) / 2, 7), cout << text[41 + (int)res].s;
	this_thread::sleep_for(1s);
}
