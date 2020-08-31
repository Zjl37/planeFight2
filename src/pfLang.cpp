#include <string>
#include "pfUI.hpp"
#include "pfLang.hpp"
using namespace std;

pfTextElem operator+(const pfTextElem &t1, const pfTextElem &t2) {
	return pfTextElem(t1.s+t2.s,t1.d+t2.d);
}

const int ntext=256;
pfTextElem text[ntext];

void clear();
extern SMALL_RECT winr;

char buf1[65536];

void pfLangRead(const char Lang[]) {
	GetPrivateProfileStringA("main","inner_title","Lang pack missing",buf1,65536,Lang), text[0]=buf1;
	GetPrivateProfileStringA("main","title","Lang pack missing",buf1,65536,Lang), text[1]=buf1;
	GetPrivateProfileStringA("main","welcome","Lang pack missing",buf1,65536,Lang), text[2]=buf1;
	GetPrivateProfileStringA("main","error","Lang pack missing",buf1,65536,Lang), text[3]=buf1;
	GetPrivateProfileStringA("main","lang_name","Lang pack missing",buf1,65536,Lang), text[4]=buf1;
	GetPrivateProfileStringA("main","loading","Lang pack missing",buf1,65536,Lang), text[5]=buf1;
	GetPrivateProfileStringA("main","input_username","Lang pack missing",buf1,65536,Lang), text[6]=buf1;
	GetPrivateProfileStringA("main","enter","Lang pack missing",buf1,65536,Lang), text[7]=buf1;
	GetPrivateProfileStringA("main","start1","Lang pack missing",buf1,65536,Lang), text[8]=buf1;
	GetPrivateProfileStringA("main","start2","Lang pack missing",buf1,65536,Lang), text[9]=buf1;
	GetPrivateProfileStringA("main","start3","Lang pack missing",buf1,65536,Lang), text[10]=buf1;
	GetPrivateProfileStringA("main","back","Lang pack missing",buf1,65536,Lang), text[11]=buf1;
	GetPrivateProfileStringA("main","start21","Lang pack missing",buf1,65536,Lang), text[20]=buf1;
	GetPrivateProfileStringA("main","start22","Lang pack missing",buf1,65536,Lang), text[21]=buf1;
	GetPrivateProfileStringA("main","resize_win_msg","Lang pack missing",buf1,65536,Lang), text[35]=buf1;
	GetPrivateProfileStringA("about","msg","Lang pack missing",buf1,65536,Lang), text[14]=buf1;
	GetPrivateProfileStringA("about","repo","Lang pack missing",buf1,65536,Lang), text[19]=buf1;
	GetPrivateProfileStringA("game_init","tab0","Lang pack missing",buf1,65536,Lang), text[22]=buf1;
	GetPrivateProfileStringA("game_init","tab1","Lang pack missing",buf1,65536,Lang), text[23]=buf1;
	GetPrivateProfileStringA("game_init","tab2","Lang pack missing",buf1,65536,Lang), text[24]=buf1;
	GetPrivateProfileStringA("game_init","play","Lang pack missing",buf1,65536,Lang), text[25]=buf1;
	GetPrivateProfileStringA("game_init","clear","Lang pack missing",buf1,65536,Lang), text[26]=buf1;
	GetPrivateProfileStringA("game_init","wrong_plane1","Lang pack missing",buf1,65536,Lang), text[27]=buf1;
	GetPrivateProfileStringA("game_init","wrong_plane2","Lang pack missing",buf1,65536,Lang), text[28]=buf1;
	GetPrivateProfileStringA("game_init","button_unselect","Lang pack missing",buf1,65536,Lang), text[29]=buf1;
	GetPrivateProfileStringA("game_init","button_select","Lang pack missing",buf1,65536,Lang), text[30]=buf1;
	GetPrivateProfileStringA("game_init","button_dec","Lang pack missing",buf1,65536,Lang), text[32]=buf1;
	GetPrivateProfileStringA("game_init","button_inc","Lang pack missing",buf1,65536,Lang), text[34]=buf1;
	GetPrivateProfileStringA("game_init","button_switch","Lang pack missing",buf1,65536,Lang), text[89]=buf1;
	GetPrivateProfileStringA("game_init","cross_border_mode","Lang pack missing",buf1,65536,Lang), text[31]=buf1;
	GetPrivateProfileStringA("game_init","plane_num","Lang pack missing",buf1,65536,Lang), text[33]=buf1;
	GetPrivateProfileStringA("game","game_starting","Lang pack missing",buf1,65536,Lang), text[36]=buf1;
	GetPrivateProfileStringA("game","surrender","Lang pack missing",buf1,65536,Lang), text[38]=buf1;
	GetPrivateProfileStringA("game","attack","Lang pack missing",buf1,65536,Lang), text[39]=buf1;
	GetPrivateProfileStringA("game","cursor","Lang pack missing",buf1,65536,Lang), text[40]=buf1;
	GetPrivateProfileStringA("game","void","Lang pack missing",buf1,65536,Lang), text[41]=buf1;
	GetPrivateProfileStringA("game","hit","Lang pack missing",buf1,65536,Lang), text[42]=buf1;
	GetPrivateProfileStringA("game","destroy","Lang pack missing",buf1,65536,Lang), text[43]=buf1;
	GetPrivateProfileStringA("game","victory","Lang pack missing",buf1,65536,Lang), text[44]=buf1;
	GetPrivateProfileStringA("game","lose","Lang pack missing",buf1,65536,Lang), text[45]=buf1;
	GetPrivateProfileStringA("game","lose2","Lang pack missing",buf1,65536,Lang), text[46]=buf1;
	GetPrivateProfileStringA("game","victory2","Lang pack missing",buf1,65536,Lang), text[47]=buf1;
	GetPrivateProfileStringA("game","back_to_main","Lang pack missing",buf1,65536,Lang), text[48]=buf1;
	GetPrivateProfileStringA("game","mark","Lang pack missing",buf1,65536,Lang), text[49]=buf1;
	GetPrivateProfileStringA("game","erase","Lang pack missing",buf1,65536,Lang), text[50]=buf1;
	GetPrivateProfileStringA("game","msg_cross_border_mode","Lang pack missing",buf1,65536,Lang), text[74]=buf1;
	GetPrivateProfileStringA("game","msg_plane_num","Lang pack missing",buf1,65536,Lang), text[75]=buf1;
	GetPrivateProfileStringA("game","give_up","Lang pack missing",buf1,65536,Lang), text[76]=buf1;
	GetPrivateProfileStringA("game","ready","Lang pack missing",buf1,65536,Lang), text[77]=buf1;
	GetPrivateProfileStringA("game","setting_info1","Lang pack missing",buf1,65536,Lang), text[78]=buf1;
	GetPrivateProfileStringA("game","setting_info2","Lang pack missing",buf1,65536,Lang), text[79]=buf1;
	GetPrivateProfileStringA("game","msg_give_up","Lang pack missing",buf1,65536,Lang), text[80]=buf1;
	GetPrivateProfileStringA("game","err_bad_ready","Lang pack missing",buf1,65536,Lang), text[81]=buf1;
	GetPrivateProfileStringA("game","wait_ready","Lang pack missing",buf1,65536,Lang), text[82]=buf1;
	GetPrivateProfileStringA("game","completely_destroy","Lang pack missing",buf1,65536,Lang), text[83]=buf1;
	GetPrivateProfileStringA("game","msg_completely_destroy","Lang pack missing",buf1,65536,Lang), text[84]=buf1;
	GetPrivateProfileStringA("game","connection_lost1","Lang pack missing",buf1,65536,Lang), text[85]=buf1;
	GetPrivateProfileStringA("game","connection_lost2","Lang pack missing",buf1,65536,Lang), text[86]=buf1;
	GetPrivateProfileStringA("game","first1","Lang pack missing",buf1,65536,Lang), text[87]=buf1;
	GetPrivateProfileStringA("game","first2","Lang pack missing",buf1,65536,Lang), text[88]=buf1;
	GetPrivateProfileStringA("socket","err1","Lang pack missing",buf1,65536,Lang), text[52]=buf1;
	GetPrivateProfileStringA("socket","msg1","Lang pack missing",buf1,65536,Lang), text[53]=buf1;
	GetPrivateProfileStringA("socket","msg2","Lang pack missing",buf1,65536,Lang), text[54]=buf1;
	GetPrivateProfileStringA("socket","msg3","Lang pack missing",buf1,65536,Lang), text[55]=buf1;
	GetPrivateProfileStringA("socket","err4","Lang pack missing",buf1,65536,Lang), text[56]=buf1;
	GetPrivateProfileStringA("socket","msg4","Lang pack missing",buf1,65536,Lang), text[57]=buf1;
	GetPrivateProfileStringA("socket","err5","Lang pack missing",buf1,65536,Lang), text[58]=buf1;
	GetPrivateProfileStringA("socket","msg5","Lang pack missing",buf1,65536,Lang), text[59]=buf1;
	GetPrivateProfileStringA("socket","err6","Lang pack missing",buf1,65536,Lang), text[60]=buf1;
	GetPrivateProfileStringA("socket","msg6","Lang pack missing",buf1,65536,Lang), text[61]=buf1;
	GetPrivateProfileStringA("socket","err7","Lang pack missing",buf1,65536,Lang), text[62]=buf1;
	GetPrivateProfileStringA("socket","msg7","Lang pack missing",buf1,65536,Lang), text[63]=buf1;
	GetPrivateProfileStringA("socket","bad_ver1","Lang pack missing",buf1,65536,Lang), text[64]=buf1;
	GetPrivateProfileStringA("socket","bad_ver2","Lang pack missing",buf1,65536,Lang), text[65]=buf1;
	GetPrivateProfileStringA("socket","err8","Lang pack missing",buf1,65536,Lang), text[66]=buf1;
	GetPrivateProfileStringA("socket","msg8","Lang pack missing",buf1,65536,Lang), text[67]=buf1;
	GetPrivateProfileStringA("socket","err9","Lang pack missing",buf1,65536,Lang), text[68]=buf1;
	GetPrivateProfileStringA("socket","msg9","Lang pack missing",buf1,65536,Lang), text[69]=buf1;
	GetPrivateProfileStringA("socket","err10","Lang pack missing",buf1,65536,Lang), text[70]=buf1;
	GetPrivateProfileStringA("socket","bad_msg","Lang pack missing",buf1,65536,Lang), text[71]=buf1;
	// for(int i=0;i<ntext;i++) if(text[i].s.size())
	// 	text[i].s.pop_back();
	text[12]="planeFight ";
	text[13]=" by Zjl37 ";
	text[37]="AI";

	clear();
	pfLangInit(winr.Right); 
}

void pfLangDetect() {
	LANGID lid = GetSystemDefaultLangID();
	if((lid&0xff)==0x04) pfLangRead("lang/zh-Hans.txt");
	else pfLangRead("lang/en.txt");
}

void pfLangInit(int winw) {
	gotoXY(2,2);
	cout<<text[5].s<<endl;
	for(int i=0; i<ntext; i++) {
		bool isAscii=true;
		for(unsigned j=0; j<text[i].s.length(); j++)
			if(text[i].s[j]&128) {
				isAscii=false; break;
			}
		if(isAscii) {
			text[i].d=0; continue;
		}
		gotoXY(0,4);
		cout<<text[i].s;
		text[i].d=text[i].s.length()-(getY()-4)*winw-getX();
	}
	SetConsoleTitleA(text[1].s.c_str());
}