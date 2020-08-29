#include "pfGame.hpp"
#include "pfAI.hpp"
#include "pfUI.hpp"

pfBF vbf;

bool pfAIinit(const pfGameInfo &g, const vector<short> &mk) {
	vbf.clear();
	int k=0;
	for(int i=0; i<g.h; i++)
		for(int j=0; j<g.w; j++)
			if(mk[j+i*g.w]==darkRed) {
				if(vbf.pl[j+i*g.w]) return false;
				bool ret=0; char r=0;
				do {
					int d=rand()&3;
					ret=vbf.placeplane(j,i,d,g.cw);
					if(!ret) r|=1<<d;
				} while(!ret&&r!=15);
				if(r==15) return false;
				++k;
			}
	int wcl23=0;
	while(k<g.n) {
		short x=rand()%g.w, y=rand()%g.h;
		while(vbf.pl[x+y*g.w]||mk[x+y*g.w]) {
			x=rand()%g.w, y=rand()%g.h;
		}
		if(vbf.placeplane(x,y,rand()&3,g.cw))
			++k;
		++wcl23;
		if(wcl23>64)
			return false;
	}
	return true;
}

bool pfAIcheck(const pfGameInfo &g, const vector<short> &mk) {
	for(int j=0; j<g.h; j++)
		for(int i=0; i<g.w; i++) {
			if(!g.cd && mk[i+j*g.w]==green && vbf.pl[i+j*g.w])
				return false;
			if(mk[i+j*g.w]==red && !vbf.pl[i+j*g.w])
				return false;
		}
	return true;
}

#define PFAI_MAXTRY 1000000
bool pfAIdecide(const pfGameInfo &g, const vector<short> &mk, short &tgx, short &tgy) {
	vbf.resize(g.w,g.h);
	int ttt=0;
	do {
		bool ret=false;
		while(!ret&&ttt<PFAI_MAXTRY) {
			ret=pfAIinit(g,mk);
			++ttt;
		}
	} while(!pfAIcheck(g,mk)&&ttt<PFAI_MAXTRY);
	if(ttt>=PFAI_MAXTRY) {
		return false;
	}
	for(int i=0; i<g.h; i++)
		for(int j=0; j<g.w; j++)
			if(vbf.pl[j+i*g.w]&8 && mk[j+i*g.w]!=darkRed) {
				tgx=j, tgy=i;
				return true;
			}
	return false;
}