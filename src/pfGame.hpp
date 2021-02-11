#pragma once
#include <vector>
#include <string>
#include "pfUI.hpp"
using namespace std;

const short hcc[16]={7,15,15,15,15,15,15,0,15,15,0,0,15,0,0,0}; // High contrast color

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
