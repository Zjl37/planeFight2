#pragma once
#include <string>
#include <vector>
using namespace std;

typedef struct pfTextElem {
	string s;
	int d;
	pfTextElem(): s("") {}
	pfTextElem(const char *t): s(t) {}
	pfTextElem(string t): s(t) {}
	pfTextElem(string t, int f): s(t), d(f) {}
	int len() const {
		return s.length() - d;
	}
} pfTextElem;
pfTextElem operator+(const pfTextElem &, const pfTextElem &);

typedef struct pfLangFile {
	string dir;
	pfTextElem langName;
	short lidlb;
	vector<short> lidrb;
} pfLangFile;

extern pfTextElem text[];
extern vector<pfLangFile> lf;

void pfLangRead(const char[]);
bool pfLangDetect(const string &langDir);
void pfLangInit(int);