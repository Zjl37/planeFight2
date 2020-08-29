#pragma once
#include <vector>
#include <string>
#include "pfUI.hpp"
using namespace std;

const short hcc[16]={7,15,15,15,15,15,15,0,15,15,0,0,15,0,0,0}; // High contrast color
const struct pfRePosCh {
	short dx,dy; string ch;
} plShape[4][10] = {
	{{0,0,"\u2503 "},{-2,1,"\u2501 "},{-1,1,"\u2501 "},{0,1,"\u254b "},{1,1,"\u2501 "},{2,1,"\u2501 "},{0,2,"\u2503 "},{-1,3,"\u2501 "},{0,3,"\u253b "},{1,3,"\u2501 "}},
	{{-3,-1,"\u2503 "},{-3,0,"\u2523 "},{-3,1,"\u2503 "},{-2,0,"\u2501 "},{-1,-2,"\u2503 "},{-1,-1,"\u2503 "},{-1,0,"\u254b "},{-1,1,"\u2503 "},{-1,2,"\u2503 "},{0,0,"\u2501 "}},
	{{-1,-3,"\u2501 "},{0,-3,"\u2533 "},{1,-3,"\u2501 "},{0,-2,"\u2503 "},{-2,-1,"\u2501 "},{-1,-1,"\u2501 "},{0,-1,"\u254b "},{1,-1,"\u2501 "},{2,-1,"\u2501 "},{0,0,"\u2503 "}},
	{{3,-1,"\u2503 "},{3,0,"\u252b "},{3,1,"\u2503 "},{2,0,"\u2501 "},{1,-2,"\u2503 "},{1,-1,"\u2503 "},{1,0,"\u254b "},{1,1,"\u2503 "},{1,2,"\u2503 "},{0,0,"\u2501 "}}
};

struct pfGameInfo {
	short w,h,n,d;
	// w,h: map size; n: number of planes; d: difficulty
	bool cw,cd; // cw: enable cross-border mode, cd: enable completely-destroy
};

struct pfBF {
	short x,y,w,h;
	vector<string> ch;
	vector<short> pl;
	vector<short> mk;
	void resize(short,short);
	void clear();
	void draw(bool);
	void basic_placeplane(short,short,short,bool);
	bool placeplane(short,short,short,bool);
};
