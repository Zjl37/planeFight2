#pragma once
#include <string>
#include <vector>

#define pfUA "planeFight console application 2.6 (Zjl37/planefight2)"

// A structure to describe the shape of a "plane", storing a short string at
// a relative position to "plane head".
struct PfRePosCh {
	short dx, dy;
	std::string ch;
};

// plShape[i] describe the shape of a plane facing direction i.
// direction: 0=up 1=right 2=down 3=left
extern PfRePosCh plShape[4][10];

// Attack result
enum class PfAtkRes {
	empty = 0, // `void` is avoided for keyword
	hit = 1,
	destroy = 2,
};

// Battle field aka map(?)
struct pfBF {
	int w, h;
	int nPlaced = 0;
	mutable std::vector<std::string> ch;
	std::vector<short> pl;
	// TODO: the value and color of the attack result should be discriminated
	std::vector<short> mk;

	pfBF();
	pfBF(int w, int h);
	void resize(int nw, int nh);
	void clear();
	void basic_placeplane(int x, int y, short d, bool cw);
	bool placeplane(int x, int y, short d, bool cw);
	bool AutoArrange();
};
