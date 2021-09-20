#pragma once
#include "pfCommon.hpp"
#include "pfUI.hpp"
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
	pfBF myBf, othersBf;
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
	const pfBF &GetMyBF() const;
	const pfBF &GetOthersBF() const;
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
	void ArrangeReady(const pfBF &ar);
};

extern pfGameInfo curGame;
extern pfBF bg, bf1;
extern std::shared_ptr<PfPlayer> player[2];