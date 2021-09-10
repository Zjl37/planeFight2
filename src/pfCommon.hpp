#pragma once
#include <string>
#include <vector>

#define pfVersion "2.6"
#define pfVerStr "planefight 2.6"
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
	// short x, y;
	uint16_t w, h;
	mutable std::vector<std::string> ch;
	std::vector<short> pl;
	// TODO: the value and color of the attack result should be discriminated
	std::vector<short> mk;

	pfBF();
	pfBF(uint16_t w, uint16_t h);
	void resize(short, short);
	void clear();
	void Draw(int, int, bool) const;
	void basic_placeplane(short, short, short, bool);
	bool placeplane(short, short, short, bool);
	bool AutoArrange();
};
