/**
 * Copyright © 2020-2022 Zjl37 <2693911885@qq.com>
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

struct PfGameInfo {
	// w,h: map size; n: number of planes
	uint16_t w, h, n;
	// cw: enable cross-border mode; cd: enable completely-destroy
	bool cw, cd, isFirst;
};

PfGameInfo InvertIsFirst(const PfGameInfo &gi);

struct PfGame {
	unsigned id;
	PfGameInfo gamerules;
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
	PfGame(PfGameInfo, unsigned id);

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
	virtual ~PfPlayer() = default;

	// Actions
	virtual void NewGame(const PfGameInfo &, unsigned id);
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
	const std::string &GetName() const;
};

class PfLocalPlayer {
	friend class PfPlayer;
	friend class PfRemotePlayer;

	PfGame game;
	PfBF myBf, othersBf;
	std::string name;
	struct {
		int x, y;
	} lastAtk;

	void OnGameStart();
	void OnGameover();
	void OnOtherGiveup();
	void OnOtherSurrender();
	void AttackResulted(PfAtkRes);
	void BeingAttacked(short x, short y);
	void SetOthersBF(const std::vector<short> &pl);

	public:
	PfLocalPlayer(const std::string &name);
	void NewGame(const PfGameInfo &, unsigned id);
	void Giveup();
	void OnOtherReady();
	void Attack(short, short);
	void ArrangeReady(const PfBF &ar);
	void Surrender();

	const PfGame &GetGame() const;
	const std::string &GetName() const;
	const PfBF &GetMyBF() const;
	const PfBF &GetOthersBF() const;
};

extern PfGameInfo curGame;
extern PfBF bf1;
extern std::shared_ptr<PfLocalPlayer> player0;
extern std::shared_ptr<PfPlayer> player1;
