#include "pfGame.hpp"
#include "ftxui/component/component.hpp"

namespace ftxui::pfext {
	Component FlatButton(ConstStringRef label, std::function<void()> on_click, Decorator dec);
	Component FlatButton(ConstStringRef label, std::function<void()> on_click);
	Element BasicPfBattleField(const pfBF &bf, Box *box = nullptr);
	Element PfBattleFieldStatic(const pfBF &bf, Box *box = nullptr);
	Component PfBattleFieldPrepare(pfBF &bf, const pfGameInfo &gamerules, int &selectedFacing);
	Element BackgroundWithScatteredPlane();
	Component Park(int &selectedFacing);
	Component GameInfoInteractive(pfGameInfo &gamerules, pfBF &bf);
	Elements splitlines(const std::string &t);
	Component PfBattleFieldGame();
	Component FirstPlayerToggle(bool &state);
	Element GameInfoStatic(const pfGameInfo &gamerules);
}