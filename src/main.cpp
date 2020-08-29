#include <ctime>
#include <vector>
#include <sstream>
#include <winsock2.h>
#include "pfGame.hpp"
#include "pfUI.hpp"
#include "pfLang.hpp"
#include "pfAI.hpp"

using namespace std;

#define pfVersion "2.0"
#define pfVerStr "planefight 2.0"

const string marker[]={
	"\u2501 ","\u2503 ","\u254b ","\u2523 ","\u252b ","\u2533 ","\u253b ",
	"\u2500 ","\u2502 ","\u253c ","\u251c ","\u2524 ","\u252c ","\u2534 ",
	"\u2550 ","\u2551 ","\u256c ","\u2560 ","\u2563 ","\u2566 ","\u2569 ",
	"\uff1f","\uff01"
};

bool isFirst;
int page,tab[16],nue,turn;
DWORD nEvents;
HANDLE hIn,hOut;
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
		int tx=x+plShape[d][i].dx, ty=y+plShape[d][i].dy;
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
		int tx=x+plShape[d][i].dx, ty=y+plShape[d][i].dy;
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

int p2npl,p10des1,p10des2,p10srd;
WSADATA wsaData;
char buf[65536],sendbuf[65536]="pf",tmpbuf[65536];
string sIP;

void setPage(int);

void conInit() {
	DWORD mode;
	GetConsoleMode(hIn,&mode);
	mode |= ENABLE_PROCESSED_INPUT;
	mode |= ENABLE_MOUSE_INPUT;
	mode |= ENABLE_QUICK_EDIT_MODE;
	mode -= ENABLE_QUICK_EDIT_MODE;
	mode |= ENABLE_WINDOW_INPUT;
	SetConsoleMode(hIn,mode);
}

inline void clear() {
	system("cls"), conInit();
}

int pfCheckVer(const string &s) {
	if(s==pfVerStr) return 0;
	else return -1;
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
	gotoXY(0,3), cout<<text[53].s;
	gotoXY(0,getY()+1);
	if(getIP())
		cout<<text[54].s<<sIP;
	else
		cout<<text[55].s;
	sockServer=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sockServer==INVALID_SOCKET) {
		WSACleanup();
		showErrorMsg(text[56],1);
		return false;
	}
	gotoXY(0,getY()+1), cout<<text[57].s;
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
	gotoXY(0,getY()+1), cout<<text[59].s;
	ret=listen(sockServer,1);
	if(ret==SOCKET_ERROR) {
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[60],1);
		return false;
	}
	gotoXY(0,getY()+1), cout<<text[61].s;
	int len=sizeof(SOCKADDR);
	sockClient=accept(sockServer,(SOCKADDR*)&addrClient,&len);
	if(sockClient==INVALID_SOCKET) {
		closesocket(sockServer);
		WSACleanup();
		showErrorMsg(text[62],1);
		return false;
	}
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
		// check ret
		ret=recv(sockClient,buf,10,0);
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
	page=x;
	if(page==2) {
		memset(tab,0,sizeof tab);
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
		p2npl=0;
	} else if(page==10) {
		memset(tab,0,sizeof tab);
		p10des1=p10des2=p10srd=0;
	} else if(page==19) {
		bf2.mk=bf3.mk;
	} else if(page==41) {
		if(!curGame.n) {
			curGame.n=3;
			bf1.resize(10,10), bf2.resize(10,10), bf3.resize(10,10);
			curGame.w=curGame.h=10;
		} else {
			bf1.clear(), bf2.clear(), bf3.clear();
		}
	} else if(page==42) {
		curGame.d=-2;
		if(pfServerInit())
			setPage(2);
	} else if(page==51) {
		curGame.d=-1;
		sIP="";
		pfClientInit();
	} else if(page==52) {
		gotoXY(0,1);
		for(int i=0; i<winr.Right; i++) putchar(' ');
		if(pfClientConnect())
			setPage(2);
	}
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
			if(ty<bf1.y) ty+=bf1.y*2;
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
		gotoXY(bf1.x+i*2,bf1.y-2), cout<<setw(2)<<i;
		gotoXY(bf2.x+i*2,bf2.y-2), cout<<setw(2)<<i;
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

void drawMarker() {
	for(int i=0; i<23; i++)
		i==tab[1]?setColor(white,darkYellow):setColor(black,yellow),
		gotoXY(3+i%7*4,10+curGame.h+i/7*2), cout<<marker[i];
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
	} if(page==2) {
		drawBF(false);
		if(tab[0]==0)
			drawPark();
	} else if(page==10) {
		drawBF(false);
		gotoXY(winr.Right-10,1), cout<<"#"<<turn;
		if(tab[0]==1)
			drawMarker();
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
	playername.d=getX()-(2+text[6].len())+(getY()-8)*csbi.srWindow.Right;
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
	if(ue[4].y-ue[3].y>4) {
		ue[4].y=ue[3].y+4;
		nue=4;
	} else {
		ue[4].y=ue[3].y+7;
		ue[5]=pfLabel(text[20],8,10,dfc,dbc,grey,dbc,false);
		ue[5].clickFunc=p1Play21;
		ue[6]=pfLabel(text[21],8,11,dfc,dbc,grey,dbc,false);
		ue[6].clickFunc=p1Play22;
		nue=6;
	}
	drawUiElem();
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
	if(p2npl!=curGame.n) return;
	if(curGame.d>0) {
		bool tmp=bfInit(bf2);
		if(tmp==false) {
			refreshPage();
			return;
		}
		isFirst=rand()&1;
	} else {
		strcpy(sendbuf+6,"ready");
		int ret=send(sockClient,sendbuf,12,0);
		// check ret...
		gotoXY((winr.Right-text[82].len())/2,winr.Bottom*2/3), cout<<text[82].s;
		ret=recv(sockClient,buf,12,0);
		// check ret
		if(!pfCheckMsg(buf,NULL)) return;
		memcpy(tmpbuf,buf+6,6);
		tmpbuf[6]=0;
		if(strcmp(tmpbuf,"giveup")==0) {
			showErrorMsg(text[80],1);
			return;
		}
		tmpbuf[11]=0;
		if(strcmp(tmpbuf,"ready")) {
			showErrorMsg(text[81],1);
			return;
		}
		if(curGame.d&1) {
			// client
			ret=recv(sockClient,buf,12,0);
			if(!pfCheckMsg(buf,"start"))
				return;
			isFirst=!*(bool*)(buf+11);
		} else {
			strcpy(sendbuf+6,"start");
			*(bool*)(sendbuf+11)=isFirst=rand()&1;
			ret=send(sockClient,sendbuf,12,0);
			// check ret
		}
	}
	turn=1;
	banner(text[36],winr.Bottom/3,white,pink);
	Sleep(1000);
	setPage(10);
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
			int d=pl[x+y*curGame.w]&3;
			for(int i=0; i<10; i++)
				pl[x+plShape[d][i].dx+(y+plShape[d][i].dy)*curGame.w]|=16;
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
		ue[5]=pfLabel(text_en[4],2,12,dfc,dbc,grey,black,false);
		ue[5].clickFunc=[] { text=text_en; clear(); pfLangInit(winr.Right); refreshPage(); };
		ue[6]=pfLabel(text_zh_Hans[4],ue[5].right()+2,12,dfc,dbc,grey,black,false);
		ue[6].clickFunc=[] { text=text_zh_Hans; clear(); pfLangInit(winr.Right); refreshPage(); };
		nue=6;
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
		ue[6].clickFunc=p2Start;
		if(tab[0]==0) {
			ue[7]=pfLabel(text[26],12,18+curGame.h,black,yellow,black,darkYellow,false);
			ue[7].clickFunc=[] { bf1.clear(), p2npl=0, refreshPage(); };
			nue=7;
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
				nue=11;
			} else {
				tmp.str("");
				tmp<<text[75].s<<curGame.n;
				ue[9]=pfLabel(pfTextElem(tmp.str(),text[75].d),4,11+curGame.h,dfc,dbc,0,0,false);
				ue[10]=pfLabel(text[curGame.d==-1?78:79],4,13+curGame.h,dfc,dbc,0,0,false);
				nue=10;
			}
		}
	} else if(page==5) {
		ue[2]=pfLabel(text[11],0,1,black,yellow,black,darkYellow,false); // back
		ue[2].clickFunc=[] { setPage(1); };
		ue[3]=pfLabel(text[19],(winr.Right-text[19].len())/2,10,black,yellow,black,darkYellow,false);
		ue[3].clickFunc=[] { system("explorer https://github.com/Zjl37/planeFight2"); refreshPage(); };
		ue[4]=pfLabel(text[12]+pfTextElem(pfVersion)+text[13],1,2,dfc,dbc,0,0,false);
		setDefaultColor();
		ue[5]=pfLabel(text[14],3,4,dfc,dbc,0,0,false);
		ue[6]=pfLabel(text[15],3,5,dfc,dbc,0,0,false);
		ue[7]=pfLabel(text[16],3,6,dfc,dbc,0,0,false);
		ue[8]=pfLabel(text[17],3,7,dfc,dbc,0,0,false);
		nue=8;
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
		nue=5;
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
		nue=9;
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
	if(page==0) {
		p0GenBg();
	}
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
	for(int i=1; i<=nue; i++)
		if(ue[i].click(mx,my))
			return;
	if(page==2 && tab[0]==0) {
		if(my>=10+curGame.h && my<=16+curGame.h) {
			if(mx>=4 && mx<=17) tab[1]=0;
			else if(mx>=20 && mx<=33) tab[1]=1;
			else if(mx>=36 && mx<=49) tab[1]=2;
			else if(mx>=52 && mx<=65) tab[1]=3;
			drawPark();
		}
		if(mx>=bf1.x && mx<bf1.x+bf1.w*2 && my>=bf1.y && my<bf1.y+bf1.h && p2npl<curGame.n) {
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
	} else if(page==10) {
		if(mx>=bf2.x && mx<bf2.x+bf2.w*2 && my>=bf2.y && my<bf2.y+bf2.h) {
			if(tab[0]==0 && bf3.mk[(mx-bf2.x)/2+(my-bf2.y)*bf3.w]==black) {
				attack((mx-bf2.x)/2,my-bf2.y);
			} else if(tab[0]==1) {
				bf3.ch[(mx-bf2.x)/2+(my-bf2.y)*bf3.w]=marker[tab[1]];
				drawBF(false);
			} else if(tab[0]==2) {
				bf3.ch[(mx-bf2.x)/2+(my-bf2.y)*bf3.w]="";
				drawBF(false);
			}
		} else if(tab[0]==1) {
			for(int i=0; i<23; i++) if(my==10+curGame.h+i/7*2 && mx>=3+i%7*4 && mx<=5+i%7*4) {
				tab[1]=i;
				drawMarker();
			}
		}

	}
}

void process() {
	if(page==10 && !isMyTurn()) {
		p10EnemyTurn();
		return;
	}
	if(isMouseEvent) {
		if(isLmbPressed) {
			processMouseClick();
		} else if(page==2) {
			if(mx>=bf1.x && mx<bf1.x+bf1.w*2 && my>=bf1.y && my<bf1.y+bf1.h && tab[0]==0 && p2npl<curGame.n && bf1.ch[(mx-bf1.x)/2+(my-bf1.y)*bf1.w].empty()) {
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
			if(vkCode==13) { // enter
				ue[3]._click();
			} else if(vkCode==8) { // backspace
				pop_back_utf8(playername.s);
				gotoXY(2,8);
				cout<<setw(csbi.srWindow.Right)<<"";
				gotoXY(2,8);
				cout<<text[6].s<<playername.s;
			} else if(vkCode==0||vkCode>=32) {
				memset(buf,0,5);
				WideCharToMultiByte(65001,0,&rec.Event.KeyEvent.uChar.UnicodeChar,1,buf,4,NULL,NULL);
				playername.s+=buf;
				cout<<buf;
			}
		} else if(page==51) {
			if(vkCode==13) { // enter
				setPage(52);
			} else if(vkCode==8) { // backspace
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
	pfLangDetect();
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);
	hIn=GetStdHandle(STD_INPUT_HANDLE);
	hOut=GetStdHandle(STD_OUTPUT_HANDLE);
	srand(time(0));
	conInit();
	SetConsoleWindowInfo(hOut,true,&winr);
	pfLangInit(csbi.dwSize.X);
	GetConsoleScreenBufferInfo(hOut,&csbi);
	winr=csbi.srWindow;
	p0GenBg();
	setPage(0);
	while(true) {
		GetConsoleScreenBufferInfo(hOut,&csbi);
		if(winr.Right!=csbi.srWindow.Right||winr.Bottom!=csbi.srWindow.Bottom) {
			winr=csbi.srWindow;
			if(page==0||page==1) p0GenBg();
			uptCursorState();
			refreshPage();
		}
		WINBOOL ret=ReadConsoleInput(hIn,&rec,1,&nEvents);
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
	return 0;
}