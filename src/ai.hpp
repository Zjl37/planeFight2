/**
 * Copyright © 2021-2022 Zjl37 <2693911885@qq.com>
 * Copyright © 2021 qwqAutomaton
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
#include "game.hpp"
#include "ui.hpp"
bool pfAIdecide(const PfGameInfo &g, const std::vector<short> &mk, short &tgx, short &tgy);

class PfAI: public PfPlayer {
	struct {
		int x, y;
	} lastAtk;
	void OnGameStart();
	void Attack(short x, short y);
	void BeingAttacked(short, short);
	void AttackResulted(PfAtkRes res);

	public:
	PfAI();
	void ArrangeReady(const PfBF &ar);
};