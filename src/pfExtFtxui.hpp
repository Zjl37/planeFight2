#include "pfGame.hpp"
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