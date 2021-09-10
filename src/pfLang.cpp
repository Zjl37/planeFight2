#include "pfUI.hpp"
#include "pfLang.hpp"
#include <windows.h>
using namespace std;

pfTextElem operator+(const pfTextElem &t1, const pfTextElem &t2) {
	return pfTextElem(t1.s + t2.s, t1.d + t2.d);
}

const int ntext = 256;
pfTextElem text[ntext];

void clear();

char buf1[65536];
struct pfLfIdx {
	int id;
	string sec, key;
};
const vector<pfLfIdx> idx{
	{ 0, "main", "inner_title" },
	{ 1, "main", "title" },
	{ 2, "main", "welcome" },
	{ 3, "main", "error" },
	{ 4, "main", "lang_name" },
	{ 5, "main", "loading" },
	{ 6, "main", "input_username" },
	{ 7, "main", "enter" },
	{ 8, "main", "start1" },
	{ 9, "main", "start2" },
	{ 10, "main", "start3" },
	{ 11, "main", "back" },
	// 12 const
	// 13 const
	{ 14, "about", "msg" },
	// 15 reserved
	{ 16, "main", "exit" },
	{ 17, "main", "chlang" },
	{ 19, "about", "repo" },
	{ 20, "main", "start21" },
	{ 21, "main", "start22" },
	{ 22, "game_init", "tab0" },
	{ 23, "game_init", "tab1" },
	{ 24, "game_init", "tab2" },
	{ 25, "game_init", "play" },
	{ 26, "game_init", "clear" },
	{ 27, "game_init", "wrong_plane1" },
	{ 28, "game_init", "wrong_plane2" },
	{ 29, "game_init", "button_unselect" },
	{ 30, "game_init", "button_select" },
	{ 31, "game_init", "cross_border_mode" },
	{ 32, "game_init", "button_dec" },
	{ 33, "game_init", "plane_num" },
	{ 34, "game_init", "button_inc" },
	{ 35, "main", "resize_win_msg" },
	{ 36, "game", "game_starting" },
	// 37 const
	{ 38, "game", "surrender" },
	{ 39, "game", "attack" },
	{ 40, "game", "cursor" },
	{ 41, "game", "void" },
	{ 42, "game", "hit" },
	{ 43, "game", "destroy" },
	{ 44, "game", "victory" },
	{ 45, "game", "lose" },
	{ 46, "game", "lose2" },
	{ 47, "game", "victory2" },
	{ 48, "game", "back_to_main" },
	{ 49, "game", "mark" },
	{ 50, "game", "erase" },
	// { 52, "socket", "err1" },
	// { 53, "socket", "msg1" },
	{ 54, "socket", "msg2" },
	{ 55, "socket", "msg3" },
	{ 56, "socket", "err4" },
	{ 57, "socket", "msg4" },
	{ 58, "socket", "err5" },
	{ 59, "socket", "msg5" },
	{ 60, "socket", "err6" },
	{ 61, "socket", "msg6" },
	{ 62, "socket", "err7" },
	// { 63, "socket", "msg7" },
	{ 64, "socket", "bad_ver1" },
	{ 65, "socket", "bad_ver2" },
	{ 66, "socket", "err8" },
	{ 67, "socket", "msg8" },
	{ 68, "socket", "err9" },
	// { 69, "socket", "msg9" },
	{ 70, "socket", "err10" },
	{ 71, "socket", "bad_msg" },
	{ 74, "game", "msg_cross_border_mode" },
	{ 75, "game", "msg_plane_num" },
	{ 76, "game", "give_up" },
	{ 77, "game", "ready" },
	// { 78, "game", "setting_info1" },
	// { 79, "game", "setting_info2" },
	{ 80, "game", "msg_give_up" },
	{ 81, "game", "err_bad_ready" },
	{ 82, "game", "wait_ready" },
	{ 83, "game", "completely_destroy" },
	{ 84, "game", "msg_completely_destroy" },
	{ 85, "game", "connection_lost1" },
	{ 86, "game", "connection_lost2" },
	{ 87, "game", "first1" },
	{ 88, "game", "first2" },
	{ 89, "game_init", "button_switch" },
	{ 90, "game", "msg_map_size" },
	{ 91, "game", "wait_ready1" },
	{ 92, "main", "exit0" }
};

void pfLangRead(const char Lang[]) {
	for(auto i: idx) {
		GetPrivateProfileStringA(i.sec.c_str(), i.key.c_str(), "", buf1, 65536, Lang);
		if(!buf1[0]) {
			stringstream tmp("");
			tmp << "LANG-" << i.id;
			text[i.id] = tmp.str();
		} else {
			text[i.id] = buf1;
		}
	}
	text[12] = "planeFight ";
	text[13] = " by Zjl37 ";
	text[37] = "AI";

	clear();
	pfLangInit(scrW);
}

pfLangFile lfi;
vector<pfLangFile> lf;
int curLfi = -1, fbLfi = -1;

WIN32_FIND_DATAA ffd;

bool pfLangDetect(const string &langDir) {
	LANGID lid = GetSystemDefaultLangID();
	HANDLE h = FindFirstFileA((langDir + "*.txt").c_str(), &ffd);
	if(h == INVALID_HANDLE_VALUE)
		return false;
	int ret = 0;
	do {
		string filename = langDir + ffd.cFileName;
		GetPrivateProfileStringA("lang", "match_lid_lb", "Lang pack missing", buf1, 65536, filename.c_str());
		if(strcmp(buf1, "Lang pack missing")) {
			lfi.dir = filename;
			stringstream tmp(buf1);
			tmp >> lfi.lidlb;
			lfi.lidrb.clear();
			GetPrivateProfileStringA("lang", "match_lid_rb", "Lang pack missing", buf1, 65536, filename.c_str());
			if(strcmp(buf1, "Lang pack missing")) {
				tmp.str(buf1);
				int lidrbi = 0;
				while(tmp >> lidrbi)
					lfi.lidrb.push_back(lidrbi);
			}
			GetPrivateProfileStringA("lang", "lang_name", "Lang pack missing", buf1, 65536, filename.c_str());
			gotoXY(0, 4);
			cout << buf1;
			lfi.langName.s = buf1;
			auto pos = getXY();
			lfi.langName.d = lfi.langName.s.length() - (pos.second - 4) * scrW - pos.first;
			lf.push_back(lfi);
		}
		ret = FindNextFileA(h, &ffd);
	} while(ret == TRUE);
	FindClose(h);
	if(!lf.size()) return false;
	for(int i = 0; i < (int)lf.size(); i++) {
		if((lid & 0xff) == lf[i].lidlb) {
			if(!lf[i].lidrb.size()) {
				curLfi = i;
			} else {
				bool flag = false;
				for(short j: lf[i].lidrb)
					if(j == (lid & 0xff00)) {
						flag = 1;
						break;
					}
				if(flag) curLfi = i;
			}
		}
		if((lid & 0xff) == 9)
			fbLfi = i;
	}
	if(~curLfi)
		pfLangRead(lf[curLfi].dir.c_str());
	else if(~fbLfi)
		pfLangRead(lf[fbLfi].dir.c_str());
	else
		pfLangRead(lf[0].dir.c_str());
	return true;
}

void pfLangInit(int winw) {
	gotoXY(2, 2);
	cout << text[5].s << endl;
	for(int i = 0; i < ntext; i++) {
		bool isAscii = true;
		for(unsigned j = 0; j < text[i].s.length(); j++)
			if(text[i].s[j] & 128) {
				isAscii = false;
				break;
			}
		if(isAscii) {
			text[i].d = 0;
			continue;
		}
		gotoXY(0, 4);
		cout << text[i].s;
		auto pos = getXY();
		text[i].d = text[i].s.length() - (pos.second - 4) * winw - pos.first;
	}
	SetConsoleTitleA(text[1].s.c_str());
}