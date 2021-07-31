#pragma once
#include <string>
#include <sstream>
#include <vector>

typedef struct pfTextElem {
	std::string s;
	int d;
	pfTextElem(): s("") {}
	pfTextElem(const char *t): s(t) {}
	pfTextElem(std::string t): s(t) {}
	pfTextElem(std::string t, int f): s(t), d(f) {}
	int len() const {
		return s.length() - d;
	}
} pfTextElem;
pfTextElem operator+(const pfTextElem &, const pfTextElem &);

typedef struct pfLangFile {
	std::string dir;
	pfTextElem langName;
	short lidlb;
	std::vector<short> lidrb;
} pfLangFile;

extern pfTextElem text[];
extern std::vector<pfLangFile> lf;

void pfLangRead(const char[]);
bool pfLangDetect(const std::string &langDir);
void pfLangInit(int);