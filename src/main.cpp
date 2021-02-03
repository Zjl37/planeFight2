#include <io.h>
#include <fcntl.h>
#include <ctime>
#include <vector>
#include <sstream>
#include <winsock2.h>
#include "pfGame.hpp"
#include "pfUI.hpp"
#include "pfLang.hpp"
#include "pfAI.hpp"

using namespace std;

#define pfVersion "2.3"
#define pfVerStr "planefight 2.3"

const int PF_NMARKER = 23;

const string marker[PF_NMARKER]={
	"\u2501 ","\u2503 ","\u254b ","\u2523 ","\u252b ","\u2533 ","\u253b ",
	"\u2500 ","\u2502 ","\u253c ","\u251c ","\u2524 ","\u252c ","\u2534 ",
	"\u2550 ","\u2551 ","\u256c ","\u2560 ","\u2563 ","\u2566 ","\u2569 ",
	"\uff1f","\uff01"
};
const int P1_NNLUE = 5;

bool _fl_ = 1;
bool isFirst;
int page,tab[16],nue,turn;
DWORD nEvents;
HANDLE hIn,hOut,hOutOrg,hErr;
SMALL_RECT winr={0,0,80,30};
INPUT_RECORD rec;
CONSOLE_CURSOR_INFO cci;
CONSOLE_SCREEN_BUFFER_INFO csbi;
pfLabel ue[128];
pfTextElem playername,enemyname,errMsg;
pfGameInfo curGame;

void pfBF::resize(short nw, short nh) {
	w=nw, h=nh;
	ch.clear(), ch.resize(w*h);
	pl.clear(), pl.resize(w*h);
	mk.clear(), mk.resize(w*h);
}
void pfBF::clear() {
	resize(w,h);
}
void pfBF::draw(bool forceClear) {
	for(short i=0; i<h; i++) {
		if(forceClear) gotoY(y+i);
		for(short j=0; j<w; j++) {
			if(forceClear) {
				gotoX(x+j*2);
				setColor(hcc[mk[i*w+j]],mk[i*w+j]);
				if(ch[i*w+j].empty()) cout<<"  ";
				else cout<<ch[i*w+j];
			} else if(!ch[i*w+j].empty()) {
				gotoXY(x+j*2,y+i);
				setColor(hcc[mk[i*w+j]],mk[i*w+j]);
				cout<<ch[i*w+j];
			}
		}
	}
}
void pfBF::basic_placeplane(short x, short y, short d, bool cw) {
	for(int i=0; i<10; i++) {
		short tx=x+plShape[d][i].dx, ty=y+plShape[d][i].dy;
		if(cw) {
			if(tx<0) tx+=w;
			if(tx>=w) tx-=w;
			if(ty<0) ty+=h;
			if(ty>=h) ty-=h;
		}
		ch[ty*w+tx]=plShape[d][i].ch;
		pl[ty*w+tx]=d|4;
	}
	pl[y*w+x]|=8;
}
bool pfBF::placeplane(short x, short y, short d, bool cw) {
	if(!cw)
		for(int i=0; i<10; i++)
			if(x+plShape[d][i].dx>=w||x+plShape[d][i].dx<0||y+plShape[d][i].dy>=h||y+plShape[d][i].dy<0) return false;
	for(int i=0; i<10; i++) {
		short tx=x+plShape[d][i].dx, ty=y+plShape[d][i].dy;
		if(cw) {
			if(tx<0) tx+=w;
			if(tx>=w) tx-=w;
			if(ty<0) ty+=h;
			if(ty>=h) ty-=h;
		}
		if(pl[ty*w+tx]) return false;
	}
	basic_placeplane(x,y,d,cw);
	return true;
}
pfBF bg,bf1,bf2,bf3;

int p2npl,p2isP1Ready,p2isP2Ready;
int p10des1,p10des2,p10srd;
WSADATA wsaData;
char buf[65536],sendbuf[65536]="pf",tmpbuf[65536];
string sIP;

void setPage(int);

void conInit() {
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOutOrg = GetStdHandle(STD_OUTPUT_HANDLE);
	hErr = GetStdHandle(STD_ERROR_HANDLE);
	hOut = CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetStdHandle(STD_OUTPUT_HANDLE, hOut);
	SetConsoleActiveScreenBuffer(hOut);
	fclose(stdout);
	*stdout = *_fdopen(_open_osfhandle((intptr_t)hOut, _O_APPEND), "w");
	/*
		Create a new CSB for this program so that the
		original console screen get reserved after exit.
		Redirect stdout to the new CSB.
	*/

	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);

	GetConsoleScreenBufferInfo(hOut, &csbi);
	csbi.dwSize.X = max((short)80, csbi.dwSize.X);
	csbi.dwSize.Y = max((short)30, csbi.dwSize.Y);
	if(csbi.srWindow.Right<80 || csbi.srWindow.Bottom<30) {
		SetConsoleScreenBufferSize(hOut, csbi.dwSize);
		csbi.srWindow.Right = csbi.dwSize.X-1;
		csbi.srWindow.Bottom = csbi.dwSize.Y-1;
		SetConsoleWindowInfo(hOut, true, &csbi.srWindow);
	}
	winr=csbi.srWindow;

	DWORD mode;
	GetConsoleMode(hIn,&mode);
	mode |= ENABLE_PROCESSED_INPUT;
	mode |= ENABLE_MOUSE_INPUT;
	mode |= ENABLE_QUICK_EDIT_MODE;
	mode -= ENABLE_QUICK_EDIT_MODE;
	mode |= ENABLE_WINDOW_INPUT;
	SetConsoleMode(hIn,mode);
}

int pfCheckVer(const string &s) {
	stringstream curVer(pfVerStr), iVer(s);
	char ch;
	int curVerSec1=0, iVerSec1=0, curVerSec2=0, iVerSec2=0;
	string curGName, iGName;
	curVer>>curGName>>curVerSec1>>ch>>curVerSec2;
	iVer>>iGName>>iVerSec1>>ch>>iVerSec2;
	if(curGName!=iGName||curVerSec1!=iVerSec1)
		return -1;
	if(curVerSec2!=iVerSec2) return 1;
	return 0;
}

void showErrorMsg(const pfTextElem &t, int rpage) {
	errMsg=t;
	setPage(rpage^0x80000000);
}

bool getIP() {
	int ret=gethostname(buf,65536);
	if(ret==-1) return false;
	struct hostent *host;
	host=gethostbyname(buf);
	if(host==NULL) return false;
	sIP=inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);
	return true;
}
SOCKET sockServer,sockClient;
SOCKADDR_IN addrServer,addrClient;
int hg;
bool pfCheckMsg(const char *msg, const char *i) {
	if(msg[0]!='p'||msg[1]!='f') {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71],1);
		return false;
	}
	if(*(int*)(msg+2)!=hg) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71],1);
		return false;
	}
	if(!i) return true;
	int len=strlen(i);
	memcpy(tmpbuf,msg+6,len);
	tmpbuf[len]=0;
	if(strcmp(tmpbuf,i)) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71],1);
		return false;
	}
	return true;
}
bool pfServerInit() {
	setDefaultColor(), clear();
	ue[0].draw();
	if(WSAStartup(MAKEWORD(2,2),&wsaData)) {
		showErrorMsg(text[52],1);
		return false;
	}
	sIP=""; getIP();
	sockServer=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sockServer==INVALID_SOCKET) {
		WSACleanup();
		showErrorMsg(text[56],1);
		return false;
	}
	addrServer.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	addrServer.sin_family=AF_INET;
	addrServer.sin_port=htons(51937);
	int ret=bind(sockServer,(SOCKADDR*)&addrServer,sizeof(SOCKADDR));
	if(ret==SOCKET_ERROR) {
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[58],1);
		return false;
	}
	ret=listen(sockServer,1);
	if(ret==SOCKET_ERROR) {
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[60],1);
		return false;
	}
	return true;
}
bool pfServerAccept() {
	int len=sizeof(SOCKADDR), ret=0;
	sockClient=accept(sockServer,(SOCKADDR*)&addrClient,&len);
	if(sockClient==INVALID_SOCKET)
		return false;
	closesocket(sockServer);
	gotoXY(0,getY()+1), cout<<text[63].s;
	ret=recv(sockClient,buf,65536,0);
	// check return value
	ret=pfCheckVer(buf);
	if(ret==-1) {
		strcpy(sendbuf+6,"!ver");
		send(sockClient,sendbuf,11,0);
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[64]+(string)buf,1);
		return false;
	} else if(ret==1) {
		gotoXY(0,getY()+1), cout<<text[65].s<<buf;
	}
	hg=*(int*)(sendbuf+2)=rand()<<15|rand();
	strcpy(sendbuf+6,"hello");
	ret=send(sockClient,sendbuf,11,0);
	// check ret
	strcpy(sendbuf+6,"gameinfo");
	memcpy(sendbuf+14,&curGame,sizeof curGame);
	ret=send(sockClient,sendbuf,14+sizeof curGame,0);
	// check ret
	strcpy(sendbuf+6,"name");
	*(int*)(sendbuf+10)=playername.d;
	strcpy(sendbuf+14,playername.s.c_str());
	ret=send(sockClient,sendbuf,15+playername.s.length(),0);
	// check ret
	ret=recv(sockClient,buf,65535,0);
	// check ret
	if(!pfCheckMsg(buf,"name"))
		return false;
	enemyname=pfTextElem(string(buf+14),*(int*)(buf+10));
	return true;
}
bool pfClientInit() {
	if(WSAStartup(MAKEWORD(2,2),&wsaData)) {
		showErrorMsg(text[52],1);
		return false;
	}
	gotoXY(0,3), cout<<text[53].s;
	sockClient=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sockServer==INVALID_SOCKET) {
		WSACleanup();
		showErrorMsg(text[66],1);
		return false;
	}
	gotoXY(0,getY()+1), cout<<text[67].s;
	gotoXY(0,getY()+1);
	return true;
}
bool pfClientConnect() {
	addrServer.sin_addr.S_un.S_addr=inet_addr(sIP.c_str());
	addrServer.sin_family=AF_INET;
	addrServer.sin_port=htons(51937);
	int ret=connect(sockClient,(SOCKADDR*)&addrServer,sizeof(SOCKADDR));
	if(ret==SOCKET_ERROR) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[68],1);
		return false;
	}
	gotoXY(0,getY()+1), cout<<text[69].s;
	ret=send(sockClient,pfVerStr,strlen(pfVerStr)+1,0);
	// check ret
	ret=recv(sockClient,buf,11,0);
	buf[11]=0;
	if(strcmp(buf+6,"!ver")==0) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[70],1);
		return false;
	} else if(buf[0]!='p'||buf[1]!='f') {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71],1);
		return false;
	} else if(strcmp(buf+6,"hello")) {
		closesocket(sockClient);
		WSACleanup();
		showErrorMsg(text[71],1);
		return false;
	}
	hg=*(int*)(sendbuf+2)=*(int*)(buf+2);
	ret=recv(sockClient,buf,14+sizeof curGame,0);
	if(!pfCheckMsg(buf,"gameinfo"))
		return false;
	memcpy(&curGame,buf+14,sizeof curGame);
	curGame.d=-1;
	strcpy(sendbuf+6,"name");
	*(int*)(sendbuf+10)=playername.d;
	strcpy(sendbuf+14,playername.s.c_str());
	ret=send(sockClient,sendbuf,15+playername.s.length(),0);
	// check ret
	ret=recv(sockClient,buf,65535,0);
	// check ret
	if(!pfCheckMsg(buf,"name"))
		return false;
	enemyname=pfTextElem(string(buf+14),*(int*)(buf+10));
	return true;
}

void pfExchangeMap() {
	strcpy(sendbuf+6,"mp");
	for(int i=0; i<(int)bf1.ch.size(); i++) {
		*(short*)(sendbuf+8)=bf1.pl[i];
		int ret=send(sockClient,sendbuf,10,0);
		if(ret<=0) {
			showErrorMsg(text[85],1); break;
		}
		ret=recv(sockClient,buf,10,0);
		if(ret<=0) {
			showErrorMsg(text[86],1); break;
		}
		if(!pfCheckMsg(buf,"mp"))
			return;
		bf2.pl[i]=*(short*)(buf+8);
	}
	closesocket(sockClient);
	WSACleanup();
	for(int i=0; i<bf2.h; i++)
		for(int j=0; j<bf2.w; j++)
			if(bf2.pl[j+i*bf2.w]&8) {
				bf2.basic_placeplane(j,i,bf2.pl[j+i*bf2.w]&3,curGame.cw);
			}
}

bool isMyTurn() {
	return (turn&1)^1^isFirst;
}

void refreshPage();

void uptCursorState() {
	if(page==0||page==51)
		showCursor();
	else
		hideCursor();
}

void setPage(int x) {
	if(x==2) {
		memset(tab,0,sizeof tab);
		isFirst=rand()&1;
		if(curGame.d==2) {
			if(!curGame.n) {
				curGame.n=3;
				bf1.resize(10,10), bf2.resize(10,10), bf3.resize(10,10);
				curGame.w=curGame.h=10;
			} else {
				bf1.clear(), bf2.clear(), bf3.clear();
			}
		} else {
			bf1.resize(curGame.w,curGame.h),
			bf2.resize(curGame.w,curGame.h),
			bf3.resize(curGame.w,curGame.h);
		}
		p2isP1Ready=p2isP2Ready=p2npl=0;
	} else if(x==10) {
		memset(tab,0,sizeof tab);
		p10des1=p10des2=p10srd=0;
	} else if(x==19) {
		bf2.mk=bf3.mk;
	} else if(x==41) {
		if(!curGame.n) {
			curGame.n=3;
			bf1.resize(10,10), bf2.resize(10,10), bf3.resize(10,10);
			curGame.w=curGame.h=10;
		} else {
			bf1.clear(), bf2.clear(), bf3.clear();
		}
	} else if(x==42) {
		curGame.d=-2;
		if(!pfServerInit()) return;
	} else if(x==51) {
		curGame.d=-1;
		sIP="";
		if(!pfClientInit()) return;
	}
	page=x;
	refreshPage();
}

void printcoord(short ax, short ay) {
	stringstream tmp1("");
	if(isMyTurn()) tmp1<<">>>>  ";
	else tmp1<<"<<<<  ";
	tmp1<<"("<<ax<<","<<ay<<")";
	if(isMyTurn()) tmp1<<"  >>>>";
	else tmp1<<"  <<<<";
	string tmp2=tmp1.str();
	setColor(lightGrey,black);
	gotoXY((winr.Right-tmp2.length())/2,9);
	cout<<tmp2;
	Sleep(rand()%250);
	setColor(black,black);
	gotoXY((winr.Right-tmp2.length())/2,9);
	cout<<tmp2;
	Sleep(rand()%250);
	setColor(grey,black);
	gotoXY((winr.Right-tmp2.length())/2,9);
	cout<<tmp2;
	Sleep(rand()%250);
	setColor(white,black);
	gotoXY((winr.Right-tmp2.length())/2,9);
	cout<<tmp2;
	Sleep(500+rand()%250);
}

void drawPlane(short x, short y, short d, bool r) {
	// direction: 0=up 1=right 2=down 3=left
	if(r && !curGame.cw)
		for(int i=0; i<10; i++)
			if(x+plShape[d][i].dx*2>=bf1.x+bf1.w*2 || x+plShape[d][i].dx*2<bf1.x || y+plShape[d][i].dy>=bf1.y+bf1.h || y+plShape[d][i].dy<bf1.y)
				return;
	for(int i=0; i<10; i++) {
		int tx=x+plShape[d][i].dx*2, ty=y+plShape[d][i].dy;
		if(r) {
			if(tx<bf1.x) tx+=bf1.w*2;
			if(tx>=bf1.x+bf1.w*2) tx-=bf1.w*2;
			if(ty<bf1.y) ty+=bf1.h*2;
			if(ty>=bf1.y+bf1.h) ty-=bf1.h;
		}
		gotoXY(tx,ty);
		cout<<plShape[d][i].ch;
	}
}

void drawPark() {
	if(tab[1]==0) setColor(black,aqua);
	else setColor(black,white);
	clearR(4,10+curGame.h,17,16+curGame.h);
	drawPlane(10,12+curGame.h,0,false);
	if(tab[1]==1) setColor(black,aqua);
	else setColor(black,white);
	clearR(20,10+curGame.h,33,16+curGame.h);
	drawPlane(28,13+curGame.h,1,false);
	if(tab[1]==2) setColor(black,aqua);
	else setColor(black,white);
	clearR(36,10+curGame.h,49,16+curGame.h);
	drawPlane(42,14+curGame.h,2,false);
	if(tab[1]==3) setColor(black,aqua);
	else setColor(black,white);
	clearR(52,10+curGame.h,65,16+curGame.h);
	drawPlane(56,13+curGame.h,3,false);
}

void drawBF(bool showBf2) {
	bf1.x=4, bf1.y=bf2.y=bf3.y=5;
	bf2.x=bf3.x=winr.Right-4-bf2.w*2;
	setDefaultColor();
	box(bf1.x,bf1.y,bf1.w*2,bf1.h,2);
	box(bf2.x,bf2.y,bf2.w*2,bf2.h,2);
	for(int i=0; i<curGame.w; i++) {
		int j=i;
		while(j && j%10==0) j/=10;
		gotoXY(bf1.x+i*2,bf1.y-2), cout<<setw(2)<<j%10;
		gotoXY(bf2.x+i*2,bf2.y-2), cout<<setw(2)<<j%10;
	}
	for(int i=0; i<curGame.h; i++) {
		gotoXY(bf1.x-4,bf1.y+i), cout<<i;
		gotoXY(bf2.x-4,bf2.y+i), cout<<i;
	}
	gotoXY(bf1.x,2), cout<<playername.s<<endl;
	gotoXY(bf2.x,2), cout<<enemyname.s<<endl;
	bf1.draw(true);
	if(showBf2) bf2.draw(true);
	else bf3.draw(true);
}

void drawUiElem() {
	setDefaultColor(), clear();
	if(page==0||page==1)
		bg.draw(false);
	for(int i=1; i<=nue; i++)
		ue[i].draw();
	if(page&0x80000000) {
		banner(errMsg,winr.Bottom/3,white,red);
		return;
	}
	if(page==0) {
		gotoXY(ue[4].right(),ue[4].y);
	} else if(page==2) {
		drawBF(false);
		if(tab[0]==0)
			drawPark();
	} else if(page==4) {
		bf1.x=4, bf1.y=5;
		box(bf1.x,bf1.y,curGame.w*2,curGame.h,2);
	} else if(page==10) {
		drawBF(false);
		gotoXY(winr.Right-10,1), cout<<"#"<<turn;
	} else if(page==19) {
		drawBF(true);
		gotoXY(winr.Right-10,1), cout<<"#"<<turn;
	} else if(page==51) {
		gotoXY(5,7), cout<<sIP;
	}
}

bool bfInit(pfBF &bf) {
	int i=0, ttry=0;
	bf.clear();
	while(i<curGame.n&&ttry<10000) {
		if(bf.placeplane(rand()%bf.w,rand()%bf.h,rand()&3,curGame.cw))
			++i;
		++ttry;
	}
	if(i<curGame.n)
		return bf.clear(), false;
	return true;
}

void p0GenBg() {
	bg.resize(winr.Right/2,winr.Bottom);
	for(int i=0; i<(int)bg.w*bg.h/25; i++)
		bg.placeplane(rand()%bg.w,rand()%bg.h,rand()&3,curGame.cw);
}

void p0InputOK() {
	if(playername.s.empty()) {
		setDefaultColor();
		refreshPage();
		return;
	}
	setDefaultColor();
	setPage(1);
}

void p1Play21() {
	setPage(41);
}
void p1Play22() {
	setPage(51);
}

void p1Play2() {
	tab[0]=!tab[0];
	refreshPage();
}

void p2Tab0() {
	tab[0]=0;
	refreshPage();
}
void p2Tab1() {
	tab[0]=1;
	refreshPage();
}
void p2Start() {
	turn=1;
	banner(text[36],winr.Bottom/3,white,pink);
	Sleep(1000);
	setPage(10);
}
void p2Ready() {
	if(p2npl!=curGame.n) return;
	if(curGame.d>0) {
		bool tmp=bfInit(bf2);
		if(tmp==false) {
			refreshPage();
			return;
		}
		p2Start();
	} else {
		int ret;
		if(!p2isP1Ready) {
			p2isP1Ready=1;
			strcpy(sendbuf+6,"ready");
			ret=send(sockClient, sendbuf, 12, 0);
			if(ret<=0) {
				showErrorMsg(text[85], 1);
				return;
			}
			if(!(curGame.d&1) && p2isP2Ready) {
				strcpy(sendbuf+6,"start");
				*(bool*)(sendbuf+11)=isFirst;
				ret=send(sockClient,sendbuf,12,0);
				p2Start();
			}
		}
	}
}
void p2Recv() {
	int ret=recv(sockClient,buf,12,0);
	if(!pfCheckMsg(buf,NULL)) return;
	memcpy(tmpbuf,buf+6,6);
	tmpbuf[6]=0;
	if(strcmp(tmpbuf,"giveup")==0) {
		closesocket(sockClient);
		showErrorMsg(text[80],1);
		return;
	}
	tmpbuf[5]=0;
	if(strcmp(tmpbuf,"ready")==0) {
		p2isP2Ready=1;
		if(!(curGame.d&1) && p2isP1Ready) {
			strcpy(sendbuf+6,"start");
			*(bool*)(sendbuf+11)=isFirst;
			ret=send(sockClient,sendbuf,12,0);
			p2Start();
		} else refreshPage();
	} else if((curGame.d&1) && strcmp(tmpbuf,"start")==0) {
		isFirst=!*(bool*)(buf+11);
		p2Start();
	} else {
		showErrorMsg(text[81],1);
		return;
	}
}
void p2SwitchCw() {
	if(curGame.cw) {
		bf1.clear();
		p2npl=0;
		curGame.cw=false;
	} else {
		curGame.cw=true;
	}
	refreshPage();
}
void p2AddPl(short x) {
	if(curGame.n+x<1||curGame.n+x>curGame.w*curGame.h/10) {
		refreshPage();
		return;
	}
	curGame.n+=x;
	if(p2npl>curGame.n) {
		p2npl=0;
		bf1.clear();
	}
	refreshPage();
}
void p2Giveup() {
	strcpy(sendbuf+6,"giveup");
	send(sockClient,sendbuf,12,0);
	setPage(1);
}

void p10Surrender() {
	p10srd=1;
	if(curGame.d<=0) {
		strcpy(sendbuf+6,"isurrender");
		send(sockClient,sendbuf,16,0);
		pfExchangeMap();
	}
	setPage(19);
}

short attackL(int x, int y, vector<short> &pl, vector<short> &mk) {
	if(mk[x+y*curGame.w]!=black)
		return -1;
	if(pl[x+y*curGame.w]&8) {
		if(curGame.cd) {
			short d=pl[x+y*curGame.w]&3;
			for(int i=0; i<10; i++) {
				short tx=x+plShape[d][i].dx, ty=y+plShape[d][i].dy;
				if(curGame.cw) {
					if(tx<0) tx+=curGame.w;
					if(tx>=curGame.w) tx-=curGame.w;
					if(ty<0) ty+=curGame.h;
					if(ty>=curGame.h) ty-=curGame.h;
				}
				pl[tx+ty*curGame.w]|=16;
			}
		}
		return mk[x+y*curGame.w]=darkRed, 2;
	}
	if(pl[x+y*curGame.w]&4 && !(pl[x+y*curGame.w]&16))
		return mk[x+y*curGame.w]=red, 1;
	return mk[x+y*curGame.w]=green, 0;
}

short attackR(short x, short y) {
	strcpy(sendbuf+6,"attack");
	*(short*)(sendbuf+12)=x, *(short*)(sendbuf+14)=y;
	int ret=send(sockClient,sendbuf,16,0);
	if(ret<=0) {
		showErrorMsg(text[85],1);
		return -1;
	}
	ret=recv(sockClient,buf,14,0);
	if(ret<=0) {
		showErrorMsg(text[86],1);
		return -1;
	}
	if(!pfCheckMsg(buf,"result"))
		return -1;
	short res=*(short*)(buf+12);
	if(res==2) {
		bf3.mk[x+y*curGame.w]=darkRed;
	} else if(res==1) {
		bf3.mk[x+y*curGame.w]=red;
	} else if(res==0) {
		bf3.mk[x+y*curGame.w]=green;
	}
	return res;
}

void showAttackMsg(short res) {
	if(res==0)
		setColor(black,green);
	else if(res==1)
		setColor(white,red);
	else if(res==2)
		setColor(white,darkRed);
	if(res>=0&&res<=2)
		gotoXY((winr.Right-text[41+res].len())/2,7), cout<<text[41+res].s;
	Sleep(1000);
	refreshPage();
}

void attack(short x, short y) {
	short res;
	if(curGame.d>0)
		res=attackL(x,y,bf2.pl,bf3.mk);
	else
		res=attackR(x,y);
	if(res<0||res>2) return;
	drawBF(false);
	printcoord(x,y);
	showAttackMsg(res);
	if(res==2) {
		++p10des2;
		if(p10des2==curGame.n) {
			if(curGame.d<=0) pfExchangeMap();
			Sleep(200);
			setPage(19);
			return;
		}
	} 
	++turn;
}

void p10EnemyTurn() {
	short res,ax,ay;
	if(curGame.d>0) {
		short ret=pfAIdecide(curGame,bf1.mk,ax,ay);
		if(!ret) {
			p10srd=2;
			setPage(19);
			return;
		}
	} else {
		int ret=recv(sockClient,buf,16,0);
		if(ret<=0) {
			showErrorMsg(text[86],1);
			return;
		}
		if(!pfCheckMsg(buf,NULL))
			return;
		memcpy(tmpbuf,buf+6,10);
		tmpbuf[10]=0;
		if(strcmp(tmpbuf,"isurrender")==0) {
			p10srd=2;
			pfExchangeMap();
			setPage(19);
			return;
		}
		if(!pfCheckMsg(buf,"attack"))
			return;
		ax=*(short*)(buf+12), ay=*(short*)(buf+14);
	}
	res=attackL(ax,ay,bf1.pl,bf1.mk);
	if(curGame.d<=0) {
		strcpy(sendbuf+6,"result");
		*(short*)(sendbuf+12)=res;
		int ret=send(sockClient,sendbuf,14,0);
		if(ret<=0) {
			showErrorMsg(text[85],1);
			return;
		}
	}
	printcoord(ax,ay);
	bf1.draw(false);
	showAttackMsg(res);
	if(res==2) {
		++p10des1;
		if(p10des1==curGame.n) {
			if(curGame.d<=0) pfExchangeMap();
			Sleep(200);
			setPage(19);
			return;
		}
	}
	++turn;
	if(res==0) {
		setColor(black,green);
	}
}

void buildUiElem() {
	stringstream tmp("");
	tmp<<left<<setw(winr.Right+text[0].d)<<text[0].s; // inner title
	ue[1]=pfLabel(pfTextElem(tmp.str(),text[0].d),0,0,white,blue,0,0,false);
	if(page&0x80000000) {
		ue[2]=pfLabel(text[11],0,1,black,yellow,black,darkYellow,false); // back
		ue[2].clickFunc=[] { setPage(page^0x80000000); };
		nue=2;
		return;
	}
	if(page==0) {
		tmp.str("");
		tmp<<setw((winr.Right-text[2].len())/2)<<""<<left<<setw(winr.Right+text[2].d-(winr.Right-text[2].len())/2)<<text[2].s;
		ue[2]=pfLabel(pfTextElem(tmp.str(),text[2].d),0,5,white,pink,0,0,true);
		ue[3]=pfLabel(text[7],2,10,black,yellow,black,darkYellow,false);
		ue[3].clickFunc=p0InputOK;
		ue[4]=pfLabel(text[6]+playername,2,8,dfc,dbc,0,0,false);
		ue[5]=pfLabel(text[92],ue[3].right()+2,10,white,red,white,darkRed,false);
		ue[5].clickFunc=[] { _fl_ = 0; };

		ue[P1_NNLUE+1]=pfLabel(lf[0].langName,2,12,dfc,dbc,grey,black,false);
		ue[P1_NNLUE+1].clickFunc=[] {
			pfLangRead(lf[0].dir.c_str()), refreshPage();
		};
		for(int i=1; i<(int)lf.size(); i++) {
			if(ue[P1_NNLUE+i].right()+2+lf[i].langName.len()>winr.Right) {
				ue[P1_NNLUE+1+i]=pfLabel(lf[i].langName,2,ue[P1_NNLUE+i].y+1,dfc,dbc,grey,black,false);
			} else {
				ue[P1_NNLUE+1+i]=pfLabel(lf[i].langName,ue[P1_NNLUE+i].right()+2,ue[P1_NNLUE+i].y,dfc,dbc,grey,black,false);
			}
			ue[P1_NNLUE+1+i].clickFunc=[i] { pfLangRead(lf[i].dir.c_str()), refreshPage(); };
		}
		nue=P1_NNLUE+lf.size();
	} else if(page==1) {
		tmp.str("");
		tmp<<setw(winr.Right-1+text[8].d)<<text[8].s;
		ue[2]=pfLabel(pfTextElem(tmp.str(),text[8].d),3,3,black,white,black,lightGrey,true);
		ue[2].clickFunc=[] { enemyname=text[37], curGame.d=2, setPage(2); };
		tmp.str("");
		tmp<<setw(winr.Right-1+text[9].d)<<text[9].s;
		ue[3]=pfLabel(pfTextElem(tmp.str(),text[9].d),6,7,black,white,black,lightGrey,true);
		ue[3].clickFunc=p1Play2;
		tmp.str("");
		tmp<<setw(winr.Right-1+text[10].d)<<text[10].s;
		ue[4]=pfLabel(pfTextElem(tmp.str(),text[10].d),9,11,black,white,black,lightGrey,true);
		ue[4].clickFunc=[] { setPage(5); };
		nue=4;

		ue[5]=pfLabel(text[16],12,15,white,red,white,darkRed,true);
		ue[5].clickFunc=[] { _fl_=0; };
		if(ue[5].right()+2+text[17].len() < winr.Right)
			ue[6]=pfLabel(text[17],ue[5].right()+2,15,black,white,black,lightGrey,true);
		else
			ue[6]=pfLabel(text[17],ue[5].x,19,black,white,black,lightGrey,true);
		ue[6].clickFunc=[] { setPage(0); };
		nue=6;
		if(tab[0]) {
			ue[7]=pfLabel(text[20],8,10,dfc,dbc,grey,dbc,false);
			ue[7].clickFunc=p1Play21;
			ue[8]=pfLabel(text[21],8,11,dfc,dbc,grey,dbc,false);
			ue[8].clickFunc=p1Play22;
			nue=8;
		}
	} else if(page==2) {
		if(curGame.d>0) {
			ue[2]=pfLabel(text[11],0,1,black,yellow,black,darkYellow,false); // back
			ue[2].clickFunc=[] { setPage(1); };
		} else {
			ue[2]=pfLabel(text[76],0,1,black,yellow,black,darkYellow,false); // give up
			ue[2].clickFunc=p2Giveup;
		}
		ue[3]=pfLabel(text[22],3,8+curGame.h,white,darkGreen,black,green,false);
		ue[3].clickFunc=p2Tab0;
		ue[4]=pfLabel(text[23],ue[3].x+text[22].len()+2,8+curGame.h,white,darkGreen,black,green,false);
		ue[4].clickFunc=p2Tab1;
		ue[5]=pfLabel(); // reserved for [Preferences] tab
		if(tab[0]>=0 && tab[0]<=2)
			ue[3+tab[0]].fgc=black, ue[3+tab[0]].bgc=green;
		if(p2npl==curGame.n)
			ue[6]=pfLabel(text[curGame.d>0?25:77],(winr.Right-text[curGame.d>0?25:77].len())/2,10,black,yellow,black,darkYellow,true);
		else
			ue[6]=pfLabel();
		ue[6].clickFunc=p2Ready;
		if(tab[0]==0) {
			if(p2isP1Ready) {
				nue=6;
			} else {
				ue[7]=pfLabel(text[26],12,18+curGame.h,black,yellow,black,darkYellow,false);
				ue[7].clickFunc=[] { if(!p2isP1Ready) bf1.clear(), p2npl=0, refreshPage(); };
				nue=7;
			}
		} else if(tab[0]==1) {
			if(curGame.d>0) {
				ue[7]=pfLabel((curGame.cw?text[30]:text[29])+text[31],4,10+curGame.h,dfc,dbc,grey,black,false);
				ue[7].clickFunc=p2SwitchCw;
			} else if(curGame.cw) {
				ue[7]=pfLabel(text[74],4,10+curGame.h,dfc,dbc,grey,black,false);
			} else {
				ue[7]=pfLabel();
			}
			if(curGame.d>0) {
				ue[8]=pfLabel((curGame.cd?text[30]:text[29])+text[83],4,12+curGame.h,dfc,dbc,grey,black,false);
				ue[8].clickFunc=[] { curGame.cd=!curGame.cd; refreshPage(); };
			} else if(curGame.cd) {
				ue[8]=pfLabel(text[84],4,12+curGame.h,dfc,dbc,grey,black,false);
			} else ue[8]=pfLabel();
			if(curGame.d>0) {
				ue[9]=pfLabel(text[32],4,11+curGame.h,black,yellow,black,darkYellow,false);
				ue[9].clickFunc=[] { p2AddPl(-1); };
				tmp.str("");
				tmp<<text[33].s<<curGame.n;
				ue[10]=pfLabel(pfTextElem(tmp.str(),text[33].d),ue[9].right()+2,11+curGame.h,dfc,dbc,0,0,false);
				ue[11]=pfLabel(text[34],ue[10].right()+2,11+curGame.h,black,yellow,black,darkYellow,false);
				ue[11].clickFunc=[] { p2AddPl(1); };
				ue[12]=pfLabel(text[89]+(isFirst?text[87]:text[88]),4,14+curGame.h,dfc,dbc,grey,black,false);
				ue[12].clickFunc=[] { isFirst=!isFirst, refreshPage(); };
			} else {
				tmp.str("");
				tmp<<text[75].s<<curGame.n;
				ue[9]=pfLabel(pfTextElem(tmp.str(),text[75].d),4,11+curGame.h,dfc,dbc,0,0,false);
				ue[10]=pfLabel(text[curGame.d==-1?78:79],4,13+curGame.h,dfc,dbc,0,0,false);
				ue[11]=pfLabel();
				ue[12]=pfLabel(text[89]+(isFirst?text[87]:text[88]),4,14+curGame.h,dfc,dbc,grey,black,false);
			}
			tmp.str("");
			tmp<<curGame.h<<"*"<<curGame.w;
			if(curGame.d>0)
				ue[13]=pfLabel(text[90]+tmp.str(),4,16+curGame.h,black,yellow,black,darkYellow,false),
				ue[13].clickFunc=[] { tab[15]=page; setPage(4); };
			else
				ue[13]=pfLabel(text[90]+tmp.str(),4,16+curGame.h,dfc,dbc,grey,black,false);
			nue=13;
		}
		if(p2isP2Ready)
			ue[++nue]=pfLabel(text[91].s,(winr.Right-text[91].len())/2,winr.Bottom*2/3,yellow,dbc,darkYellow,dbc,false);
		else if(p2isP1Ready)
			ue[++nue]=pfLabel(text[82].s,(winr.Right-text[82].len())/2,winr.Bottom*2/3,yellow,dbc,darkYellow,dbc,false);
	} else if(page==4) {
		ue[2]=pfLabel(text[11],0,1,black,yellow,black,darkYellow,false); // back
		ue[2].clickFunc=[] {
			bf1.resize(curGame.w,curGame.h), bf2.resize(curGame.w,curGame.h), bf3.resize(curGame.w,curGame.h);
			setPage(tab[15]);
		};
		tmp.str("");
		tmp<<curGame.h<<"*"<<curGame.w;
		ue[3]=pfLabel(text[90]+tmp.str(),4,2,dfc,dbc,grey,black,false);
		nue=3;
	} else if(page==5) {
		ue[2]=pfLabel(text[11],0,1,black,yellow,black,darkYellow,false); // back
		ue[2].clickFunc=[] { setPage(1); };
		ue[3]=pfLabel(text[19],(winr.Right-text[19].len())/2,10,black,yellow,black,darkYellow,false);
		ue[3].clickFunc=[] { system("explorer https://github.com/Zjl37/planeFight2"); refreshPage(); };
		ue[4]=pfLabel(text[12]+pfTextElem(pfVersion)+text[13],1,2,dfc,dbc,0,0,false);
		setDefaultColor();
		ue[5]=pfLabel(text[14],3,4,dfc,dbc,0,0,false);
		ue[6]=pfLabel(text[15],3,5,dfc,dbc,0,0,false);
		nue=6;
	} else if(page==10) {
		ue[2]=pfLabel(text[38],0,1,black,yellow,black,darkYellow,false); // back
		ue[2].clickFunc=p10Surrender;
		ue[3]=pfLabel(text[39],3,8+curGame.h,white,darkGreen,black,green,false);
		ue[3].clickFunc=[] { tab[0]=0, refreshPage(); };
		ue[4]=pfLabel(text[49],ue[3].right()+2,8+curGame.h,white,darkGreen,black,green,false);
		ue[4].clickFunc=[] { tab[0]=1, refreshPage(); };
		ue[5]=pfLabel(text[50],ue[4].right()+2,8+curGame.h,white,darkGreen,black,green,false);
		ue[5].clickFunc=[] { tab[0]=2, refreshPage(); };
		if(tab[0]>=0&&tab[0]<=2)
			ue[3+tab[0]].fgc=black, ue[3+tab[0]].bgc=green;
		if(tab[0]==1) {
			for(int i=0; i<PF_NMARKER; i++) {
				ue[6+i]=pfLabel(pfTextElem(marker[i], 1), 3+i%7*4, 10+curGame.h+i/7*2, i==tab[1] ? white : black, i==tab[1] ? darkYellow : yellow, white, darkYellow, false);
				ue[6+i].clickFunc=[i] { tab[1]=i; refreshPage(); };
			}
			nue=5+PF_NMARKER;
		} else nue=5;
	} else if(page==19) {
		if(p10srd==2)
			ue[2]=pfLabel(text[47],(winr.Right-text[47].len())/2,4,white,darkGreen,grey,darkGreen,true);
		else if(p10srd==1)
			ue[2]=pfLabel(text[46],(winr.Right-text[46].len())/2,4,white,darkRed,grey,darkRed,true);
		else if(p10des1==curGame.n)
			ue[2]=pfLabel(text[45],(winr.Right-text[45].len())/2,4,white,red,grey,red,true);
		else if(p10des2==curGame.n)
			ue[2]=pfLabel(text[44],(winr.Right-text[44].len())/2,4,black,green,black,darkGreen,true);
		else
			ue[2]=pfLabel();
		ue[3]=pfLabel(text[48],(winr.Right-text[48].len())/2,winr.Bottom*2/3,black,yellow,black,darkYellow,true);
		ue[3].clickFunc=[] { setPage(1); };
		nue=3;
	} else if(page==41) {
		ue[2]=pfLabel(text[11],0,1,black,yellow,black,darkYellow,false); // back
		ue[2].clickFunc=[] { setPage(1); };
		ue[3]=pfLabel(text[23],4,5,dfc,dbc,0,0,false);
		ue[4]=pfLabel(text[25],(winr.Right-text[25].len())/2,winr.Bottom*2/3,black,yellow,black,darkYellow,true);
		ue[4].clickFunc=[] { setPage(42); };
		ue[5]=pfLabel((curGame.cw?text[30]:text[29])+text[31],4,7,dfc,dbc,grey,black,false);
		ue[5].clickFunc=[] {
			curGame.cw=!curGame.cw; refreshPage();
		};
		ue[6]=pfLabel(text[32],4,9,black,yellow,black,darkYellow,false);
		ue[6].clickFunc=[] {
			curGame.n=max(1,curGame.n-1); refreshPage();
		};
		tmp.str("");
		tmp<<text[33].s<<curGame.n;
		ue[7]=pfLabel(pfTextElem(tmp.str(),text[33].d),ue[6].right()+2,9,dfc,dbc,0,0,false);
		ue[8]=pfLabel(text[34],ue[7].right()+2,9,black,yellow,black,darkYellow,false);
		ue[8].clickFunc=[] {
			curGame.n=min(curGame.n+1,curGame.w*curGame.h/10); refreshPage();
		};
		ue[9]=pfLabel((curGame.cd?text[30]:text[29])+text[83],4,11,dfc,dbc,grey,black,false);
		ue[9].clickFunc=[] {
			curGame.cd=!curGame.cd; refreshPage();
		};
		ue[10]=pfLabel(text[89]+(isFirst?text[87]:text[88]),4,13,dfc,dbc,grey,black,false);
		ue[10].clickFunc=[] { isFirst=!isFirst, refreshPage(); };
		tmp.str("");
		tmp<<curGame.h<<"*"<<curGame.w;
		ue[11]=pfLabel(text[90]+tmp.str(),4,15,black,yellow,black,darkYellow,false),
		ue[11].clickFunc=[] { tab[15]=page; setPage(4); };
		nue=11;
	} else if(page==42) {
		ue[2]=pfLabel(text[11],0,1,black,yellow,black,darkYellow,false); // back
		ue[2].clickFunc=[] { closesocket(sockServer); setPage(1); };
		if(sIP.empty())
			ue[3]=pfLabel(text[55],0,4,dfc,dbc,dfc,dbc,false);
		else
			ue[3]=pfLabel(text[54]+sIP,0,4,dfc,dbc,dfc,dbc,false);
		ue[4]=pfLabel(text[61],0,6,dfc,dbc,dfc,dbc,false);
		nue=4;
	} else if(page==51) {
		ue[2]=pfLabel(text[11],0,1,black,yellow,black,darkYellow,false); // back
		ue[2].clickFunc=[] { setPage(1); };
		ue[3]=pfLabel(text[67],4,5,dfc,dbc,0,0,false);
		nue=3;
	} else nue=1;
}

void refreshPage() {
	uptCursorState();
	if(winr.Right<70||winr.Bottom<28) return;
	buildUiElem();
	drawUiElem();
}

void pop_back_utf8(string &s) {
	if(s.empty()) return;
	while((s[s.length()-1]&192)==128)
		s.pop_back();
	s.pop_back();
}

#define isMouseEvent (rec.EventType==MOUSE_EVENT)
#define isKeyEvent (rec.EventType==KEY_EVENT)
#define vkCode rec.Event.KeyEvent.wVirtualKeyCode
#define isLmbPressed (rec.Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED)
#define mx rec.Event.MouseEvent.dwMousePosition.X
#define my rec.Event.MouseEvent.dwMousePosition.Y

void processMouseClick() {
	for(int i=nue; i>=1; i--)
		if(ue[i].click(mx,my)) {
			return;
		}
	if(page==2 && tab[0]==0) {
		if(my>=10+curGame.h && my<=16+curGame.h) {
			if(mx>=4 && mx<=17) tab[1]=0;
			else if(mx>=20 && mx<=33) tab[1]=1;
			else if(mx>=36 && mx<=49) tab[1]=2;
			else if(mx>=52 && mx<=65) tab[1]=3;
			drawPark();
		}
		if(mx>=bf1.x && mx<bf1.x+bf1.w*2 && my>=bf1.y && my<bf1.y+bf1.h && p2npl<curGame.n && !p2isP1Ready) {
			setDefaultColor();
			clearR(0,7+curGame.h,winr.Right,7+curGame.h);
			if(bf1.placeplane((mx-bf1.x)>>1,my-bf1.y,tab[1],curGame.cw)) {
				++p2npl;
				if(p2npl==curGame.n)
					refreshPage(); // so that the play button will appear
			} else {
				setColor(red,black);
				gotoXY(2,7+curGame.h), cout<<text[curGame.cw?27:28].s;
			}
			drawBF(false);
		}
	} else if(page==4) {
		int nx=(mx-bf1.x)/2, ny=my-bf1.y;
		if(nx>=5&&nx<=winr.Right&&ny>=5) if(nx!=curGame.w||ny!=curGame.h) {
			curGame.w=nx; curGame.h=ny;
			refreshPage();
		}
	} else if(page==10) {
		if(mx>=bf2.x && mx<bf2.x+bf2.w*2 && my>=bf2.y && my<bf2.y+bf2.h) {
			if(tab[0]==0 && isMyTurn() && bf3.mk[(mx-bf2.x)/2+(my-bf2.y)*bf3.w]==black) {
				attack((mx-bf2.x)/2,my-bf2.y);
			} else if(tab[0]==1) {
				bf3.ch[(mx-bf2.x)/2+(my-bf2.y)*bf3.w]=marker[tab[1]];
				drawBF(false);
			} else if(tab[0]==2) {
				bf3.ch[(mx-bf2.x)/2+(my-bf2.y)*bf3.w]="";
				drawBF(false);
			}
		}
	}
}

void process() {
	if(curGame.d>0 && page==10 && !isMyTurn()) {
		p10EnemyTurn();
		return;
	}
	if(isMouseEvent) {
		if(isLmbPressed) {
			processMouseClick();
		} else if(page==2) {
			if(mx>=bf1.x && mx<bf1.x+bf1.w*2 && my>=bf1.y && my<bf1.y+bf1.h && tab[0]==0 && p2npl<curGame.n && bf1.ch[(mx-bf1.x)/2+(my-bf1.y)*bf1.w].empty() && !p2isP1Ready) {
				drawBF(false);
				setColor(grey,black);
				drawPlane(bf1.x+((mx-bf1.x)&(short)-2),my,tab[1],true);
			}
		} else if(page==10) {
			if(mx>=bf2.x && mx<bf2.x+bf2.w*2 && my>=bf2.y && my<bf2.y+bf2.h && bf3.mk[(mx-bf2.x)/2+(my-bf2.y)*bf2.w]==0) {
				bf3.draw(true);
				setColor(grey,black);
				gotoXY(bf2.x+((mx-bf2.x)|1)-1,my), cout<<text[40].s;
			}
		}
	} else if(isKeyEvent && rec.Event.KeyEvent.bKeyDown) {
		if(page==0) {
			bool uptName = 0;
			if(vkCode==VK_RETURN) {
				ue[3]._click();
			} else if(vkCode==VK_BACK) {
				pop_back_utf8(playername.s);
				uptName = 1;
				gotoXY(2,8);
				cout<<setw(csbi.srWindow.Right)<<"";
			} else if(vkCode==VK_ESCAPE) {
				ue[5]._click();
			} else if(vkCode==0||vkCode>=32) {
				memset(buf,0,5);
				WideCharToMultiByte(65001,0,&rec.Event.KeyEvent.uChar.UnicodeChar,1,buf,4,NULL,NULL);
				playername.s+=buf;
				uptName = 1;
			}
			if(uptName) {
				gotoXY(2,8);
				cout<<text[6].s<<playername.s;
				playername.d=playername.s.length()-(getX()-(2+text[6].len())+(getY()-8)*winr.Right);
			}
		} else if(page==51) {
			if(vkCode==VK_RETURN) {
				if(pfClientConnect())
					setPage(2);
			} else if(vkCode==VK_BACK) {
				if(sIP.length()) {
					sIP.pop_back();
					gotoX(getX()-1), cout<<" ";
					gotoX(getX()-1);
				}
			} else if(vkCode==0||vkCode>=32) {
				WideCharToMultiByte(65001,0,&rec.Event.KeyEvent.uChar.UnicodeChar,1,buf,4,NULL,NULL);
				if(buf[0]=='.'||(buf[0]>='0'&&buf[0]<='9')) {
					sIP+=buf[0];
					cout<<buf[0];
				}
			}
		}
	}
}

int main(int argc, char** argv) {
	srand(time(0));
	conInit();
	string langDir;
	{
		char buf[65536];
		GetModuleFileNameA(NULL, buf, 65536);
		langDir = buf;
	}
	while(*langDir.rbegin()!='\\')
		langDir.pop_back();
	langDir.append("lang\\");
	if(!pfLangDetect(langDir)) {
		cerr<<pfVerStr<<endl;
		setColor_(red,black,hErr);
		cerr<<"Language file not found! Make sure folder \"lang\" is in the same directory as the executeable is. Or go to https://github.com/Zjl37/planeFight2 and re-download the game."<<endl;
		setDefaultColor_(hErr);
		return 1;
	}
	p0GenBg();
	setPage(0);
	while(_fl_) {
		DWORD tmp;
		GetConsoleScreenBufferInfo(hOut,&csbi);
		if(winr.Right!=csbi.srWindow.Right||winr.Bottom!=csbi.srWindow.Bottom) {
			winr=csbi.srWindow;
			if(page==0||page==1) p0GenBg();
			uptCursorState();
			refreshPage();
		}
		GetNumberOfConsoleInputEvents(hIn,&nEvents);
		while(nEvents--) {
			WINBOOL ret=ReadConsoleInput(hIn,&rec,1,&tmp);
			if(winr.Right<70||winr.Bottom<28) {
				clear();
				banner(text[35],winr.Bottom/2-1,white,red);
			} else if(ret) {
				process();
			} else {
				banner(text[3],1,white,red);
				break;
			}
		}
		if(curGame.d<=0 && (page==2 || (page==10 && !isMyTurn()))) {
			// aware of client socket event
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sockClient,&fds);
			timeval tv={0,100000};
			select(0,&fds,NULL,NULL,&tv);
			if(FD_ISSET(sockClient,&fds)) {
				if(page==2)
					p2Recv();
				else if(page==10)
					p10EnemyTurn();
			}
		} else if(page==42) {
			// aware of server socket event
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sockServer,&fds);
			timeval tv={0,100000};
			select(0,&fds,NULL,NULL,&tv);
			if(FD_ISSET(sockServer,&fds) && pfServerAccept())
				setPage(2);
		} else {
			// if not aware of any socket event, let PeekConsoleInput stuck the loop.
			PeekConsoleInput(hIn,&rec,1,&tmp);
		}
	}
	return 0;
}