#pragma once
#include "pfGame.hpp"
#include "pfUI.hpp"
bool pfAIdecide(const pfGameInfo &g, const std::vector<short> &mk, short &tgx, short &tgy);

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
	void ArrangeReady(const pfBF &ar);
};