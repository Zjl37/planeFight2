/**
 * Copyright © 2020-2021 Zjl37 <2693911885@qq.com>
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
#include "common.hpp"
#include "ui.hpp"
#include <memory>
#include <random>
#include <string>

// High contrast color
const short hcc[17] = { 7, 15, 15, 15, 15, 15, 15, 0, 15, 15, 0, 0, 15, 0, 0, 0, 16 };

struct pfGameInfo_legacy {
	short w, h, n, d;
	// w,h: map size; n: number of planes; d: difficulty
	bool cw, cd; // cw: enable cross-border mode, cd: enable completely-destroy
};

struct pfGameInfo {
	// w,h: map size; n: number of planes
	uint16_t w, h, n;
	// cw: enable cross-border mode; cd: enable completely-destroy
	bool cw, cd;
};

struct PfGame {
	unsigned id;
	pfGameInfo gamerules;
	bool isFirst;
	int turn;
	int nDestroyedOthers, nDestroyedMine;

	enum : unsigned {
		me_ready = 0x01,
		other_ready = 0x02,
		ready = 0x03,
		me_surrender = 0x04,
		other_surrender = 0x08,
	};
	unsigned state;

	PfGame();
	PfGame(pfGameInfo, unsigned id, bool);

	public:
	bool isMyTurn() const;
	bool Over() const;
};

class PfPlayer {
	protected:
	PfGame game;
	PfBF myBf, othersBf;
	std::string name;

	virtual void OnGameStart();

	public:
	PfPlayer() = default;
	std::weak_ptr<PfPlayer> other;

	// Actions
	virtual void NewGame(const pfGameInfo &, unsigned id, bool isFirst);
	void ArrangeReady();
	void Giveup();
	virtual void Attack(short, short);

	// Callback?
	virtual void OnOtherReady();
	virtual void OnOtherGiveup();
	virtual void OnOtherSurrender();
	virtual void BeingAttacked(short, short);
	virtual void AttackResulted(PfAtkRes); // hmm, how shold I name this fucntion
	virtual void MapRequested();
	virtual void Surrender();
	virtual void SetOthersBF(const std::vector<short> &pl);
	virtual void OnGameover();

	// Observer
	const PfGame &GetGame() const;
	const std::string &GetName() const;
	const PfBF &GetMyBF() const;
	const PfBF &GetOthersBF() const;
};

class PfLocalPlayer: public PfPlayer {
	struct {
		int x, y;
	} lastAtk;

	void NewGame(const pfGameInfo &, unsigned id, bool isFirst);
	void OnGameStart();
	void OnGameover();
	void OnOtherGiveup();
	void OnOtherSurrender();
	void Attack(short, short);
	void AttackResulted(PfAtkRes);
	void BeingAttacked(short x, short y);
	void Surrender();
	void SetOthersBF(const std::vector<short> &pl);

	public:
	PfLocalPlayer(const std::string &name);
	void ArrangeReady(const PfBF &ar);
};

extern pfGameInfo curGame;
extern PfBF bg, bf1;
extern std::shared_ptr<PfPlayer> player[2];