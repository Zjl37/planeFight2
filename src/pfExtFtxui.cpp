#include "pfExtFtxui.hpp"
#include "pfUiCtrl.hpp"
#include "pfUI.hpp"
#include "pfLocale.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include <memory>
#include <numeric>
#include <random>
#include <string>

extern std::mt19937 rng;

extern bool isFirst;

/* clang-format off */
namespace ftxui::pfext { // planeFight's extension
	Component FlatButton(ConstStringRef label, std::function<void()> on_click, Decorator dec) {
		static ButtonOption btnNoBorder{false};
		auto btn = Button(label, on_click, btnNoBorder);
		return Renderer(btn, [=]() { return btn->Render() | dec; });
	}
	Component FlatButton(ConstStringRef label, std::function<void()> on_click) {
		static ButtonOption btnNoBorder{false};
		auto btn = Button(label, on_click, btnNoBorder);
		return Renderer(btn, [=]() { return btn->Render(); });
	}
	Element BasicPfBattleField(const pfBF &bf, Box *box) {
		const int PFBF_CELL_WIDTH = 2;
		Elements lines;
		for(int i = 0; i < bf.h; i++) {
			Elements cells;
			for(int j = i*bf.w; j < (i+1)*bf.w; ++j) {
				Decorator dec = size(WIDTH, EQUAL, PFBF_CELL_WIDTH);
				if(bf.mk[j] == 10) {
					dec = dec | bgcolor(Color::Green);
				} else if(bf.mk[j] == 12) {
					dec = dec | bgcolor(Color::Red);
				} else if(bf.mk[j] == 4) {
					dec = dec | bgcolor(Color::DarkRed);
				}
				cells.push_back(text(bf.ch[j]) | dec);
			}
			lines.push_back(hbox(cells));
		}
		auto dec = size(WIDTH, EQUAL, bf.w * 2) | size(HEIGHT, EQUAL, bf.h);
		if(box) {
			dec = dec | reflect(*box);
		}
		return hbox({
			filler() | size(WIDTH, EQUAL, 1),
			vbox(lines) | dec
		});
	}
	Element PfBattleFieldStatic(const pfBF &bf, Box *box) {
		// cut tail zeros off and return the last digit.
		auto SigDigit = [](unsigned x) {
			if(x == 0) return x;
			while(x % 10 == 0) {
				x /= 10;
			}
			return x % 10;
		};
		std::string xscale(6 + 2*bf.w, ' ');
		for(int i = 0; i < bf.w; ++i) {
			xscale[4 + 2*i] = '0' + SigDigit(i);
		}
		Elements yscale(1, text(""));
		for(int j = 0; j < bf.h; ++j) {
			using namespace std::string_literals;
			yscale.push_back(text(char('0' + SigDigit(j)) + " "s));
		}
		return vbox({
			text(xscale),
			hbox({
				vbox(yscale),
				borderDouble(BasicPfBattleField(bf, box))
			})
		});
	}
	Component PfBattleFieldPrepare(pfBF &bf, const pfGameInfo &gamerules, int &selectedFacing) {
		bf.clear();
		struct BfInteractInfo {
			Box boxBf;
			struct {
				bool enabled = false;
				int x = 0, y = 0;
			} keyCtrl;
		};
		auto ii = std::make_shared<BfInteractInfo>();
		return CatchEvent(
			Renderer([&, ii](bool) {
				if(ii->keyCtrl.enabled) {
					return dbox({
						PfBattleFieldStatic(bf, &ii->boxBf),
						vbox({
							filler() | size(HEIGHT, EQUAL, 2 + ii->keyCtrl.y),
							hbox({
								filler() | size(WIDTH, EQUAL, 4 + ii->keyCtrl.x * 2),
								text("□") | color(Color::White),
							})
						})
					});
				}
				return PfBattleFieldStatic(bf, &ii->boxBf);
			}),
			[&, ii](Event e) {
				if(e.is_mouse()) {
					if(!ii->boxBf.Contain(e.mouse().x, e.mouse().y)) return false;
					int bx = (e.mouse().x - ii->boxBf.x_min) / 2, by = e.mouse().y - ii->boxBf.y_min;
					if(e.mouse().button == Mouse::WheelUp) {
						++selectedFacing %= 4;
					} else if(e.mouse().button == Mouse::WheelDown) {
						(selectedFacing += 3) %= 4;
					} else if(e.mouse().button == Mouse::None) {
						ii->keyCtrl.enabled = true;
						ii->keyCtrl.x = bx;
						ii->keyCtrl.y = by;
					} else if(e.mouse().motion == Mouse::Pressed) {
						if(e.mouse().button == Mouse::Left && bf.nPlaced < gamerules.n) {
							bf.placeplane(bx, by, selectedFacing, gamerules.cw);
						}
					}
					return true;
				} else if(e.is_character()) {
					if(ii->keyCtrl.enabled) {
						if(e.character() == "+" || e.character() == "=" || e.character() == "]") {
							++selectedFacing %= 4;
						} else if(e.character() == "-" || e.character() == "[") {
							(selectedFacing += 3) %= 4;
						}
					}
				} else {
					if(e == e.ArrowUp) {
						if(!ii->keyCtrl.enabled) {
							ii->keyCtrl.enabled = 1;
							return true;
						}
						if(ii->keyCtrl.y <= 0) {
							ii->keyCtrl.y = 0;
							ii->keyCtrl.enabled = false;
							return false;
						}
						--ii->keyCtrl.y;
						return true;
					} else if(e == e.ArrowDown) {
						if(!ii->keyCtrl.enabled) {
							ii->keyCtrl.enabled = 1;
							return true;
						}
						if(ii->keyCtrl.y >= bf.h - 1) {
							ii->keyCtrl.y = bf.h - 1;
							ii->keyCtrl.enabled = false;
							return false;
						}
						++ii->keyCtrl.y;
						return true;
					} else if(e == e.ArrowLeft) {
						if(!ii->keyCtrl.enabled) {
							ii->keyCtrl.enabled = 1;
							return true;
						}
						if(ii->keyCtrl.x <= 0) {
							ii->keyCtrl.x = 0;
							ii->keyCtrl.enabled = false;
							return false;
						}
						--ii->keyCtrl.x;
						return true;
					} else if(e == e.ArrowRight) {
						if(!ii->keyCtrl.enabled) {
							ii->keyCtrl.enabled = 1;
							return true;
						}
						if(ii->keyCtrl.x >= bf.w - 1) {
							ii->keyCtrl.x = bf.w - 1;
							ii->keyCtrl.enabled = false;
							return false;
						}
						++ii->keyCtrl.x;
						return true;
					} else if(e == e.Return) {
						if(ii->keyCtrl.enabled && bf.nPlaced < gamerules.n) {
							int bx = ii->keyCtrl.x, by = ii->keyCtrl.y;
							bf.placeplane(bx, by, selectedFacing, gamerules.cw);
						}
					}
				}
				return false;
			}
		);
	}
	Element BackgroundWithScatteredPlane() {
		if(pfui::scr.dimx() > 2*bg.w+1 || pfui::scr.dimy() > bg.h) {
			bg.resize(pfui::scr.dimx()/2, pfui::scr.dimy());
			for(int i = 0; i < (int)bg.w * bg.h / 25; i++)
				bg.placeplane(rng() % bg.w, rng() % bg.h, rng() & 3, false);
		}
		return BasicPfBattleField(bg);
	}
	Component Park(int &selectedFacing) {
		return Container::Horizontal({
			Container::Vertical({
				Renderer([]() { return text("Facing") | bold; }),
				Container::Vertical(
					{
						MenuEntry("up"),
						MenuEntry("right"),
						MenuEntry("down"),
						MenuEntry("left"),
					},
					&selectedFacing
				),
			}),
			Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
			Container::Tab({
				Renderer([]() { return vbox({
					text("              "),
					text("              "),
					text("      ┃       "),
					text("  ━━━━╋━━━━   "),
					text("      ┃       "),
					text("    ━━┻━━     "),
					text("              "),
				}) | color(Color::Black) | bgcolor(Color::White); }),
				Renderer([]() { return vbox({
					text("              "),
					text("      ┃       "),
					text("  ┃   ┃       "),
					text("  ┣━━━╋━━     "),
					text("  ┃   ┃       "),
					text("      ┃       "),
					text("              "),
				}) | color(Color::Black) | bgcolor(Color::White); }),
				Renderer([]() { return vbox({
					text("              "),
					text("    ━━┳━━     "),
					text("      ┃       "),
					text("  ━━━━╋━━━━   "),
					text("      ┃       "),
					text("              "),
					text("              "),
				}) | color(Color::Black) | bgcolor(Color::White); }),
				Renderer([]() { return vbox({
					text("              "),
					text("      ┃       "),
					text("      ┃   ┃   "),
					text("    ━━╋━━━┫   "),
					text("      ┃   ┃   "),
					text("      ┃       "),
					text("              "),
				}) | color(Color::Black) | bgcolor(Color::White); }),
			}, &selectedFacing)
		});
	}
	Component GameInfoInteractive(pfGameInfo &gamerules, pfBF &bf) {
		using namespace std::string_literals;

		CheckboxOption chkboxToggleCwOpt;
		chkboxToggleCwOpt.on_change = [&]() {
			if(!gamerules.cw) bf.clear();
		};

		auto component = Container::Vertical({
			Renderer([&]() {
				return text(TT("You are going to play with ").str() + (player[1] ? player[1]->GetName() : "???"));
			}),
			
			Container::Horizontal({
				FlatButton(TT("－").str(), [&]() {
					gamerules.n = std::max(gamerules.n - 1, 1);
					if(gamerules.n < bf.nPlaced) bf.clear();
				}, bgcolor(Color::Yellow)),
				Renderer([&]() {
					return text("  "s + TT("Number of planes: ").str() + std::to_string(gamerules.n) + "  ");
				}),
				FlatButton(TT("＋").str(), [&]() {
					++gamerules.n;
				}, bgcolor(Color::Yellow)),
			}),
			Checkbox(TT("Enable cross-border mode").str(), &gamerules.cw, chkboxToggleCwOpt),
			Checkbox(TT("Enable completely destroy").str(), &gamerules.cd),
			Container::Horizontal({
				Renderer([&]() {
					return text(TT("Map size: ").str() + std::to_string(gamerules.h) + "x" + std::to_string(gamerules.w));
				}),
				Renderer([&]() {
					return filler();
				}),
				FlatButton(
					TT("[adjust]").str(),
					[]() { NextPage(PfPage::adjust_map); },
					bgcolor(Color::Yellow)
				)
			}),
			pfext::FirstPlayerToggle(isFirst),
		});
		return Renderer(component, [=]() { return component->Render() | border; });
	}
	Element GameInfoStatic(const pfGameInfo &gamerules) {
		return vbox({
			text(TT("You are going to play with ").str() + (player[1] ? player[1]->GetName() : "???")),
			text(TT("Number of planes: ").str() + std::to_string(gamerules.n)),
			text(gamerules.cw ? TT("✓ Cross-border mode enabled").str() : ""),
			text(gamerules.cd ? TT("✓ Completely-destroy enabled").str() : ""),
			text(TT("Map size: ").str() + std::to_string(gamerules.h) + "x" + std::to_string(gamerules.w)),
			text(isFirst ? TT("I goes first") : TT("The other player goes first")),
		});
	}
	Elements splitlines(const std::string &t) {
		Elements res;
		std::stringstream ss(t);
		std::string s;
		while(std::getline(ss, s)) {
			res.push_back(text(s));
		}
		return res;
	}
	Component PfBattleFieldGame() {
		struct BfInteractInfo {
			Box boxBf;
			struct {
				bool enabled = false;
				int x = 0, y = 0;
			} keyCtrl;

			// BfInteractInfo() {
			// 	std::clog<<"BfInteractInfo constructed."<<std::endl;
			// }
			// ~BfInteractInfo() {
			// 	std::clog<<"BfInteractInfo destructed."<<std::endl;
			// }
		};
		auto ii = std::make_shared<BfInteractInfo>();
		return CatchEvent(
			Renderer([&, ii](bool) {
				const pfBF &bf = player[0]->GetOthersBF();
				if(ii->keyCtrl.enabled) {
					return dbox({
						PfBattleFieldStatic(bf, &ii->boxBf),
						vbox({
							filler() | size(HEIGHT, EQUAL, 2 + ii->keyCtrl.y),
							hbox({
								filler() | size(WIDTH, EQUAL, 3 + ii->keyCtrl.x * 2),
								text(">") | color(Color::White),
								filler() | size(WIDTH, EQUAL, 2),
								text("<") | color(Color::White)
							})
						})
					});
				}
				return PfBattleFieldStatic(bf, &ii->boxBf);
			}),
			[&, ii](Event e) {
				const pfBF &bf = player[0]->GetOthersBF();
				if(e.is_mouse()) {
					if(!ii->boxBf.Contain(e.mouse().x, e.mouse().y)) return false;
					int bx = (e.mouse().x - ii->boxBf.x_min) / 2, by = e.mouse().y - ii->boxBf.y_min;
					if(e.mouse().button == Mouse::Left) {
						if(e.mouse().motion == Mouse::Pressed) {
							if(player[0]->GetGame().isMyTurn() && bf.mk[bx + by * bf.w] == 0) {
								player[0]->Attack(bx, by);
								// std::clog << "[i] in " << __PRETTY_FUNCTION__ << " event handler returns at " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
							}
						}
					} else if(e.mouse().button == Mouse::None) {
						ii->keyCtrl.enabled = true;
						ii->keyCtrl.x = bx;
						ii->keyCtrl.y = by;
					}
					return true;
				} else if(e.is_character()) {
					if(ii->keyCtrl.enabled) {
						if(e.character() == " ") {
							ii->keyCtrl.enabled = false;
						} else if(e.character() == "-") {
							bf.ch[ii->keyCtrl.x + ii->keyCtrl.y * bf.w] = "─";
						} else if(e.character() == "|") {
							bf.ch[ii->keyCtrl.x + ii->keyCtrl.y * bf.w] = "│";
						} else if(e.character() == "+") {
							bf.ch[ii->keyCtrl.x + ii->keyCtrl.y * bf.w] = "┼";
						}
						return true;
					}
				} else {
					if(e == e.ArrowUp) {
						if(!ii->keyCtrl.enabled) {
							ii->keyCtrl.enabled = 1;
							return true;
						}
						if(ii->keyCtrl.y <= 0) {
							ii->keyCtrl.y = 0;
							ii->keyCtrl.enabled = false;
							return false;
						}
						--ii->keyCtrl.y;
						return true;
					} else if(e == e.ArrowDown) {
						if(!ii->keyCtrl.enabled) {
							ii->keyCtrl.enabled = 1;
							return true;
						}
						if(ii->keyCtrl.y >= bf.h - 1) {
							ii->keyCtrl.y = bf.h - 1;
							ii->keyCtrl.enabled = false;
							return false;
						}
						++ii->keyCtrl.y;
						return true;
					} else if(e == e.ArrowLeft) {
						if(!ii->keyCtrl.enabled) {
							ii->keyCtrl.enabled = 1;
							return true;
						}
						if(ii->keyCtrl.x <= 0) {
							ii->keyCtrl.x = 0;
							ii->keyCtrl.enabled = false;
							return false;
						}
						--ii->keyCtrl.x;
						return true;
					} else if(e == e.ArrowRight) {
						if(!ii->keyCtrl.enabled) {
							ii->keyCtrl.enabled = 1;
							return true;
						}
						if(ii->keyCtrl.x >= bf.w - 1) {
							ii->keyCtrl.x = bf.w - 1;
							ii->keyCtrl.enabled = false;
							return false;
						}
						++ii->keyCtrl.x;
						return true;
					} else if(e == e.Return) {
						if(ii->keyCtrl.enabled) {
							if(player[0]->GetGame().isMyTurn() && bf.mk[ii->keyCtrl.x + ii->keyCtrl.y * bf.w] == 0) {
								player[0]->Attack(ii->keyCtrl.x, ii->keyCtrl.y);
							}
						}
					} else if(e == e.Backspace || e == e.Delete) {
						if(ii->keyCtrl.enabled) {
							bf.ch[ii->keyCtrl.x + ii->keyCtrl.y * bf.w] = "";
						}
					}
				}
				return false;
			}
		);
	}
	Component FirstPlayerToggle(bool &state) {
		const static std::vector<std::string> &entries = {
			TT("The other player goes first"),
			TT("I goes first"),
		};
		struct ToggleInfo {
			int selected;
			bool &isFirst;
			ToggleOption opt;
			Component toggle = Toggle(&entries, &selected, &opt);

			ToggleInfo(bool &isFirst): selected(isFirst), isFirst(isFirst) {
				opt.on_change = [&]() {
					isFirst = selected;
				};
			}
		};
		auto ti = std::make_shared<ToggleInfo>(state);
		return Renderer(ti->toggle, [=]() {
			return ti->toggle->Render();
		});
		// the ToggleInfo will be destroyed when the Renderer is destroyed.
	}
};