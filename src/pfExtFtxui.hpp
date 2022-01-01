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

#include "game.hpp"
#include "ftxui/component/component.hpp"

namespace ftxui::pfext {
	Component FlatButton(ConstStringRef label, std::function<void()> on_click, Decorator dec);
	Component FlatButton(ConstStringRef label, std::function<void()> on_click);
	Element BasicPfBattleField(const PfBF &bf, Box *box = nullptr);
	Element PfBattleFieldStatic(const PfBF &bf, Box *box = nullptr);
	Component PfBattleFieldPrepare(PfBF &bf, const pfGameInfo &gamerules, int &selectedFacing);
	Element BackgroundWithScatteredPlane();
	Component Park(int &selectedFacing);
	Component GameInfoInteractive(pfGameInfo &gamerules, PfBF &bf);
	Elements splitlines(const std::string &t);
	Component PfBattleFieldGame();
	Component FirstPlayerToggle(bool &state);
	Element GameInfoStatic(const pfGameInfo &gamerules);
}