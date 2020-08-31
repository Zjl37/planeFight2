#pragma once
#include <string>
using namespace std;

typedef struct pfTextElem {
	string s;
	int d;
	pfTextElem(): s("") {}
	pfTextElem(const char *t): s(t) {}
	pfTextElem(string t): s(t) {}
	pfTextElem(string t, int f): s(t), d(f) {}
	int len() const {
		return s.length()-d;
	}
} pfTextElem;
pfTextElem operator+(const pfTextElem &, const pfTextElem &);

extern pfTextElem text[];
extern pfTextElem text_zh_Hans[];
extern pfTextElem text_en[];

void pfLangRead(const char[]);
void pfLangDetect();
void pfLangInit(int);