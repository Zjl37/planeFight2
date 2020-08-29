#include "pfUI.hpp"
#include "pfLang.hpp"
using namespace std;

extern HANDLE hIn, hOut;
extern SMALL_RECT winr;
extern CONSOLE_CURSOR_INFO cci;
extern CONSOLE_SCREEN_BUFFER_INFO csbi; 

short getX_(HANDLE hStdout) {
	GetConsoleScreenBufferInfo(hStdout, &csbi); 
	return csbi.dwCursorPosition.X;
}

short getY_(HANDLE hStdout) {
	GetConsoleScreenBufferInfo(hStdout, &csbi); 
	return csbi.dwCursorPosition.Y;
}

void gotoX_(short x, HANDLE hStdout) {
	GetConsoleScreenBufferInfo(hStdout,&csbi);
	csbi.dwCursorPosition.X = x;
	SetConsoleCursorPosition(hStdout,csbi.dwCursorPosition);   
}

void gotoY_(short y, HANDLE hStdout) {
	GetConsoleScreenBufferInfo(hStdout,&csbi);
	csbi.dwCursorPosition.Y = y;
	SetConsoleCursorPosition(hStdout,csbi.dwCursorPosition);
}

void gotoXY_(short x, short y, HANDLE hStdout) {
	GetConsoleScreenBufferInfo(hStdout,&csbi);
	csbi.dwCursorPosition.X = x;
	csbi.dwCursorPosition.Y = y;
	SetConsoleCursorPosition(hStdout,csbi.dwCursorPosition);
}

void setColor_(short fgc, short bgc, HANDLE hStdout) {
	SetConsoleTextAttribute(hStdout,fgc+(bgc<<4));
}

void showCursor_(bool f, HANDLE hStdout) {
	cci.dwSize=1, cci.bVisible=f;
	SetConsoleCursorInfo(hStdout,&cci);
}

void banner(const pfTextElem &msg, short h, short fgc, short bgc) {
	setColor(fgc,bgc);
	gotoXY(0,h); cout<<setw(winr.Right)<<" ";
	gotoXY(0,h+1); cout<<setw(winr.Right)<<" ";
	gotoXY(0,h+2); cout<<setw(winr.Right)<<" ";
	gotoXY((winr.Right-msg.len())/2,h+1);
	cout<<msg.s;
	setDefaultColor();
}

void pfLabel::draw() {
	if(!~x||!~y) return;
	setColor(fgc,bgc);
	if(w) {
		gotoXY(x,y-1), cout<<setw(t.len())<<"";
		gotoXY(x,y+1), cout<<setw(t.len())<<"";
	}
	gotoXY(x,y), cout<<t.s;
}

void pfLabel::_click() {
	setColor(fgc2,bgc2);
	if(w) {
		gotoXY(x,y-1), cout<<setw(t.len())<<"";
		gotoXY(x,y+1), cout<<setw(t.len())<<"";
	}
	gotoXY(x,y), cout<<t.s;
	Sleep(100);
	clickFunc();
}

bool pfLabel::click(short mx, short my) {
	if(clickFunc==nullptr) return false;
	if(my<y-1||my>y+1||(!w&&my!=y)||mx<x||mx>x+t.len()) return false;
	_click();
	return true;
}

short pfLabel::right() {
	return x+t.len();
}

void box(short x, short y, short w, short h, short edge) {
	// edge style: 0=single 1=bold 2=double
	for(int i=x; i<x+w; i+=2)
		if(edge==0) {
			gotoXY(i,y-1), cout<<"\u2500 ";
			gotoXY(i,y+h), cout<<"\u2500 ";
		} else if(edge==1) {
			gotoXY(i,y-1), cout<<"\u2501 ";
			gotoXY(i,y+h), cout<<"\u2501 ";
		} else if(edge==2) {
			gotoXY(i,y-1), cout<<"\u2550 ";
			gotoXY(i,y+h), cout<<"\u2550 ";
		}
	for(int j=y; j<y+h; j++)
		if(edge==0) {
			gotoXY(x-2,j), cout<<"\u2502 ";
			gotoXY(x+w,j), cout<<"\u2502 ";
		} else if(edge==1) {
			gotoXY(x-2,j), cout<<"\u2503 ";
			gotoXY(x+w,j), cout<<"\u2503 ";
		} else if(edge==2) {
			gotoXY(x-2,j), cout<<"\u2551 ";
			gotoXY(x+w,j), cout<<"\u2551 ";
		}
	if(edge==0) {
		gotoXY(x-2,y-1), cout<<"\u250c ";
		gotoXY(x+w,y-1), cout<<"\u2510 ";
		gotoXY(x-2,y+h), cout<<"\u2514 ";
		gotoXY(x+w,y+h), cout<<"\u2518 ";
	} else if(edge==1) {
		gotoXY(x-2,y-1), cout<<"\u250f ";
		gotoXY(x+w,y-1), cout<<"\u2513 ";
		gotoXY(x-2,y+h), cout<<"\u2517 ";
		gotoXY(x+w,y+h), cout<<"\u251b ";
	} else if(edge==2) {
		gotoXY(x-2,y-1), cout<<"\u2554 ";
		gotoXY(x+w,y-1), cout<<"\u2557 ";
		gotoXY(x-2,y+h), cout<<"\u255a ";
		gotoXY(x+w,y+h), cout<<"\u255d ";
	}
}

void clearR(short l, short t, short r, short b) {
	for(int j=t; j<=b; j++) {
		for(int i=l; i<=r; i++) {
			gotoXY(i,j);
			putchar(' ');
		}
	}
}