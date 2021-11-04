#include "pfGame.hpp"
#include "ftxui/component/component.hpp"

namespace ftxui::pfext {
	Component FlatButton(ConstStringRef label, std::function<void()> on_click, Decorator dec);
	Component FlatButton(ConstStringRef label, std::function<void()> on_click);
	Element BasicPfBattleField(const pfBF &bf);
	Component PfBattleFieldPrepare(pfBF &bf, const pfGameInfo &gamerules, int &selectedFacing);
	Element BackgroundWithScatteredPlane();
	Component Park(int &selectedFacing);
	Component GameInfoInteractive(pfGameInfo &gamerules, pfBF &bf);
	Elements splitlines(const std::string &t);
	Element PfBattleFieldStatic(const pfBF &bf);
	Component PfBattleFieldGame();
	Component FirstPlayerToggle(bool &state);
	Element GameInfoStatic(const pfGameInfo &gamerules);
}