/**
 * Copyright © 2021 Zjl37 <2693911885@qq.com>
 *
 * This file is part of Zjl37/planeFight2.
 *
 * PlaneFight2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
struct PfBF {
	int w, h;
	int nPlaced = 0;
	mutable std::vector<std::string> ch;
	std::vector<short> pl;

	enum AttackRecord {
		unknown = 0,
		empty = 1,
		hit = 2,
		destroy = 3
	};
	std::vector<AttackRecord> mk;

	PfBF();
	PfBF(int w, int h);
	void resize(int nw, int nh);
	void clear();
	void basic_placeplane(int x, int y, short d, bool cw);
	bool TestPlace(int x, int y, short d, bool cw);
	bool placeplane(int x, int y, short d, bool cw);
	void RemovePlane(int x, int y);
	bool AutoArrange();
};
